#include <iostream>
#include "units.hpp"

// Define unit types
SU_DURATION_UNIT(second_t, "s")
SU_UNIT(joule_t, "J")
SU_UNIT(watt_t, "W")

// Define relations between these units
// In this case, seconds * watts = joules
// This will also define any derived relations
// e.g. joules / watts = seconds
SU_MUL(second_t, watt_t, joule_t)

// su::unit works almost exactly like std::chrono::duration,
// except it takes an additional template parameter indicating
// the type of each unit
using second = su::unit<second_t, int64_t>;

using watt = su::unit<watt_t, int64_t>;
using kilowatt = su::unit<watt_t, int64_t, std::kilo>;
using kilowatt_d = su::unit<watt_t, double, std::kilo>;

using joule = su::unit<joule_t, int64_t>;
using megajoule_d = su::unit<joule_t, double, std::mega>;

int main() {
    constexpr auto pc_power = watt(500);
    constexpr auto kettle_power = kilowatt(2);
    static_assert(kettle_power + pc_power == watt(2500));

    constexpr auto total_power_kw = su::unit_cast<kilowatt_d>(kettle_power + pc_power);
    std::cout << total_power_kw << std::endl; // 2.5kW

    constexpr int64_t power_ratio = kettle_power / pc_power;
    static_assert(power_ratio == 4);

    constexpr auto duration = second(4);
    constexpr auto std_duration = std::chrono::seconds(duration);
    static_assert(std_duration == std::chrono::seconds(4));

    constexpr auto energy_used = (kettle_power + pc_power) * duration;
    static_assert(energy_used == megajoule_d(0.01));
}