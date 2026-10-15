#include "hotkey_name.h"

#include <cstdio>

namespace {
int failures{};
void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}
}  // namespace

int main() {
    using w3vr::hotkey_name::parse_virtual_key;

    require(parse_virtual_key("F9") == 0x78, "F9 maps to VK_F9");
    require(parse_virtual_key("f5") == 0x74, "lower-case f5 maps to VK_F5");
    require(parse_virtual_key(" F12 ") == 0x7B, "surrounding spaces are ignored");
    require(parse_virtual_key("F24") == 0x87, "F24 is the last function key");
    require(parse_virtual_key("F0") == 0, "F0 is rejected");
    require(parse_virtual_key("F25") == 0, "F25 is rejected");
    require(parse_virtual_key("0x74") == 0x74, "hex virtual-key code");
    require(parse_virtual_key("116") == 0x74, "decimal virtual-key code");
    require(parse_virtual_key("0") == 0, "zero is rejected");
    require(parse_virtual_key("0x100") == 0, "codes above 0xFE are rejected");
    require(parse_virtual_key("") == 0, "empty value is rejected");
    require(parse_virtual_key("Home") == 0, "unknown names are rejected");
    require(parse_virtual_key("F5x") == 0, "trailing text is rejected");

    if (failures == 0) {
        std::puts("hotkey_name tests passed");
    }
    return failures == 0 ? 0 : 1;
}
