#pragma once

#include <cctype>
#include <cstdlib>
#include <string>

// Parses a configurable hotkey from the INI: a function key name ("F1" to
// "F24", case-insensitive), "Space", "NumpadDivide" (the numpad "/" key, the
// recenter key of Luke Ross's R.E.A.L. VR mods), or a Windows virtual-key code
// in decimal or 0x hex.
// Returns 0 for anything else so the caller can fall back to its default.
namespace w3vr::hotkey_name {

inline constexpr int kVirtualKeyF1 = 0x70;  // VK_F1
inline constexpr int kVirtualKeySpace = 0x20;  // VK_SPACE
inline constexpr int kVirtualKeyNumpadDivide = 0x6F;  // VK_DIVIDE

inline int parse_virtual_key(const std::string& text) {
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    const std::string value = text.substr(begin, end - begin);
    if (value.empty()) {
        return 0;
    }
    std::string lower = value;
    for (char& c : lower) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (lower == "space") {
        return kVirtualKeySpace;
    }
    if (lower == "numpaddivide") {
        return kVirtualKeyNumpadDivide;
    }
    const bool function_key = (value[0] == 'F' || value[0] == 'f') &&
        value.size() >= 2 && value.size() <= 3 &&
        std::isdigit(static_cast<unsigned char>(value[1])) &&
        (value.size() == 2 || std::isdigit(static_cast<unsigned char>(value[2])));
    if (function_key) {
        const int number = std::atoi(value.c_str() + 1);
        return number >= 1 && number <= 24 ? kVirtualKeyF1 + number - 1 : 0;
    }
    char* parse_end = nullptr;
    const long code = std::strtol(value.c_str(), &parse_end, 0);
    if (parse_end == value.c_str() || *parse_end != '\0') {
        return 0;
    }
    return code >= 0x01 && code <= 0xFE ? static_cast<int>(code) : 0;
}

}  // namespace w3vr::hotkey_name
