#include "units.hpp"

SU_DURATION_UNIT(second_t, "s")
SU_UNIT(hz_t, "Hz")
SU_UNIT(joule_t, "J")
SU_UNIT(watt_t, "W")

SU_INV(second_t, hz_t)
SU_MUL(second_t, watt_t, joule_t)

template <typename Rep, typename Scale = std::ratio<1>>
using second = su::unit<second_t, Rep, Scale>;

template <typename Rep, typename Scale = std::ratio<1>>
using hz = su::unit<hz_t, Rep, Scale>;

template <typename Rep, typename Scale = std::ratio<1>>
using watt = su::unit<watt_t, Rep, Scale>;

template <typename Rep, typename Scale = std::ratio<1>>
using joule = su::unit<joule_t, Rep, Scale>;

int main()
{
    static_assert(second<int64_t>(5).count() == 5);
    static_assert(second<int64_t>(5).value() == 5);

    static_assert(second<int64_t, std::kilo>(5) == second<int64_t>(5000));
    static_assert(second<int64_t, std::kilo>(5).count() == 5);
    static_assert(second<int64_t, std::kilo>(5).value() == 5000);

    static_assert(second<int16_t, std::mega>(5) == second<int64_t>(5'000'000));

    static_assert(second<double, std::kilo>(0.5) == second<int64_t>(500));

    static_assert(second<int64_t>(6) / second<int64_t>(3) == 2);
    static_assert(second<int64_t>(1) / second<int64_t>(2) == 0);
    static_assert(second<double>(1) / second<int64_t>(2) == 0.5);
    static_assert(second<int64_t, std::kilo>(1) / second<int64_t>(2) == 500);

    static_assert(second<int64_t>(6) / 3 == second<int64_t>(2));
    static_assert(second<int64_t>(3) * 2 == second<int64_t>(6));
    static_assert(second<int64_t>(3) + second<int64_t>(2) == second<int64_t>(5));
    static_assert(second<int64_t>(5) - second<int64_t>(2) == second<int64_t>(3));
    static_assert(second<int64_t, std::kilo>(1) + second<int64_t>(1) == second<int64_t>(1001));
    static_assert(second<int64_t, std::kilo>(1) - second<int64_t>(1) == second<int64_t>(999));
    static_assert(second<int64_t>(3) % second<int64_t>(2) == second<int64_t>(1));
    static_assert(second<int64_t>(3) % 2 == second<int64_t>(1));

    static_assert(second<int64_t>(3) * hz<int64_t>(2) == 6);
    static_assert(hz<int64_t>(2) * second<int64_t>(3) == 6);
    static_assert(second<int64_t, std::kilo>(3) * hz<int64_t>(2) == 6000);
    static_assert(second<int64_t, std::kilo>(3) * hz<int64_t, std::milli>(2) == 6);

    static_assert(second<int64_t>(3) * watt<int64_t>(2) == joule<int64_t>(6));
    static_assert(watt<int64_t>(2) * second<int64_t>(3) == joule<int64_t>(6));
    static_assert(second<int64_t, std::kilo>(3) * watt<int64_t>(2) == joule<int64_t>(6000));
    static_assert(second<int64_t, std::kilo>(3) * watt<int64_t, std::milli>(2) == joule<int64_t>(6));

    static_assert(su::as_nano / hz<int64_t>(20'000'000) == second<int64_t, std::nano>(50));

    static_assert(std::chrono::seconds(second<int64_t>(5)) == std::chrono::seconds(5));
    static_assert(second<int64_t>(5) == second<int64_t>(std::chrono::seconds(5)));
    static_assert(std::chrono::seconds(second<int64_t, std::kilo>(5)) == std::chrono::seconds(5000));
    static_assert(second<int64_t, std::kilo>(5) == second<int64_t>(std::chrono::seconds(5000)));
}