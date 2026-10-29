#pragma once

#include <cstdint>

namespace host_clock {
inline std::uint32_t nowMs = 0;
}
inline unsigned long millis() { return host_clock::nowMs; }

struct HostSerial {
    void begin(unsigned long) {}
    template <typename... Args>
    void printf(const char*, Args...) {}
};
inline HostSerial Serial;
