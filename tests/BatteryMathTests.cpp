#include "battery_math.h"

#include <cassert>

int main() {
    using umbm::NormalizeBattery;

    assert(NormalizeBattery(0, 0, 100) == 0);
    assert(NormalizeBattery(50, 0, 100) == 50);
    assert(NormalizeBattery(100, 0, 100) == 100);
    assert(NormalizeBattery(127, 0, 255) == 50);
    assert(NormalizeBattery(128, 0, 255) == 50);
    assert(!NormalizeBattery(101, 0, 100));
    assert(!NormalizeBattery(10, 10, 10));
    assert(NormalizeBattery(20, 20, 120) == 0);
    assert(NormalizeBattery(120, 20, 120) == 100);

    return 0;
}
