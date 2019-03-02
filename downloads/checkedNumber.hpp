#pragma once
#include <type_traits>
#include <exception>
#include <cassert>
#include <string>
#include <functional>
#include <istream>
#include <initializer_list>

template <typename Number, typename Derived>
class CheckedNumber {
public:
    using ThisClass = CheckedNumber<Number, Derived>;

    class InvariantViolatedException: public std::exception {
    public:
        InvariantViolatedException(
            const std::string& invariantDescription
        ): exceptionMessage_M("Invariant " + invariantDescription + " violated!") {
        }

        const char*
        what(
        ) const noexcept {
            return exceptionMessage_M.c_str();
        }

    private:
        const std::string exceptionMessage_M;
    };

    operator Number(
    ) const {
        return value_M;
    }

    explicit operator const Number&(
    ) const {
        return value_M;
    }

    explicit operator Number&(
    ) {
        return value_M;
    }

    ThisClass& operator =(Number other) {
        value_M = std::move(other);
        checkInvariant();
        return *this;
    }


    ThisClass& operator +=(const Number& other) {
        value_M += other;
        checkInvariant();
        return *this;
    }


    ThisClass& operator -=(const Number& other) {
        value_M -= other;
        checkInvariant();
        return *this;
    }

    ThisClass& operator *=(const Number& other) {
        value_M *= other;
        checkInvariant();
        return *this;
    }

    ThisClass& operator /=(const Number& other) {
        value_M /= other;
        checkInvariant();
        return *this;
    }

    ThisClass& operator++() {
        ++value_M;
        checkInvariant();
        return *this;
    }

    ThisClass operator++(int) {
        const Number oldValue = value_M;
        operator++();
        return *this;
    }

    ThisClass& operator--() {
        --value_M;
        checkInvariant();
        return *this;    }

    Number operator--(int) {
        const Number oldValue = value_M;
        operator--();
        return oldValue;
    }

    template <typename OtherNumber = Number, typename = typename std::enable_if<std::is_integral<OtherNumber>::value && std::is_integral<Number>::value, void>::type>
    ThisClass& operator %=(const OtherNumber& other) {
        value_M %= other;
        checkInvariant();
        return *this;
    }

    template <typename OtherNumber = Number, typename = typename std::enable_if<std::is_integral<OtherNumber>::value && std::is_integral<Number>::value, void>::type>
    ThisClass& operator |=(const OtherNumber& other) {
        value_M |= other;
        checkInvariant();
        return *this;
    }

    template <typename OtherNumber = Number, typename = typename std::enable_if<std::is_integral<OtherNumber>::value && std::is_integral<Number>::value, void>::type>
    ThisClass& operator &=(const OtherNumber& other) {
        value_M &= other;
        checkInvariant();
        return *this;
    }

    template <typename OtherNumber = Number, typename = typename std::enable_if<std::is_integral<OtherNumber>::value && std::is_integral<Number>::value, void>::type>
    ThisClass& operator <<=(const OtherNumber& other) {
        value_M <<= other;
        checkInvariant();
        return *this;
    }

    template <typename OtherNumber = Number, typename = typename std::enable_if<std::is_integral<OtherNumber>::value && std::is_integral<Number>::value, void>::type>
    ThisClass& operator >>=(const OtherNumber& other) {
        value_M >>= other;
        checkInvariant();
        return *this;
    }


protected:
    CheckedNumber(Number value, const bool doCheck = true):
        value_M(std::move(value)) {
        if (doCheck) {
            checkInvariant();
        }
    }

    void checkInvariant() {
        const Derived& derivedInstance = static_cast<const Derived&>(*this);
        if (!derivedInstance.invariant(value_M)) {
            throw InvariantViolatedException(derivedInstance.invariantDescription(value_M));
        }
    }
private:
    Number value_M;
}; // End of CheckedNumber

template <typename Number>
class PositiveNumber: public CheckedNumber<Number, PositiveNumber<Number>> {
public:
    using Super = CheckedNumber<Number, PositiveNumber<Number>>;
    friend class CheckedNumber<Number, PositiveNumber<Number>>;

    PositiveNumber(Number number):
        Super(number) {
    }
private:
    bool invariant(const Number& value) const {
        return value > 0;
    }

    std::string invariantDescription(const Number& value) const {
        return std::to_string(value) + " > 0";
    }
};

template <typename Number, typename Derived>
std::istream& operator>>(std::istream& istream, CheckedNumber<Number, Derived>& number) {
    return istream >> static_cast<Number&>(number);
}


template <typename Number>
class NumberInClosedInterval: public CheckedNumber<Number, NumberInClosedInterval<Number>> {
public:
    using Super = CheckedNumber<Number, NumberInClosedInterval<Number>>;
    friend class CheckedNumber<Number, NumberInClosedInterval<Number>>;

    NumberInClosedInterval(Number number, Number lowerBound, Number upperBound):
        Super(number, false), lowerBound_M(std::move(lowerBound)), upperBound_M(std::move(upperBound)) {
        Super::checkInvariant();
    }

private:
    Number lowerBound_M;
    Number upperBound_M;

    bool invariant(const Number& value) const {
        return value >= lowerBound_M && value <= upperBound_M;
    }

    std::string invariantDescription(const Number& value) const {
        return std::to_string(lowerBound_M) + " <= " + std::to_string(value) + " <= " + std::to_string(upperBound_M);
    }
};
