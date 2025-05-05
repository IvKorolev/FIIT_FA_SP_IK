//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"
#include <ranges>
#include <exception>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>


constexpr size_t BASE = 1ULL << (8 * sizeof(unsigned int));


std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept {
    if (*this == other) return std::strong_ordering::equal;

    if (_sign != other._sign) {
        return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
    }

    bool compare_as_positive = _sign;

    if (_digits.size() != other._digits.size()) {
        if (_digits.size() < other._digits.size())
            return compare_as_positive ? std::strong_ordering::less : std::strong_ordering::greater;
        else
            return compare_as_positive ? std::strong_ordering::greater : std::strong_ordering::less;
    }

    for (size_t i = _digits.size(); i-- > 0;) {
        if (_digits[i] < other._digits[i])
            return compare_as_positive ? std::strong_ordering::less : std::strong_ordering::greater;
        if (_digits[i] > other._digits[i])
            return compare_as_positive ? std::strong_ordering::greater : std::strong_ordering::less;
    }

    return std::strong_ordering::equal;
}

big_int::operator bool() const noexcept
{
    return (!_digits.empty()) && !(_digits.size() == 1 && _digits[0] == 0);
}

big_int &big_int::operator++() &
{
    return operator+=(1);
}


big_int big_int::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

big_int &big_int::operator--() &
{
    return operator-=(1);
}


big_int big_int::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

big_int &big_int::operator+=(const big_int &other) &
{
    if (_sign && !other._sign) {
        return minus_assign(other);
    }
    return plus_assign(other);
}


big_int& big_int::operator-=(const big_int& other) & {
    // A - B

    if (_sign && !other._sign) {
        // A - (-B) = A + B
        return plus_assign(other);
    }

    if (!_sign && other._sign) {
        // (-A) - B = -(A + B)
        plus_assign(other);
        _sign = false;
        return *this;
    }

    if (_sign && other._sign) {
        // A - B
        if (*this >= other) {
            return minus_assign(other);
        } else {
            big_int tmp = other;
            tmp.minus_assign(*this);
            *this = std::move(tmp);
            _sign = false;
            return *this;
        }
    }

    // (-A) - (-B) = B - A
    if (*this >= other) {
        minus_assign(other);
        _sign = false;
        return *this;
    } else {
        big_int tmp = other;
        tmp.minus_assign(*this);
        *this = std::move(tmp);
        _sign = true;
        return *this;
    }
}


big_int big_int::operator+(const big_int &other) const
{
    auto tmp = *this;
    return tmp += other;
}

big_int big_int::operator-(const big_int &other) const
{
    auto tmp = *this;
    return tmp -= other;
}

big_int big_int::operator*(const big_int &other) const
{
    auto tmp = *this;
    return tmp *= other;
}

big_int big_int::operator/(const big_int &other) const
{
    auto tmp = *this;
    return tmp /= other;
}

big_int big_int::operator%(const big_int &other) const
{
    auto tmp = *this;
    return tmp %= other;
}

big_int big_int::operator&(const big_int &other) const
{
    auto tmp = *this;
    return tmp &= other;
}

big_int big_int::operator|(const big_int &other) const
{
    auto tmp = *this;
    return tmp |= other;
}

big_int big_int::operator^(const big_int &other) const
{
    auto tmp = *this;
    return tmp ^= other;
}

big_int big_int::operator<<(size_t shift) const
{
    auto tmp = *this;
    return tmp <<= shift;
}

big_int big_int::operator>>(size_t shift) const
{
    auto tmp = *this;
    return tmp >>= shift;
}

big_int &big_int::operator%=(const big_int &other) &
{
    return operator-=((*this / other) * other);
}

void big_int::optimise() noexcept
{
    while(!_digits.empty() && _digits.back() == 0)
        _digits.pop_back();

}

big_int big_int::operator~() const
{
    auto res = *this;
    res._sign = !res._sign;
    for (auto& num : res._digits)
    {
        num = ~num;
    }
    res.optimise();
    return res;
}

big_int &big_int::operator&=(const big_int &other) &
{
    if (!_sign && other._sign)
        _sign = true;

    for(size_t i = 0; i < _digits.size(); ++i)
        _digits[i] &= i < other._digits.size() ? other._digits[i] : 0;

    optimise();
    return *this;
}

big_int &big_int::operator|=(const big_int &other) &
{
    if (_sign != other._sign)
    {
        _sign = false;
    }

    if (_digits.size() < other._digits.size())
        _digits.resize(other._digits.size(), 0);

    for (size_t i = 0; i < _digits.size(); ++i)
    {
        _digits[i] |= other._digits[i];
    }
    optimise();
    return *this;
}

big_int &big_int::operator^=(const big_int &other) &
{
    if (_sign != other._sign)
    {
        _sign = true;
    }
    else
    {
        _sign = false;
    }

    if (_digits.size() < other._digits.size())
        _digits.resize(other._digits.size(), 0);

    for(size_t i = 0; i < _digits.size(); ++i)
        _digits[i] ^= other._digits[i];

    optimise();
    return *this;

}

big_int &big_int::operator<<=(size_t shift) &
{
    if (shift == 0)
        return *this;

    const size_t bits_per_digit = std::numeric_limits<unsigned int>::digits;
    const size_t digits_shift = shift / bits_per_digit;
    const size_t bits_shift = shift % bits_per_digit;

    if (digits_shift > 0)
        _digits.insert(_digits.begin(), digits_shift, 0);

    if (bits_shift > 0)
    {
        unsigned int carry = 0;
        for (auto &digit : _digits)
        {
            unsigned int tmp = digit;
            digit = (tmp << bits_shift) | carry;
            carry = tmp >> (bits_per_digit - bits_shift);
        }
        if (carry != 0)
            _digits.push_back(carry);
    }

    optimise();
    return *this;
}

big_int &big_int::operator>>=(size_t shift) &
{
    if (shift == 0)
        return *this;

    const size_t bits_per_digit = std::numeric_limits<unsigned int>::digits;
    const size_t digits_shift = shift / bits_per_digit;
    const size_t bits_shift = shift % bits_per_digit;

    if (digits_shift > 0)
    {
        if (digits_shift >= _digits.size())
        {
            _digits = {0};
            _sign = false;
            return *this;
        }
        _digits.erase(_digits.begin(), _digits.begin() + digits_shift);
    }

    if (bits_shift > 0)
    {
        unsigned int carry = 0;
        for (auto it = _digits.rbegin(); it != _digits.rend(); ++it)
        {
            unsigned int tmp = *it;
            *it = (tmp >> bits_shift) | carry;
            carry = tmp << (bits_per_digit - bits_shift);
        }
    }

    optimise();
    return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) &
{
    const unsigned int mask = __detail::generate_half_mask();
    if (_digits.size() < other._digits.size() + shift)
    {
        _digits.resize(other._digits.size() + shift, 0);
    }

    unsigned int carry = 0;

    for (size_t i = 0; i < other._digits.size(); ++i)
    {
        unsigned int a = _digits[i], b = i < shift ? 0 : i - shift < other._digits.size() ?  other._digits[i - shift] : 0;

        unsigned int sum = (a & mask) + (b & mask) + carry;
        carry = sum & (1u << (sizeof(unsigned int) * 4)) ? 1 : 0;
        sum &= mask;

        unsigned int sum_high = ((a >> (sizeof(unsigned int) * 4)) & mask) +
                                ((b >> (sizeof(unsigned int) * 4)) & mask) + carry;
        carry = sum_high & (1u << (sizeof(unsigned int) * 4)) ? 1 : 0;
        sum_high &= mask;

        _digits[i] = (sum_high << (sizeof(unsigned int) * 4)) + sum;
    }

    if (carry != 0)
    {
        _digits.push_back(carry);
    }

    optimise();
    return *this;
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) &
{
    if (_digits.size() < other._digits.size() + shift)
    {
        _digits.resize(other._digits.size() + shift, 0);
    }

    unsigned int borrow = 0;

    for (size_t i = 0; i < other._digits.size(); ++i)
    {
        unsigned int a = _digits[i], b = i < shift ? 0 : i - shift < other._digits.size() ? other._digits[i - shift] : 0;
        b += borrow;

        if (borrow != 0 && b == 0)
        {
            continue;
        }

        borrow = b > a;
        _digits[i] = a - b;
    }

    for (size_t i = other._digits.size(); borrow != 0 && i < _digits.size(); ++i)
    {
        unsigned int a = _digits[i];
        borrow = a > borrow ? 0 : 1;
        _digits[i] = a - borrow;
    }

    optimise();
    return *this;
}

big_int &big_int::operator*=(const big_int &other) &
{

    return multiply_assign(other, multiplication_rule::Karatsuba);
}

big_int &big_int::operator/=(const big_int &other) &
{
    return divide_assign(other);
}

std::string big_int::to_string() const
{
    if (_digits.empty() || _digits[0] == 0)
        return "0";

    std::stringstream res;
    auto tmp = *this;

    bool sign = tmp._sign;

    tmp._sign = true;

    while (tmp > 0_bi)
    {
        auto val = tmp % 10_bi;
        tmp /= 10;
        res << char('0' + (val._digits.empty() ? 0 : val._digits[0]));
    }

    if (!sign)
    {
        res << '-';
    }

    std::string d = res.str();
    std::reverse(d.begin(), d.end());
    return d;
}


std::ostream &operator<<(std::ostream &stream, const big_int &value)
{
    stream << value.to_string();
    return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value)
{
    std::string val;
    stream >> val;
    value = big_int(val);
    return stream;
}

bool big_int::operator==(const big_int &other) const noexcept
{
    if (_sign != other._sign)
    {
        return false;
    }

    if (_digits.size() != other._digits.size())
    {
        return false;
    }

    for (size_t i = 0; i < _digits.size(); ++i)
    {
        if (_digits[i] != other._digits[i])
        {
            return false;
        }
    }

    return true;
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign) : _sign(sign), _digits(digits)
{
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept : _sign(sign), _digits(std::move(digits))
{

}

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int> allocator)
{

    if (num.empty())
    {
        throw std::invalid_argument("error: std::string &num is empty");
    }

    if (radix < 2 || radix > 36)
    {
        throw std::invalid_argument("error: radix < 2 or radix > 36");
    }

    std::string num_copy = num;
    if (num_copy[0] == '-')
    {
        _sign = false;
        num_copy = num_copy.substr(1);
    }
    else
    {
        _sign = true;
    }

    while (num_copy.size() > 1 && num_copy[0] == '0')
    {
        num_copy = num_copy.substr(1);
    }

    if (num_copy.empty())
    {
        _digits.push_back(0);
        _sign = true;
        return;
    }
    int i = 0;

    _digits.push_back(0);
    for (char digit_char : num_copy)
    {
        unsigned int digit;
        if (std::isdigit(digit_char))
        {
            digit = digit_char - '0';
        }
        else if (std::isalpha(digit_char))
        {
            digit = std::toupper(digit_char) - 'A' + 10;
        }
        else
        {
            throw std::invalid_argument("invalid character in number string");
        }

        if (digit >= radix)
        {
            throw std::invalid_argument("digit exceeds radix");
        }
        auto temp = big_int(digit, allocator);

        *this *= radix;
        *this += big_int(digit, allocator);
    }
    std::cout << std::endl;
    if (_digits.size() == 1 && _digits[0] == 0)
    {
        _sign = true;
    }
}


big_int::big_int(pp_allocator<unsigned int> allocator) : _sign(true), _digits(allocator)
{
    _digits.push_back(0);
}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &
{
    if (*this == big_int(0) || other == big_int(0))
    {
        *this = big_int(0);
        return *this;
    }

    std::vector<unsigned int, pp_allocator<unsigned int>> result(_digits.size() + other._digits.size(), 0);
    bool result_sign = (_sign == other._sign);

    switch (rule)
    {
        case multiplication_rule::trivial:

            for (size_t i = 0; i < _digits.size(); ++i)
            {
                unsigned long long carry = 0;
                for (size_t j = 0; j < other._digits.size() || carry > 0; ++j)
                {
                    unsigned long long current = result[i + j] +
                                                 static_cast<unsigned long long>(_digits[i]) *
                                                 (j < other._digits.size() ? other._digits[j] : 0) +
                                                 carry;

                    result[i + j] = static_cast<unsigned int>(current);
                    carry = current >> (sizeof(unsigned int) * 8);
                }
            }
            break;
        case multiplication_rule::Karatsuba: {
            if (_digits.size() < threshold || other._digits.size() < threshold) {
                return multiply_assign(other, multiplication_rule::trivial);
            }

            size_t n = std::max(_digits.size(), other._digits.size());
            size_t half = n / 2;

            big_int low1(std::vector<unsigned int>(_digits.begin(), _digits.begin() + std::min(half, _digits.size())), true);
            big_int high1(std::vector<unsigned int>(_digits.begin() + std::min(half, _digits.size()), _digits.end()), true);

            big_int low2(std::vector<unsigned int>(other._digits.begin(), other._digits.begin() + std::min(half, other._digits.size())), true);
            big_int high2(std::vector<unsigned int>(other._digits.begin() + std::min(half, other._digits.size()), other._digits.end()), true);

            big_int z0 = low1 * low2;
            big_int z1 = (low1 + high1) * (low2 + high2);
            big_int z2 = high1 * high2;

            *this = z2;
            this->plus_assign(z1 - z2 - z0, half);
            this->plus_assign(z0, 0);

            _sign = (_sign == other._sign);
            return *this;
        }
        case multiplication_rule::SchonhageStrassen:
            std::cout << "SchonhageStrassen" << std::endl;
            break;
        default:
            break;
    }

    _digits = std::move(result);
    _sign = result_sign;
    optimise();
    return *this;
}

big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &
{
    if (!other) {
        throw std::invalid_argument("Division by zero is not allowed.");
    }

    big_int result((pp_allocator<unsigned int>()));

    switch (rule) {
        case division_rule::trivial: {
            big_int dividend = *this;
            big_int divisor = other;
            result = big_int(0);

            dividend._sign = true;
            divisor._sign = true;

            while (dividend >= divisor) {
                big_int temp_divisor = divisor;
                big_int quotient = 1;

                while ((temp_divisor << 1) <= dividend) {
                    temp_divisor <<= 1;
                    quotient <<= 1;
                }

                dividend -= temp_divisor;
                result += quotient;
            }

            result._sign = (_sign == other._sign);
            break;
        }
        case division_rule::Newton:
        case division_rule::BurnikelZiegler:
            throw not_implemented("big_int &big_int::divide_assign(const big_int &other, division_rule rule) &", "your code should be here...");
    }

    *this = std::move(result);
    return *this;
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &
{
    throw not_implemented("big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &", "your code should be here...");
}

big_int operator""_bi(unsigned long long n)
{
    return n;
}