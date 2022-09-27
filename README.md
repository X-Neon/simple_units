# Simple Units

Simple Units is a C++20 header-only library for strongly typed units. Plenty of other units libraries already exist, but I find they are often overly complex. Their heavy use of template metaprogramming heavily impacts compile times, and causes issues with IDE features such as type deduction, autocompletion, etc. Simple Units is a much smaller and simpler in comparison, and is focused on providing a pleasant developer experience with a minimal subset of features that satisfy the majority of use cases. The API is nearly identical to the `std::chrono::duration` API, so it be immediately familiar to most C++ users.

## Example

```cpp
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
```

## API

### Unit

```cpp
template <typename Tag, typename Rep, typename Scale = std::ratio<1>>
class unit
{
public:
    using tag = Tag;
    using rep = Rep;
    using scale = Scale;

    constexpr unit() = default;
    unit(const unit&) = default;

    template <typename Rep2>
    constexpr explicit unit(const Rep2& v);

    template <typename Rep2, typename Scale2>
    constexpr unit(const unit<Tag, Rep2, Scale2>& u);

    static constexpr unit zero();
    static constexpr unit min();
    static constexpr unit max();

    constexpr unit operator+() const;
    constexpr unit operator-() const;

    constexpr unit& operator+=(const unit& u);
    constexpr unit& operator-=(const unit& u);
    constexpr unit& operator*=(const Rep& v);
    constexpr unit& operator/=(const Rep& v);
    constexpr unit& operator%=(const unit& u);
    constexpr unit& operator%=(const Rep& v);

    // Returns the internal value with no additional scaling applied
    constexpr Rep count() const;

    // Returns the true value represented, taking into account the unit scaling
    template <typename Rep2 = double>
    constexpr Rep2 value() const;

    // If Tag is a duration type
    template <typename Rep2, typename Scale2>
    constexpr unit(const std::chrono::duration<Rep2, Scale2>& d);

    // If Tag is a duration type
    template <typename Rep2, typename Scale2>
    constexpr operator std::chrono::duration<Rep2, Scale2>() const;
};
```

### Macros

```cpp
// Defines lhs_1 * lhs_2 = rhs
#define SU_MUL(lhs_1, lhs_2, rhs)

// Defines lhs_1 * lhs_2 = rhs
#define SU_DIV(lhs_1, lhs_2, rhs)

// Defines lhs = rhs^(-1)
#define SU_INV(lhs, rhs)

// Creates a unit called "name" with a printable symbol "symbol_str"
#define SU_UNIT(name, symbol_str)

// Creates a unit called "name" with a printable symbol "symbol_str"
// This unit is implicitely convertible to std::chrono::duration
#define SU_DURATION_UNIT(name, symbol_str)
```

### Other

```cpp
template <typename Tag, typename Scale = std::ratio<1>>
using unit_d = unit<Tag, double, Scale>;

template <typename Tag, typename Scale = std::ratio<1>>
using unit_i = unit<Tag, int64_t, Scale>;

template <typename Rep, typename Scale>
using quantity = unit<void, Rep, Scale>;

constexpr quantity<int64_t, std::nano> as_nano(1'000'000'000);
constexpr quantity<int64_t, std::nano> as_micro(1'000'000);
constexpr quantity<int64_t, std::nano> as_milli(1'000);

template <typename To, typename Tag, typename Rep, typename Scale>
constexpr To unit_cast(const unit<Tag, Rep, Scale>& u);
```

### Operators

```cpp
template <typename Tag1, typename Rep1, typename Scale1, typename Tag2, typename Rep2, typename Scale2>
constexpr auto operator*(const unit<Tag1, Rep1, Scale1>& a, const unit<Tag2, Rep2, Scale2>& b);

template <typename T, typename Tag, typename Rep, typename Scale>
constexpr auto operator*(const unit<Tag, Rep, Scale>& a, const T& b);

template <typename T, typename Tag, typename Rep, typename Scale>
constexpr auto operator*(const T& a, const unit<Tag, Rep, Scale>& b);

template <typename Tag1, typename Rep1, typename Scale1, typename Tag2, typename Rep2, typename Scale2>
constexpr auto operator/(const unit<Tag1, Rep1, Scale1>& a, const unit<Tag2, Rep2, Scale2>& b);

template <typename T, typename Tag, typename Rep, typename Scale>
constexpr auto operator/(const unit<Tag, Rep, Scale>& a, const T& b);

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator%(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b);

template <typename T, typename Tag, typename Rep, typename Scale>
constexpr auto operator%(const unit<Tag, Rep, Scale>& a, const T& b);

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator+(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b);

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator-(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b);

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr bool operator==(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b);

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator<=>(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b);

template <typename Tag, typename Rep, typename Scale>
std::ostream& operator<<(std::ostream& s, const unit<Tag, Rep, Scale>& u);
```