"""Render witcher3vr-launcher.svg to witcher3vr-launcher.ico.

Uses headless Microsoft Edge to rasterise the SVG (no extra Python packages needed):
all icon sizes are drawn side by side in one transparent screenshot, which is then
cut into per-size PNG images and packed into a multi-size ICO file.

Usage: python launcher/icon/make-icon.py [--edge <path to msedge.exe>]
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path

SIZES = [256, 128, 64, 48, 40, 32, 24, 20, 16]
HERE = Path(__file__).resolve().parent
SVG = HERE / "witcher3vr-launcher.svg"
ICO = HERE / "witcher3vr-launcher.ico"
DEFAULT_EDGE = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"


def read_png_rgba(data: bytes):
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG file")
    pos, idat, header = 8, b"", None
    while pos < len(data):
        length, kind = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        if kind == b"IHDR":
            header = struct.unpack(">IIBBBBB", body)
        elif kind == b"IDAT":
            idat += body
        pos += 12 + length
    width, height, depth, color, _, _, interlace = header
    if depth != 8 or color not in (2, 6) or interlace:
        raise ValueError(f"unsupported PNG format (depth {depth}, colour {color})")
    channels = 4 if color == 6 else 3
    raw = zlib.decompress(idat)
    stride = width * channels
    rows, prev, pos = [], bytearray(stride), 0
    for _ in range(height):
        kind = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            a = line[i - channels] if i >= channels else 0
            b = prev[i]
            c = prev[i - channels] if i >= channels else 0
            if kind == 1:
                line[i] = (line[i] + a) & 0xFF
            elif kind == 2:
                line[i] = (line[i] + b) & 0xFF
            elif kind == 3:
                line[i] = (line[i] + ((a + b) >> 1)) & 0xFF
            elif kind == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xFF
        rows.append(bytes(line) if channels == 4 else
                    b"".join(bytes(line[i:i + 3]) + b"\xff" for i in range(0, stride, 3)))
        prev = line
    return width, height, rows


def write_png_rgba(width: int, rows) -> bytes:
    def chunk(kind, body):
        return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body))
    raw = b"".join(b"\x00" + row for row in rows)
    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", width, len(rows), 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 9))
            + chunk(b"IEND", b""))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--edge", default=DEFAULT_EDGE)
    args = parser.parse_args()

    total_width = sum(SIZES)
    images = []
    x = 0
    for size in SIZES:
        images.append(f'<img src="{SVG.as_uri()}" width="{size}" height="{size}" '
                      f'style="position:absolute;left:{x}px;top:0">')
        x += size
    page = ("<!doctype html><html><body style='margin:0;background:transparent'>"
            + "".join(images) + "</body></html>")

    with tempfile.TemporaryDirectory() as tmp:
        html = Path(tmp) / "icon.html"
        shot = Path(tmp) / "icon.png"
        html.write_text(page, encoding="utf-8")
        subprocess.run([args.edge, "--headless", "--disable-gpu", "--hide-scrollbars",
                        "--force-device-scale-factor=1", "--default-background-color=00000000",
                        f"--window-size={total_width},{max(SIZES)}",
                        f"--screenshot={shot}", html.as_uri()],
                       check=True, capture_output=True)
        width, height, rows = read_png_rgba(shot.read_bytes())

    pngs = []
    x = 0
    for size in SIZES:
        crop = [row[x * 4:(x + size) * 4] for row in rows[:size]]
        if not any(px for row in crop for px in row[3::4]):
            raise RuntimeError(f"{size}px image is empty; rendering failed")
        pngs.append((size, write_png_rgba(size, crop)))
        x += size

    header = struct.pack("<HHH", 0, 1, len(pngs))
    offset = 6 + 16 * len(pngs)
    entries, blobs = b"", b""
    for size, png in pngs:
        entries += struct.pack("<BBBBHHII", size % 256, size % 256, 0, 0, 1, 32, len(png), offset)
        blobs += png
        offset += len(png)
    ICO.write_bytes(header + entries + blobs)
    print(f"wrote {ICO} ({len(pngs)} sizes, {ICO.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
