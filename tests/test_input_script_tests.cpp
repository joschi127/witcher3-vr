#include "test_input_script.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {
int failures{};
void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}
}

int main() {
    using namespace w3vr::test_input;

    std::vector<Event> events;
    std::string error;
    const char* script =
        "# intro\r\n"
        "1000 tap A\r\n"
        "  \r\n"
        "1500 tap X 250   # longer press\n"
        "3000 stick L 0 1\n"
        "3000 trigger R 0.5\n"
        "2000 down LB\n"
        "2600 up LB\n"
        "4000 capture\n"
        "4200 flight_dump\n"
        "5000 stick L 0 0\n"
        "9000 quit";
    require(parse_script(script, events, error), "valid script parses");
    require(events.size() == 12, "tap expands to down and up");
    for (size_t i = 1; i < events.size(); ++i) {
        require(events[i - 1].time_ms <= events[i].time_ms, "events are time ordered");
    }

    Player player(events);
    auto due = player.advance(999);
    require(player.state().buttons == 0 && due.captures == 0 && !due.quit,
        "nothing before the first event");
    player.advance(1000);
    require(player.state().buttons == 0x1000, "A held during tap");
    player.advance(1100);
    require(player.state().buttons == 0, "A released after 100 ms");
    player.advance(1600);
    require(player.state().buttons == 0x4000, "X held during a 250 ms tap");
    player.advance(2100);
    require(player.state().buttons == 0x0100, "X released, LB down");
    due = player.advance(4000);
    require(player.state().buttons == 0, "LB released");
    require(player.state().ly == 1.0f && player.state().rt == 0.5f,
        "stick and trigger applied");
    require(due.captures == 1 && !due.quit, "capture reported once");
    due = player.advance(4500);
    require(due.captures == 0, "capture not repeated");
    require(due.flight_dumps == 1, "flight dump reported once");
    due = player.advance(10000);
    require(player.state().ly == 0.0f, "stick returned to centre");
    require(due.quit && player.finished(), "quit reported at the end");

    const char* bad[] = {
        "abc tap A", "100 tap Z", "100 stick L 2 0", "100 trigger L -1",
        "100 jump", "100", "100 tap A x",
    };
    for (const char* text : bad) {
        require(!parse_script(text, events, error), "invalid line is rejected");
        require(error.rfind("line 1:", 0) == 0, "error names the line");
    }

    Recorder recorder;
    PadState pad{};
    require(recorder.feed(0, pad).empty(), "idle pad records nothing");
    pad.buttons = 0x1000;
    pad.lx = 0.07f;
    auto lines = recorder.feed(500, pad);
    require(lines.size() == 1 && lines[0] == "500 down A",
        "button press recorded, stick jitter ignored");
    pad.buttons = 0;
    pad.ly = 0.98f;
    lines = recorder.feed(620, pad);
    require(lines.size() == 2 && lines[0] == "620 up A" &&
            lines[1] == "620 stick L 0.00 1.00",
        "release and stick move recorded");
    require(recorder.feed(700, pad).empty(), "unchanged state records nothing");

    std::string replay;
    for (const auto& line : {"500 down A", "620 up A", "620 stick L 0.00 1.00"}) {
        replay += line;
        replay += '\n';
    }
    require(parse_script(replay, events, error) && events.size() == 3,
        "recorded lines parse as a script");

    require(to_gameinput_buttons(0x1000) == 0x00000004, "A maps to GameInput A");
    require(to_gameinput_buttons(0x0010 | 0x0020) == 0x00000003,
        "START/BACK map to Menu/View");
    require(from_gameinput_buttons(0x00000010 | 0x00000800) == (0x4000 | 0x0200),
        "GameInput X and right shoulder map back");
    require(from_gameinput_buttons(0x00010000) == 0,
        "GameInput-only trigger button has no XInput mask");
    for (const auto& button : kButtons) {
        require(from_gameinput_buttons(to_gameinput_buttons(button.mask)) == button.mask,
            "every script button round-trips through GameInput");
    }

    if (failures != 0) {
        std::fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    std::puts("test_input_script tests passed");
    return 0;
}
