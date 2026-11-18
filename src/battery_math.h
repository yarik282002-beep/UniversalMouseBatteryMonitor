#pragma once

#include <algorithm>
#include <optional>

namespace umbm {

inline std::optional<int> NormalizeBattery(unsigned long raw, long logicalMin, long logicalMax) noexcept {
    if (logicalMax <= logicalMin) {
        return std::nullopt;
    }

    const auto value = static_cast<long long>(raw);
    if (value < logicalMin || value > logicalMax) {
        return std::nullopt;
    }

    const auto numerator = (value - logicalMin) * 100LL;
    const auto denominator = static_cast<long long>(logicalMax) - logicalMin;
    const auto rounded = (numerator + denominator / 2LL) / denominator;

    return std::clamp(static_cast<int>(rounded), 0, 100);
}

} // namespace umbm
