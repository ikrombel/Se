#pragma once

#include <array>

#include <type_traits>

#if __cplusplus < 202002L && __cpp_generic_lambdas < 201707L
namespace std {
template< class T, std::size_t N >
constexpr std::array<std::remove_cv_t<T>, N> to_array( T (&a)[N] );
template< class T, std::size_t N >
constexpr std::array<std::remove_cv_t<T>, N> to_array( T (&&a)[N] );
}
#endif

namespace Se
{

/// Make bitwise operators (| & ^ ~) automatically construct FlagSet from Enum.
#define SE_AUTOMATIC_FLAGSET(Enum) \
    constexpr inline Se::FlagSet<Enum> operator | (const Enum lhs, const Enum rhs) { return Se::FlagSet<Enum>(lhs) | rhs; } \
    constexpr inline Se::FlagSet<Enum> operator & (const Enum lhs, const Enum rhs) { return Se::FlagSet<Enum>(lhs) & rhs; } \
    constexpr inline Se::FlagSet<Enum> operator ^ (const Enum lhs, const Enum rhs) { return Se::FlagSet<Enum>(lhs) ^ rhs; } \
    constexpr inline Se::FlagSet<Enum> operator ~ (const Enum rhs) { return ~Se::FlagSet<Enum>(rhs); }

/// Declare FlagSet for specific enum and create operators for automatic FlagSet construction.
#define SE_FLAGSET(enumName, flagsetName) \
    SE_AUTOMATIC_FLAGSET(enumName) \
    using flagsetName = Se::FlagSet<enumName>

/// A set of flags defined by an Enum.
template <class E>
class FlagSet
{
public:
    /// Enum type.
    using Enum = E;
    /// Integer type.
    using Integer = typename std::underlying_type<Enum>::type;

public:
    /// Ctor by integer.
    constexpr explicit FlagSet(Integer value)
        : value_(value)
    {
    }
    /// Empty constructor.
    constexpr FlagSet() = default;

    /// Copy constructor.
    constexpr FlagSet(const FlagSet& another) = default;

    /// Construct from Enum value.
    constexpr FlagSet(const Enum value)
        : value_(static_cast<Integer>(value))
    {
    }

    /// Assignment operator from flagset.
    constexpr FlagSet& operator = (const FlagSet& rhs) = default;

    /// Bitwise AND against Enum value.
    constexpr FlagSet& operator &= (const Enum value)
    {
        value_ &= static_cast<Integer>(value);
        return *this;
    }

    /// Bitwise AND against flagset value.
    constexpr FlagSet& operator &= (const FlagSet value)
    {
        value_ &= value.value_;
        return *this;
    }

    /// Bitwise OR against Enum value.
    constexpr FlagSet& operator |= (const Enum value)
    {
        value_ |= static_cast<Integer>(value);
        return *this;
    }

    /// Bitwise OR against flagset value.
    constexpr FlagSet& operator |= (const FlagSet value)
    {
        value_ |= value.value_;
        return *this;
    }

    /// Bitwise XOR against Enum value.
    constexpr FlagSet& operator ^= (const Enum value)
    {
        value_ ^= static_cast<Integer>(value);
        return *this;
    }

    /// Bitwise XOR against flagset value.
    constexpr FlagSet& operator ^= (const FlagSet value)
    {
        value_ ^= value.value_;
        return *this;
    }

    /// Bitwise AND against Enum value.
    constexpr FlagSet operator & (const Enum value) const
    {
        return FlagSet(value_ & static_cast<Integer>(value));
    }

    /// Bitwise AND against flagset value.
    constexpr FlagSet operator & (const FlagSet value) const
    {
        return FlagSet(value_ & value.value_);
    }

    /// Bitwise OR against Enum value.
    constexpr FlagSet operator | (const Enum value) const
    {
        return FlagSet(value_ | static_cast<Integer>(value));
    }

    /// Bitwise OR against flagset value.
    constexpr FlagSet operator | (const FlagSet value) const
    {
        return FlagSet(value_ | value.value_);
    }

    /// Bitwise XOR against Enum value.
    constexpr FlagSet operator ^ (const Enum value) const
    {
        return FlagSet(value_ ^ static_cast<Integer>(value));
    }

    /// Bitwise XOR against flagset value.
    constexpr FlagSet operator ^ (const FlagSet value) const
    {
        return FlagSet(value_ ^ value.value_);
    }

    /// Bitwise negation.
    constexpr FlagSet operator ~ () const
    {
        return FlagSet(~value_);
    }

    /// Boolean negation.
    constexpr bool operator ! () const
    {
        return !value_;
    }

    /// Returns true if any flag is set.
    constexpr explicit operator bool () const
    {
        return value_ != 0;
    }

    /// Cast to underlying type of enum.
    constexpr operator Integer() const
    {
        return value_;
    }

    /// Cast to enum value.
    constexpr explicit operator Enum() const
    {
        return static_cast<Enum>(value_);
    }

    /// Cast to double. Used by Lua bindings.
    explicit operator double() const
    {
        return static_cast<double>(value_);
    }

    /// Equality check against enum value.
    constexpr bool operator ==(Enum rhs) const
    {
        return value_ == static_cast<Integer>(rhs);
    }

    /// Equality check against another flagset.
    constexpr bool operator ==(FlagSet rhs) const
    {
        return value_ == rhs.value_;
    }

    /// Non-equality check against enum value.
    constexpr bool operator !=(Enum rhs) const
    {
        return !(*this == rhs);
    }

    /// Non-equality check against another flagset value.
    constexpr bool operator !=(FlagSet rhs) const
    {
        return !(*this == rhs);
    }

    /// Return true if specified enum value is set.
    constexpr bool Test(const Enum value) const
    {
        return Test(static_cast<Integer>(value));
    }

    /// Return true if specified bits are set.
    constexpr bool Test(const Integer flags) const
    {
        return (value_ & flags) == flags && (flags != 0 || value_ == flags);
    }

    /// Set or unset specified subset of flags
    /// @{
    constexpr void Set(const Integer flags, bool enabled = true)
    {
        if (enabled)
            value_ |= flags;
        else
            value_ &= ~flags;
    }

    constexpr void Set(const Enum value, bool enabled = true)
    {
        Set(static_cast<Integer>(value), enabled);
    }

    constexpr void Unset(const Integer flags)
    {
        Set(flags, false);
    }

    constexpr void Unset(const Enum value)
    {
        Set(value, false);
    }
    /// @}

    /// Return underlying integer (constant).
    constexpr Integer AsInteger() const { return value_; }

    /// Return underlying integer (non-constant).
    constexpr Integer& AsInteger() { return value_; }

    /// Return hash value.
    constexpr unsigned ToHash() const { return static_cast<unsigned>(value_); }

protected:
    /// Value.
    Integer value_ = 0;
};

/// Fixed-size array indexed by enum.
template <class T, class E, std::size_t Size = static_cast<std::size_t>(E::Count)>
class EnumArray : public std::array<T, Size>
{
public:
    /// Enum type.
    using enum_type = E;
    /// Integer type.
    using underlying_integer_type = typename std::underlying_type<enum_type>::type;
    /// Base array type.
    using base_type = std::array<T, Size>;

    /// Inherit constructors and operators from base class.
    /// @{
    constexpr EnumArray(const EnumArray& other) = default;
    constexpr EnumArray(EnumArray&& other) = default;
    constexpr EnumArray& operator=(const EnumArray& other) = default;
    constexpr EnumArray& operator=(EnumArray&& other) = default;

    constexpr bool operator==(const EnumArray& rhs) const
    {
        return static_cast<const base_type&>(*this) == static_cast<const base_type&>(rhs);
    }

    constexpr bool operator!=(const EnumArray& rhs) const { return !(*this == rhs); }
    /// @}

    /// Construct.
    /// @{
    constexpr EnumArray()
        : base_type{}
    {
    }
    constexpr EnumArray(const T& value)
        : base_type{}
    {
        this->fill(value);
    }
    template <std::size_t N>
    constexpr EnumArray(const T (&values)[N])
        : base_type{std::to_array(values)}
    {
    }
    /// @}

    /// Access.
    /// @{
    constexpr typename base_type::reference operator[](enum_type i)
    {
        return base_type::operator[](static_cast<underlying_integer_type>(i));
    }
    constexpr typename base_type::const_reference operator[](enum_type i) const
    {
        return base_type::operator[](static_cast<underlying_integer_type>(i));
    }
    constexpr typename base_type::const_reference at(enum_type i) const
    {
        return base_type::at(static_cast<underlying_integer_type>(i));
    }
    constexpr typename base_type::reference at(enum_type i)
    {
        return base_type::at(static_cast<underlying_integer_type>(i));
    }
    /// @}
};

} // namespace Se
