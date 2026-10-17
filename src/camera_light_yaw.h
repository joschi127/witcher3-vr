#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>

// REDengine places its camera lights (dialogue/cutscene fill lights, gameplay
// camera lights) from the render camera's yaw. The mod composes the HMD
// rotation into that yaw, so the lights would follow every head turn like a
// head lamp. This ledger remembers, for each composed render-camera yaw, the
// game camera's own yaw, so the light placement can use the latter.
//
// Keyed by the exact bits of the composed yaw: the light placement reads the
// same float the view rebuild consumed. A small lock-free ring is enough; a
// miss leaves the native behaviour unchanged.
namespace w3vr::camera_light_yaw {

class Ledger {
public:
    static constexpr size_t kSlots = 64;

    void record(float composed_yaw, float base_yaw) {
        if (bits(composed_yaw) == bits(base_yaw)) {
            return;
        }
        const uint64_t packed =
            (static_cast<uint64_t>(bits(composed_yaw)) << 32) | bits(base_yaw);
        const size_t slot =
            next_.fetch_add(1, std::memory_order_relaxed) % kSlots;
        slots_[slot].store(packed, std::memory_order_release);
    }

    bool lookup(float composed_yaw, float& base_yaw) const {
        const uint32_t key = bits(composed_yaw);
        for (const auto& slot : slots_) {
            const uint64_t packed = slot.load(std::memory_order_acquire);
            if (packed != 0 && static_cast<uint32_t>(packed >> 32) == key) {
                const uint32_t base = static_cast<uint32_t>(packed);
                std::memcpy(&base_yaw, &base, sizeof(base_yaw));
                return true;
            }
        }
        return false;
    }

private:
    static uint32_t bits(float value) {
        uint32_t result{};
        std::memcpy(&result, &value, sizeof(result));
        return result;
    }

    std::array<std::atomic<uint64_t>, kSlots> slots_{};
    std::atomic<size_t> next_{};
};

}  // namespace w3vr::camera_light_yaw
