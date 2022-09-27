#pragma once

#include <ratio>
#include <concepts>
#include <limits>
#include <ostream>
#include <chrono>

#define SU_MUL(lhs_1, lhs_2, rhs) \
    namespace su::ops { \
        template <> \
        struct div<rhs, lhs_1> { using type = lhs_2; }; \
        template <> \
        struct div<rhs, lhs_2> { using type = lhs_1; }; \
        template <> \
        struct mul<lhs_1, lhs_2> { using type = rhs; }; \
        template <> \
        struct mul<lhs_2, lhs_1> { using type = rhs; }; \
    }

#define SU_DIV(lhs_1, lhs_2, rhs) SU_MUL(rhs, lhs_2, lhs_1)
#define SU_INV(lhs, rhs) SU_MUL(lhs, rhs, void)

#define SU_UNIT(name, symbol_str) \
    struct name { static constexpr auto symbol = symbol_str; }; 

#define SU_DURATION_UNIT(name, symbol_str) \
    struct name { static constexpr auto symbol = symbol_str; }; \
    namespace su { \
        template <> \
        struct is_duration_type<name> : std::true_type {}; \
    }

namespace su
{

template <typename Rep>
struct treat_as_floating_point : std::is_floating_point<Rep> {};

template <typename Tag>
struct is_duration_type : std::false_type {};

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
    requires treat_as_floating_point<Rep>::value || (!treat_as_floating_point<Rep2>::value)
    constexpr explicit unit(const Rep2& v) : m_val(v) {}

    template <typename Rep2, typename Scale2>
    requires treat_as_floating_point<Rep>::value || 
        (std::ratio_divide<Scale2, Scale>::den == 1 && !treat_as_floating_point<Rep2>::value)
    constexpr unit(const unit<Tag, Rep2, Scale2>& u) : m_val(unit_cast<unit>(u).count()) {}

    template <typename Rep2, typename Scale2>
    requires is_duration_type<Tag>::value && 
        (treat_as_floating_point<Rep>::value || (std::ratio_divide<Scale2, Scale>::den == 1 && !treat_as_floating_point<Rep2>::value))
    constexpr unit(const std::chrono::duration<Rep2, Scale2>& d) : 
        m_val(std::chrono::duration_cast<std::chrono::duration<Rep, Scale>>(d).count()) {}

    static constexpr unit zero() { return unit(0); }
    static constexpr unit min() { return unit(std::numeric_limits<Rep>::min()); }
    static constexpr unit max() { return unit(std::numeric_limits<Rep>::max()); }

    constexpr unit operator+() const { return *this; }
    constexpr unit operator-() const { return unit(-m_val); }

    constexpr unit& operator+=(const unit& u) { m_val += u.m_val; }
    constexpr unit& operator-=(const unit& u) { m_val -= u.m_val; }
    constexpr unit& operator*=(const Rep& v) { m_val *= v; }
    constexpr unit& operator/=(const Rep& v) { m_val /= v; }
    constexpr unit& operator%=(const unit& u) { m_val %= u.m_val; }
    constexpr unit& operator%=(const Rep& v) { m_val %= v; }

    constexpr Rep count() const {
        return m_val;
    }

    template <typename Rep2 = double>
    constexpr Rep2 value() const {
        return unit_cast<unit<Tag, Rep2>>(*this).count();
    }

    template <typename Rep2, typename Scale2>
    requires is_duration_type<Tag>::value
    constexpr operator std::chrono::duration<Rep2, Scale2>() const {
        return std::chrono::duration<Rep2, Scale2>(unit_cast<unit<Tag, Rep2, Scale2>>(*this).count());
    }

private:
    Rep m_val;
};

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
requires std::same_as<typename To::tag, Tag>
constexpr To unit_cast(const unit<Tag, Rep, Scale>& u) {
    using R = std::ratio_divide<typename To::scale, Scale>;
    std::common_type_t<typename To::rep, Rep> v = u.count();
    return To((v * R::den) / R::num);
}

namespace ops
{

template <typename T, typename U>
struct mul {};

template <typename T>
struct mul<T, void> { using type = T; };

template <typename T>
struct mul<void, T> { using type = T; };

template <>
struct mul<void, void> { using type = void; };

template <typename T, typename U>
struct div {};

template <typename T>
struct div<T, T> { using type = void; };

template <typename T>
struct div<T, void> { using type = T; };

} // namespace ops

template <typename Tag1, typename Rep1, typename Scale1, typename Tag2, typename Rep2, typename Scale2>
requires requires { typename ops::mul<Tag1, Tag2>::type; }
constexpr auto operator*(const unit<Tag1, Rep1, Scale1>& a, const unit<Tag2, Rep2, Scale2>& b) {
    std::common_type_t<Rep1, Rep2> v = a.count() * b.count();

    if constexpr (std::is_void_v<typename ops::mul<Tag1, Tag2>::type>) {
        using S = std::ratio_multiply<Scale1, Scale2>;
        return std::common_type_t<Rep1, Rep2>((v * S::num) / S::den);
    } else {
        return unit<typename ops::mul<Tag1, Tag2>::type, std::common_type_t<Rep1, Rep2>, typename std::ratio_multiply<Scale1, Scale2>::type>(v);
    }
}

template <typename T, typename Tag, typename Rep, typename Scale>
requires requires { typename std::common_type<Rep, T>::type; }
constexpr auto operator*(const unit<Tag, Rep, Scale>& a, const T& b) {
    std::common_type_t<Rep, T> v = a.count() * b;
    return unit<Tag, decltype(v), Scale>(v);
}

template <typename T, typename Tag, typename Rep, typename Scale>
requires requires { typename std::common_type<Rep, T>::type; }
constexpr auto operator*(const T& a, const unit<Tag, Rep, Scale>& b) {
    return b * a;
}

template <typename Tag1, typename Rep1, typename Scale1, typename Tag2, typename Rep2, typename Scale2>
requires requires { typename ops::div<Tag1, Tag2>::type; }
constexpr auto operator/(const unit<Tag1, Rep1, Scale1>& a, const unit<Tag2, Rep2, Scale2>& b) {
    if constexpr (std::is_void_v<typename ops::div<Tag1, Tag2>::type>) {
        using S = std::ratio_divide<Scale1, Scale2>;
        return std::common_type_t<Rep1, Rep2>(S::num * a.count()) / (S::den * b.count());
    } else {
        std::common_type_t<Rep1, Rep2> v = a.count() / b.count();
        return unit<typename ops::div<Tag1, Tag2>::type, std::common_type_t<Rep1, Rep2>, typename std::ratio_divide<Scale1, Scale2>::type>(v);
    }
}

template <typename T, typename Tag, typename Rep, typename Scale>
requires requires { typename std::common_type<Rep, T>::type; }
constexpr auto operator/(const unit<Tag, Rep, Scale>& a, const T& b) {
    std::common_type_t<Rep, T> v = a.count() / b;
    return unit<Tag, decltype(v), Scale>(v);
}

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator%(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b) {
    using T = std::common_type_t<unit<Tag, Rep1, Scale1>, unit<Tag, Rep2, Scale2>>;
    return T(unit_cast<T>(a).count() % unit_cast<T>(b).count());
}

template <typename T, typename Tag, typename Rep, typename Scale>
requires requires { typename std::common_type<Rep, T>::type; }
constexpr auto operator%(const unit<Tag, Rep, Scale>& a, const T& b) {
    std::common_type_t<Rep, T> v = a.count() % b;
    return unit<Tag, decltype(v), Scale>(v);
}

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator+(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b) {
    using T = std::common_type_t<unit<Tag, Rep1, Scale1>, unit<Tag, Rep2, Scale2>>;
    return T(unit_cast<T>(a).count() + unit_cast<T>(b).count());
}

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator-(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b) {
    using T = std::common_type_t<unit<Tag, Rep1, Scale1>, unit<Tag, Rep2, Scale2>>;
    return T(unit_cast<T>(a).count() - unit_cast<T>(b).count());
}

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr bool operator==(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b) {
    using T = std::common_type_t<unit<Tag, Rep1, Scale1>, unit<Tag, Rep2, Scale2>>;
    return unit_cast<T>(a).count() == unit_cast<T>(b).count();
}

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
constexpr auto operator<=>(const unit<Tag, Rep1, Scale1>& a, const unit<Tag, Rep2, Scale2>& b) {
    using T = std::common_type_t<unit<Tag, Rep1, Scale1>, unit<Tag, Rep2, Scale2>>;
    return unit_cast<T>(a).count() <=> unit_cast<T>(b).count();
}

template <typename Tag, typename Rep, typename Scale>
requires requires { Tag::symbol; }
std::ostream& operator<<(std::ostream& s, const unit<Tag, Rep, Scale>& u) {
    s << u.count();

    if constexpr (std::is_same_v<Scale, std::exa>) { s << "E"; }
    else if constexpr (std::is_same_v<Scale, std::peta>) { s << "P"; }
    else if constexpr (std::is_same_v<Scale, std::tera>) { s << "T"; }
    else if constexpr (std::is_same_v<Scale, std::giga>) { s << "G"; }
    else if constexpr (std::is_same_v<Scale, std::mega>) { s << "M"; }
    else if constexpr (std::is_same_v<Scale, std::kilo>) { s << "k"; }
    else if constexpr (std::is_same_v<Scale, std::ratio<1>>) {}
    else if constexpr (std::is_same_v<Scale, std::milli>) { s << "m"; }
    else if constexpr (std::is_same_v<Scale, std::micro>) { s << "μ"; }
    else if constexpr (std::is_same_v<Scale, std::nano>) { s << "n"; }
    else if constexpr (std::is_same_v<Scale, std::pico>) { s << "p"; }
    else if constexpr (std::is_same_v<Scale, std::femto>) { s << "f"; }
    else if constexpr (std::is_same_v<Scale, std::atto>) { s << "a"; }
    else if constexpr (Scale::den == 1) { s << "[" << Scale::num << "]"; }
    else { s << "[" << Scale::num << "/" << Scale::den << "]"; }

    s << Tag::symbol;
    return s;
}

namespace detail
{

constexpr intmax_t abs(intmax_t a) {
    return a < 0 ? -a : a;
}

constexpr intmax_t gcd(intmax_t a, intmax_t b) {
    if (b == 0) {
        return abs(a);
    } else if (a == 0) {
        return abs(b);
    } else {
        return gcd(b, a % b);
    }
}

} // namespace detail

} // namespace su

namespace std
{

template <typename Tag, typename Rep, typename Scale>
struct common_type<su::unit<Tag, Rep, Scale>, su::unit<Tag, Rep, Scale>>
{
    using type = su::unit<Tag, Rep, Scale>;
};

template <typename Tag, typename Rep1, typename Scale1, typename Rep2, typename Scale2>
struct common_type<su::unit<Tag, Rep1, Scale1>, su::unit<Tag, Rep2, Scale2>>
{
    using Scale = ratio<su::detail::gcd(Scale1::num, Scale2::num), (Scale1::den / su::detail::gcd(Scale1::den, Scale2::den)) * Scale2::den>;
    using Rep = common_type_t<Rep1, Rep2>;
    using type = su::unit<Tag, Rep, Scale>;
};

} // namespace std
