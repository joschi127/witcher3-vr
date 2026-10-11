#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

// Test mode input scripts: a plain-text timeline of gamepad actions that the
// XInput hook replays on top of the physical controller, and a recorder that
// writes the physical controller in the same format. Times are milliseconds
// since the first Present, so a recording can be replayed unchanged.
//
//   # comment
//   1500 tap A            press and release (default 100 ms; "tap A 250")
//   2000 down LB          hold until "up LB"
//   2600 up LB
//   3000 stick L 0 1      left stick, x/y in -1..1, held until changed
//   3000 trigger R 1      right trigger 0..1
//   9000 capture          request an image capture (handled by the host)
//   60000 quit            close the game
//
// Button names: A B X Y LB RB BACK START LS RS UP DOWN LEFT RIGHT. The masks
// equal XINPUT_GAMEPAD_* so the host can OR them into XINPUT_GAMEPAD.
namespace w3vr::test_input {

enum class Action : uint8_t { Down, Up, Stick, Trigger, Capture, Quit };
enum class Side : uint8_t { Left, Right };

struct Event {
    uint32_t time_ms{};
    Action action{};
    uint16_t button{};
    Side side{};
    float x{};
    float y{};
};

struct PadState {
    uint16_t buttons{};
    float lx{};
    float ly{};
    float rx{};
    float ry{};
    float lt{};
    float rt{};
};

struct ButtonName {
    const char* name;
    uint16_t mask;
};

inline constexpr ButtonName kButtons[] = {
    {"UP", 0x0001}, {"DOWN", 0x0002}, {"LEFT", 0x0004}, {"RIGHT", 0x0008},
    {"START", 0x0010}, {"BACK", 0x0020}, {"LS", 0x0040}, {"RS", 0x0080},
    {"LB", 0x0100}, {"RB", 0x0200}, {"A", 0x1000}, {"B", 0x2000},
    {"X", 0x4000}, {"Y", 0x8000},
};

inline bool parse_button(std::string_view text, uint16_t& mask) {
    for (const auto& button : kButtons) {
        if (text == button.name) {
            mask = button.mask;
            return true;
        }
    }
    return false;
}

inline const char* button_name(uint16_t mask) {
    for (const auto& button : kButtons) {
        if (button.mask == mask) {
            return button.name;
        }
    }
    return "?";
}

namespace detail {
inline std::vector<std::string_view> split_words(std::string_view line) {
    std::vector<std::string_view> words;
    size_t pos = 0;
    while (pos < line.size()) {
        while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
            ++pos;
        }
        const size_t start = pos;
        while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t') {
            ++pos;
        }
        if (pos > start) {
            words.push_back(line.substr(start, pos - start));
        }
    }
    return words;
}

inline bool parse_uint(std::string_view text, uint32_t& value) {
    if (text.empty() || text.size() > 9) {
        return false;
    }
    uint32_t result = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        result = result * 10 + static_cast<uint32_t>(ch - '0');
    }
    value = result;
    return true;
}

inline bool parse_float(std::string_view text, float lo, float hi, float& value) {
    const std::string copy(text);
    char* end = nullptr;
    const float parsed = std::strtof(copy.c_str(), &end);
    if (end == copy.c_str() || *end != '\0' || !(parsed >= lo && parsed <= hi)) {
        return false;
    }
    value = parsed;
    return true;
}

inline bool parse_side(std::string_view text, Side& side) {
    if (text == "L") {
        side = Side::Left;
        return true;
    }
    if (text == "R") {
        side = Side::Right;
        return true;
    }
    return false;
}
}  // namespace detail

// Parses a script into time-ordered events ("tap" becomes down + up).
// On failure returns false and names the offending line in |error|.
inline bool parse_script(
    std::string_view text, std::vector<Event>& events, std::string& error) {
    events.clear();
    size_t line_number = 0;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string_view::npos) {
            end = text.size();
        }
        std::string_view line = text.substr(pos, end - pos);
        pos = end + 1;
        ++line_number;
        if (const size_t hash = line.find('#'); hash != std::string_view::npos) {
            line = line.substr(0, hash);
        }
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        const auto words = detail::split_words(line);
        if (words.empty()) {
            if (end == text.size()) {
                break;
            }
            continue;
        }
        const auto fail = [&](const char* why) {
            error = "line " + std::to_string(line_number) + ": " + why;
            return false;
        };
        Event event{};
        if (words.size() < 2 || !detail::parse_uint(words[0], event.time_ms)) {
            return fail("expected '<time_ms> <action> ...'");
        }
        const std::string_view action = words[1];
        if (action == "tap" || action == "down" || action == "up") {
            if (words.size() < 3 || !parse_button(words[2], event.button)) {
                return fail("unknown button");
            }
            if (action == "tap") {
                uint32_t duration = 100;
                if (words.size() >= 4 && !detail::parse_uint(words[3], duration)) {
                    return fail("bad tap duration");
                }
                event.action = Action::Down;
                events.push_back(event);
                event.action = Action::Up;
                event.time_ms += duration;
            } else {
                event.action = action == "down" ? Action::Down : Action::Up;
            }
        } else if (action == "stick") {
            if (words.size() < 5 || !detail::parse_side(words[2], event.side) ||
                !detail::parse_float(words[3], -1.0f, 1.0f, event.x) ||
                !detail::parse_float(words[4], -1.0f, 1.0f, event.y)) {
                return fail("expected 'stick L|R <x> <y>' with values in -1..1");
            }
            event.action = Action::Stick;
        } else if (action == "trigger") {
            if (words.size() < 4 || !detail::parse_side(words[2], event.side) ||
                !detail::parse_float(words[3], 0.0f, 1.0f, event.x)) {
                return fail("expected 'trigger L|R <value>' with value in 0..1");
            }
            event.action = Action::Trigger;
        } else if (action == "capture") {
            event.action = Action::Capture;
        } else if (action == "quit") {
            event.action = Action::Quit;
        } else {
            return fail("unknown action");
        }
        events.push_back(event);
        if (end == text.size()) {
            break;
        }
    }
    std::stable_sort(events.begin(), events.end(),
        [](const Event& a, const Event& b) { return a.time_ms < b.time_ms; });
    return true;
}

// Applies script events in time order. advance() returns the synthetic pad
// state to merge and reports one-shot actions that became due.
class Player {
public:
    explicit Player(std::vector<Event> events) : events_(std::move(events)) {}

    struct Due {
        uint32_t captures{};
        bool quit{};
    };

    Due advance(uint32_t now_ms) {
        Due due{};
        while (next_ < events_.size() && events_[next_].time_ms <= now_ms) {
            const Event& e = events_[next_++];
            switch (e.action) {
            case Action::Down: state_.buttons |= e.button; break;
            case Action::Up: state_.buttons &= static_cast<uint16_t>(~e.button); break;
            case Action::Stick:
                (e.side == Side::Left ? state_.lx : state_.rx) = e.x;
                (e.side == Side::Left ? state_.ly : state_.ry) = e.y;
                break;
            case Action::Trigger:
                (e.side == Side::Left ? state_.lt : state_.rt) = e.x;
                break;
            case Action::Capture: ++due.captures; break;
            case Action::Quit: due.quit = true; break;
            }
        }
        return due;
    }

    const PadState& state() const { return state_; }
    bool finished() const { return next_ >= events_.size(); }

private:
    std::vector<Event> events_;
    size_t next_{};
    PadState state_{};
};

// Converts successive physical pad states into script lines. Sticks use a
// dead zone and 0.05 steps, so small jitter does not flood the recording.
class Recorder {
public:
    static float quantize_stick(float value) {
        if (std::fabs(value) < 0.15f) {
            return 0.0f;
        }
        return std::round(value * 20.0f) / 20.0f;
    }

    static float quantize_trigger(float value) {
        return value < 0.1f ? 0.0f : std::round(value * 10.0f) / 10.0f;
    }

    std::vector<std::string> feed(uint32_t now_ms, const PadState& raw) {
        std::vector<std::string> lines;
        char buffer[96]{};
        const uint16_t changed = raw.buttons ^ last_.buttons;
        for (const auto& button : kButtons) {
            if ((changed & button.mask) != 0) {
                std::snprintf(buffer, sizeof(buffer), "%u %s %s", now_ms,
                    (raw.buttons & button.mask) != 0 ? "down" : "up", button.name);
                lines.emplace_back(buffer);
            }
        }
        last_.buttons = raw.buttons;
        const auto stick = [&](const char* side, float x, float y, float& last_x,
                               float& last_y) {
            const float qx = quantize_stick(x);
            const float qy = quantize_stick(y);
            if (qx != last_x || qy != last_y) {
                std::snprintf(buffer, sizeof(buffer), "%u stick %s %.2f %.2f",
                    now_ms, side, qx, qy);
                lines.emplace_back(buffer);
                last_x = qx;
                last_y = qy;
            }
        };
        stick("L", raw.lx, raw.ly, last_.lx, last_.ly);
        stick("R", raw.rx, raw.ry, last_.rx, last_.ry);
        const auto trigger = [&](const char* side, float value, float& last_value) {
            const float q = quantize_trigger(value);
            if (q != last_value) {
                std::snprintf(buffer, sizeof(buffer), "%u trigger %s %.1f", now_ms,
                    side, q);
                lines.emplace_back(buffer);
                last_value = q;
            }
        };
        trigger("L", raw.lt, last_.lt);
        trigger("R", raw.rt, last_.rt);
        return lines;
    }

private:
    PadState last_{};
};

}  // namespace w3vr::test_input
