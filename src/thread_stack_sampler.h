#pragma once

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Diagnostic CPU stack sampler for one thread ([debug] thread_stack_sampler=1).
// A low-priority thread suspends the target about once per millisecond, reads
// its context, unwinds a few frames with the x64 unwind tables and resumes it.
// While the target is suspended the sampler only touches preallocated memory
// (no heap, no logging), so it cannot deadlock on a lock the target holds.
// report() aggregates the ring into leaf and per-module "first frame" tables;
// addresses are printed as module+RVA for offline symbolization (linker map).
namespace w3vr::thread_stack_sampler {

inline constexpr size_t kMaxFrames = 16;
inline constexpr size_t kRingSamples = 16384;

struct Sample {
    uint64_t qpc{};
    uint32_t depth{};
    std::array<uint64_t, kMaxFrames> frames{};
};

struct State {
    std::atomic<bool> running{};
    std::atomic<DWORD> target_tid{};
    std::atomic<uint64_t> written{};
    std::atomic<uint64_t> failed{};
    std::vector<Sample> ring;
    HANDLE thread{};
};

inline State& state() {
    static State instance;
    return instance;
}

inline void set_target_thread(DWORD tid) {
    auto& s = state();
    if (s.running.load(std::memory_order_relaxed)) {
        s.target_tid.store(tid, std::memory_order_relaxed);
    }
}

inline uint32_t unwind_suspended(CONTEXT context, std::array<uint64_t, kMaxFrames>& frames) {
    uint32_t depth = 0;
    while (depth < kMaxFrames && context.Rip != 0) {
        frames[depth++] = context.Rip;
        DWORD64 image_base{};
        auto* function = RtlLookupFunctionEntry(context.Rip, &image_base, nullptr);
        if (function == nullptr) {
            // Leaf function without unwind data: the return address is at RSP.
            if (context.Rsp == 0) {
                break;
            }
            context.Rip = *reinterpret_cast<const DWORD64*>(context.Rsp);
            context.Rsp += 8;
            continue;
        }
        void* handler_data{};
        DWORD64 establisher_frame{};
        RtlVirtualUnwind(UNW_FLAG_NHANDLER, image_base, context.Rip, function,
            &context, &handler_data, &establisher_frame, nullptr);
    }
    return depth;
}

inline DWORD WINAPI sampler_main(void*) {
    auto& s = state();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    DWORD opened_tid{};
    HANDLE target{};
    while (s.running.load(std::memory_order_relaxed)) {
        Sleep(1);
        const DWORD tid = s.target_tid.load(std::memory_order_relaxed);
        if (tid == 0 || tid == GetCurrentThreadId()) {
            continue;
        }
        if (tid != opened_tid) {
            if (target != nullptr) {
                CloseHandle(target);
            }
            target = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                    THREAD_QUERY_INFORMATION, FALSE, tid);
            opened_tid = target != nullptr ? tid : 0;
            if (target == nullptr) {
                continue;
            }
        }
        const auto index = s.written.load(std::memory_order_relaxed);
        Sample& slot = s.ring[index % kRingSamples];
        if (SuspendThread(target) == static_cast<DWORD>(-1)) {
            s.failed.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        CONTEXT context{};
        context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        bool ok = GetThreadContext(target, &context) != FALSE;
        if (ok) {
            __try {
                slot.depth = unwind_suspended(context, slot.frames);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                ok = false;
            }
        }
        ResumeThread(target);
        if (!ok) {
            s.failed.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        slot.qpc = static_cast<uint64_t>(now.QuadPart);
        s.written.store(index + 1, std::memory_order_release);
    }
    if (target != nullptr) {
        CloseHandle(target);
    }
    return 0;
}

inline void start() {
    auto& s = state();
    if (s.running.exchange(true)) {
        return;
    }
    s.ring.resize(kRingSamples);
    s.thread = CreateThread(nullptr, 0, &sampler_main, nullptr, 0, nullptr);
    if (s.thread == nullptr) {
        s.running.store(false);
    }
}

inline bool enabled() { return state().running.load(std::memory_order_relaxed); }

struct ModuleRef {
    std::string name;
    uint64_t base{};
};

inline ModuleRef module_for(uint64_t address) {
    HMODULE module{};
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(address), &module)) {
        return {"?", 0};
    }
    char path[MAX_PATH]{};
    GetModuleFileNameA(module, path, MAX_PATH);
    const char* name = path;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '\\' || *p == '/') {
            name = p + 1;
        }
    }
    return {name, reinterpret_cast<uint64_t>(module)};
}

// Writes the last `seconds` of samples through `emit` (one line per call).
template <typename Emit>
void report(double seconds, const char* own_module, Emit&& emit) {
    auto& s = state();
    if (!enabled()) {
        return;
    }
    const auto written = s.written.load(std::memory_order_acquire);
    LARGE_INTEGER frequency{};
    LARGE_INTEGER now{};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&now);
    const auto window = static_cast<uint64_t>(seconds * static_cast<double>(frequency.QuadPart));
    const uint64_t first = written > kRingSamples ? written - kRingSamples : 0;

    const auto resolve = [](uint64_t address) -> std::string {
        const ModuleRef module = module_for(address);
        char text[160]{};
        std::snprintf(text, sizeof(text), "%s+0x%llX", module.name.c_str(),
            static_cast<unsigned long long>(address - module.base));
        return text;
    };

    std::map<std::string, uint32_t> leaf;
    std::map<std::string, uint32_t> leaf_module;
    std::map<std::string, uint32_t> own_first;
    std::map<std::string, uint32_t> own_inclusive;
    uint32_t total{};
    uint32_t with_own{};
    for (uint64_t i = first; i < written; ++i) {
        const Sample sample = s.ring[i % kRingSamples];
        if (sample.depth == 0 ||
            static_cast<uint64_t>(now.QuadPart) - sample.qpc > window) {
            continue;
        }
        ++total;
        const std::string leaf_name = resolve(sample.frames[0]);
        ++leaf[leaf_name];
        ++leaf_module[leaf_name.substr(0, leaf_name.find('+'))];
        bool own_seen{};
        std::vector<std::string> seen;
        for (uint32_t d = 0; d < sample.depth; ++d) {
            const std::string name = resolve(sample.frames[d]);
            if (name.rfind(own_module, 0) != 0) {
                continue;
            }
            if (!own_seen) {
                ++own_first[name];
                own_seen = true;
            }
            if (std::find(seen.begin(), seen.end(), name) == seen.end()) {
                ++own_inclusive[name];
                seen.push_back(name);
            }
        }
        if (own_seen) {
            ++with_own;
        }
    }

    const auto top = [&](const char* title, const std::map<std::string, uint32_t>& table,
                         size_t limit) {
        std::vector<std::pair<uint32_t, std::string>> rows;
        for (const auto& [name, count] : table) {
            rows.emplace_back(count, name);
        }
        std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });
        char line[256]{};
        for (size_t i = 0; i < rows.size() && i < limit; ++i) {
            std::snprintf(line, sizeof(line), "Stack sampler %s %5.1f%% %u %s", title,
                total != 0 ? 100.0 * rows[i].first / total : 0.0, rows[i].first,
                rows[i].second.c_str());
            emit(line);
        }
    };

    char header[256]{};
    std::snprintf(header, sizeof(header),
        "Stack sampler report tid=%lu window_s=%.1f samples=%u with_%s=%u (%.1f%%) failed=%llu",
        static_cast<unsigned long>(s.target_tid.load()), seconds, total, own_module, with_own,
        total != 0 ? 100.0 * with_own / total : 0.0,
        static_cast<unsigned long long>(s.failed.load()));
    emit(header);
    top("leaf_module", leaf_module, 12);
    top("leaf", leaf, 40);
    top("own_first", own_first, 40);
    top("own_inclusive", own_inclusive, 40);
}

}  // namespace w3vr::thread_stack_sampler
