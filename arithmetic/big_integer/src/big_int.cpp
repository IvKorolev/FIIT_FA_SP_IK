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

    return multiply_assign(other, multiplication_rule::trivial);
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
    //big_int result(0);
    bool result_sign = (_sign == other._sign);

    switch (rule)
    {
        case multiplication_rule::trivial: {
            big_int result = multiply_table(*this, other);
            *this = result;
            optimise();
            return *this;
        }
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

big_int big_int::multiply_table(const big_int &left, const big_int &right) noexcept {
    constexpr unsigned int HALF_BITS = (sizeof(unsigned int) * 8) / 2;  // 16
    constexpr unsigned int HALF_MASK = (1u << HALF_BITS) - 1;          // 0xFFFF

    size_t n = left._digits.size();
    size_t m = right._digits.size();
    big_int result;
    // Результат может занять n+m+1 разряд
    result._digits.assign(n + m + 1, 0u);

    for (size_t i = 0; i < n; ++i) {
        unsigned int a = left._digits[i];
        unsigned int a_low  = a & HALF_MASK;
        unsigned int a_high = a >> HALF_BITS;

        for (size_t j = 0; j < m; ++j) {
            unsigned int b = right._digits[j];
            unsigned int b_low  = b & HALF_MASK;
            unsigned int b_high = b >> HALF_BITS;

            // четыре частичных произведения
            unsigned int p_ll = a_low  * b_low;
            unsigned int p_lh = a_low  * b_high;
            unsigned int p_hl = a_high * b_low;
            unsigned int p_hh = a_high * b_high;

            // 1) аккуратная сумма средних произведений с переносом
            unsigned int mid_lo = (p_lh & HALF_MASK) + (p_hl & HALF_MASK);
            unsigned int carry_mid_lo = mid_lo >> HALF_BITS;
            mid_lo &= HALF_MASK;
            unsigned int mid_hi = (p_lh >> HALF_BITS) + (p_hl >> HALF_BITS) + carry_mid_lo;

            // 2) собираем low часть: lo_lo | ((lo_hi + mid_lo) << HALF_BITS)
            unsigned int lo_lo = p_ll & HALF_MASK;
            unsigned int lo_hi = p_ll >> HALF_BITS;
            unsigned int res_lo_hi = lo_hi + mid_lo;
            unsigned int carry_lo = res_lo_hi >> HALF_BITS;
            res_lo_hi &= HALF_MASK;
            unsigned int low = (res_lo_hi << HALF_BITS) | lo_lo;

            // 3) общий перенос в high: carry_lo + mid_hi + (p_hh low part)
            unsigned int carry_low = carry_lo + mid_hi;
            unsigned int high = p_hh + carry_low;

            // Двухшаговая схема добавления с переносом, используя только unsigned int

            // используя только unsigned int
            // 1) Добавляем low
            unsigned int sum1   = result._digits[i + j] + low;
            unsigned int carry1 = (sum1 < low) ? 1u : 0u;
            result._digits[i + j] = sum1;

            // 2) Добавляем high + carry1
            unsigned int to_add = high + carry1;
            unsigned int sum2   = result._digits[i + j + 1] + to_add;
            unsigned int carry2 = (sum2 < to_add) ? 1u : 0u;
            result._digits[i + j + 1] = sum2;

            // Финальный перенос
            result._digits[i + j + 2] += carry2;
        }
    }

    // Убираем ведущие нули
    while (result._digits.size() > 1 && result._digits.back() == 0u) {
        result._digits.pop_back();
    }

    result._sign = (left._sign == right._sign);
    result.optimise();
    return result;
}


big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &{
    if (*this == big_int(0)) return *this;
    if (other == big_int(0)) throw std::logic_error("Division by zero");

    big_int abs_this(*this);
    abs_this._sign = true;
    big_int abs_other(other);
    abs_other._sign = true;

    if (abs_this == abs_other) {
        _digits.clear();
        _digits.push_back(1);
        _sign = (_sign == other._sign);
        return *this;
    }

    if (abs_this < abs_other) {
        _digits.clear();
        _digits.push_back(0);
        _sign = true;
        return *this;
    }

    std::vector<unsigned int, pp_allocator<unsigned int>> quotient(_digits.size(), 0, _digits.get_allocator());
    big_int remain(_digits.get_allocator());
    remain._digits.clear();
    remain._digits.push_back(0);

    for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; i--) {
        remain._digits.insert(remain._digits.begin(), _digits[i]);
        while (remain._digits.size() > 1 && remain._digits.back() == 0) {
            remain._digits.pop_back();
        }

        if (remain._digits.empty()) {
            remain._sign = true;
        }

        unsigned long long left = 0, q = 0, right = BASE;
        while (left <= right) {
            unsigned long long mid = left + (right - left) / 2;
            big_int temp = abs_other * big_int(static_cast<long long>(mid), _digits.get_allocator());
            if (remain >= temp) {
                q = mid;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }

        if (q > 0) {
            big_int temp = abs_other * big_int(static_cast<long long>(q), _digits.get_allocator());
            remain -= temp;
        }
        quotient[i] = static_cast<unsigned int>(q);
    }

    _sign = (_sign == other._sign);
    _digits = std::move(quotient);
    optimise();
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