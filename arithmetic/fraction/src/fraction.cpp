#define _USE_MATH_DEFINES
#include <cmath>

#include "../include/fraction.h"

void fraction::optimise()
{
    bool is_negative = false;
    if (_numerator < 0 && _denominator < 0)
    {
        _numerator = _numerator * big_int(-1);
        _denominator = _denominator * big_int(-1);
    }
    else if (_numerator < 0 && _denominator > 0)
    {
        is_negative = true;
        _numerator = _numerator * big_int(-1);
    }
    else if (_numerator > 0 && _denominator < 0)
    {
        is_negative = true;
        _denominator = _denominator * big_int(-1);
    }

    big_int gcd = _denominator;
    big_int second = _numerator;

    if (gcd < second) std::swap(gcd, second);

    while (second > big_int(0)) {
        big_int temp = second;
        second = gcd % second;
        gcd = temp;
        //std::cout << "gcd: " << gcd << ", second: " << second << big_int(0) << std::endl;
    }

    _numerator /= gcd;
    _denominator /= gcd;

    if (is_negative) _numerator = _numerator * big_int(-1);
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f &&numerator, s &&denominator) : _numerator(numerator), _denominator(denominator)
{
    //std::cout << this->to_string() << std::endl;
    if (_denominator == 0_bi) {
        throw std::invalid_argument("Fraction is zero in denominator");
    }
    if (_numerator != 0_bi) {
        optimise();
    }
}

fraction::fraction(pp_allocator<big_int::value_type>)
{
    //Ya ne eby chto tut pisat
}

fraction &fraction::operator+=(fraction const &other) &
{
    big_int lcm = find_denominator(this, &other);

    big_int first_multiplier = lcm / _denominator;
    big_int second_multiplier = lcm / other._denominator;

    _denominator = lcm;

    _numerator = _numerator * first_multiplier;
    _numerator += (other._numerator * second_multiplier);

    optimise();
    return *this;
}

fraction fraction::operator+(fraction const &other) const
{
    fraction tmp = *this;
    tmp += other;
    return tmp;
}

fraction &fraction::operator-=(fraction const &other) &
{
    big_int lcm = find_denominator(this, &other);

    big_int first_multiplier = lcm / _denominator;
    big_int second_multiplier = lcm / other._denominator;

    _denominator = lcm;

    _numerator = _numerator * first_multiplier;
    _numerator -= (other._numerator * second_multiplier);

    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const
{
    fraction tmp = *this;
    tmp -= other;
    return tmp;
}

fraction &fraction::operator*=(fraction const &other) &
{
    _numerator *= other._numerator;
    _denominator *= other._denominator;

    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const
{
    fraction tmp = *this;
    tmp *= other;
    return tmp;
}

fraction &fraction::operator/=(fraction const &other) &
{
    _numerator *= other._denominator;
    _denominator *= other._numerator;

    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const
{
    fraction tmp = *this;
    tmp /= other;
    return tmp;
}

bool fraction::operator==(fraction const &other) const noexcept
{
    return (_numerator == other._numerator) && (_denominator == other._denominator);
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept
{
    big_int lhs = _numerator * other._denominator;
    big_int rhs = other._numerator * _denominator;
    return lhs <=> rhs;
}

std::ostream &operator<<(std::ostream &stream, fraction const &obj)
{
    stream << obj._numerator << "/" << obj._denominator;
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj)
{
    std::string a, b;
    char slash;
    stream >> a;

    stream >> slash;
    if (slash != '/') {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    stream >> b;
    obj = fraction(big_int(a), big_int(b));
    return stream;
}

std::string fraction::to_string() const
{
    if (_numerator == big_int(0)) {
        return "0/1";
    }
    return _numerator.to_string() + "/" + _denominator.to_string();
}

fraction fraction::sin(fraction const &epsilon) const
{
    fraction x = *this; // Значение угла
    fraction term = x;  // Первый член ряда
    fraction result = x; // Начальное значение результата
    size_t n = 1;       // Счетчик итераций

    while (term.abs() > epsilon) {
        term *= (x * x) / fraction(big_int((2 * n) * (2 * n + 1)), 1_bi); // Вычисляем следующий член ряда
        term *= fraction(big_int(-1), 1_bi); // Меняем знак
        result += term;       // Добавляем член к результату
        ++n;
    }

    return result;
}

fraction fraction::cos(fraction const &epsilon) const
{
    fraction x = *this; // Значение угла
    fraction term = fraction(big_int(1), 1_bi);  // Первый член ряда
    fraction result = term; // Начальное значение результата
    size_t n = 1;       // Счетчик итераций

    while (term.abs() > epsilon) {
        term *= (x * x) / fraction(big_int((2 * n - 1) * (2 * n)), 1_bi); // Вычисляем следующий член ряда
        term *= fraction(big_int(-1), 1_bi); // Меняем знак
        result += term;       // Добавляем член к результату
        ++n;
    }

    return result;
}

fraction fraction::tg(fraction const &epsilon) const
{
    fraction sinus = this->sin(epsilon);
    fraction cosinus = this->cos(epsilon);
    return sinus / cosinus;
}

fraction fraction::ctg(fraction const &epsilon) const
{
    fraction sinus = this->sin(epsilon);
    fraction cosinus = this->cos(epsilon);
    return cosinus / sinus;
}

fraction fraction::sec(fraction const &epsilon) const
{
    fraction cosinus = this->cos(epsilon);
    return fraction(big_int(1), 1_bi) / cosinus;
}

fraction fraction::cosec(fraction const &epsilon) const
{
    fraction sinus = this->sin(epsilon);
    return fraction(big_int(1), 1_bi) / sinus;
}

fraction fraction::arcsin(fraction const &epsilon) const
{
    fraction zero(0_bi, 1_bi);
    fraction one(1_bi, 1_bi);
    fraction two(2_bi, 1_bi);

    if (epsilon <= zero)
    {
        throw std::logic_error("Epsilon must greater than 0");
    }

    if (abs() > one)
    {
        throw std::logic_error("Module of number must be not greater than 1");
    }

    if (abs() > fraction(4, 5))
    {
        return fraction(big_int(15707963), big_int(10000000)) - (one - pow(2)).root(2,epsilon).arcsin(epsilon);
    }

    fraction const &x = *this;
    fraction current = x;
    fraction sum = current;
    fraction n = one;

    do
    {
        fraction tmp = two * n;
        current *= (tmp - one).pow(2) * x * x / (tmp * (tmp + one));
        sum += current;
        n += one;
    }
    while(current.abs() >= epsilon);

    return sum;
}

fraction fraction::arccos(fraction const &epsilon) const
{
    // return fraction(big_int(1), 1_bi).arcsin(epsilon) - this->arcsin(epsilon);
    return fraction(big_int(15707963), big_int(10000000)) - this->arcsin(epsilon);
}

fraction fraction::arctg(fraction const &epsilon) const
{
    return (*this / ((fraction(1_bi, 1_bi)) + pow(2)).root(2, epsilon)).arcsin(epsilon);
}

fraction fraction::arcctg(fraction const &epsilon) const
{
    return (*this / ((fraction(1_bi, 1_bi)) + pow(2)).root(2, epsilon)).arccos(epsilon);
}

fraction fraction::arcsec(fraction const &epsilon) const
{
    return (fraction(1_bi, 1_bi) / (*this)).arccos(epsilon);
}

fraction fraction::arccosec(fraction const &epsilon) const
{
    return (fraction(1_bi, 1_bi) / (*this)).arcsin(epsilon);
}

fraction fraction::pow(size_t degree) const
{
    if (degree == 0) {
        return fraction(1_bi, 1_bi);
    }
    if (degree == 1) {
        return *this;
    }
    if (degree % 2 == 0) {
        fraction temp = *this;
        temp = temp.pow(degree / 2);
        temp *= temp;
        return temp;
    }
    else {
        fraction temp = *this;
        temp = temp.pow(degree-1);
        temp *= (*this);
        return temp;
    }
}

fraction fraction::root(size_t degree, fraction const &epsilon) const
{
    if (degree == 0)
    {
        throw std::logic_error("Root degree cannot be zero");
    }
    if (degree == 1)
    {
        return *this;
    }

    fraction zero(0, 1);
    fraction one(1, 1);
    fraction two(2, 1);
    fraction fraction_degree(big_int(std::to_string(degree)), big_int("1"));
    fraction tmp_fraction = *this;


    int sign = 1;
    int l = 0;

    if (tmp_fraction < zero)
    {
        if (degree & 1)
        {
            sign = -1;
            tmp_fraction *= fraction(-1, 1);
        }
        else
        {
            throw std::logic_error("Cannot take odd degree root of negative number");
        }
    }

    fraction current = (one + tmp_fraction) / two;
    fraction next = current;

    fraction one_by_n(big_int("1"), big_int(std::to_string(degree)));

    fraction prev_degree = fraction_degree - one;

    do
    {
        current = next;
        next = one_by_n * (prev_degree * current + tmp_fraction / current.pow(degree - 1));
    }
    while ((next - current).abs() >= epsilon);

    if (sign == -1)
    {
        return next * fraction(-1, 1);
    }
    return next;
}

fraction fraction::log2(fraction const &epsilon) const
{
    return ln(epsilon) / (fraction(2_bi, 1_bi)).ln(epsilon);
}

fraction fraction::ln(fraction const &epsilon) const
{
    if (*this <= fraction(0, 1))
    {
        throw std::domain_error("Natural logarithm of non-positive number");
    }

    fraction x = *this;
    fraction result(0, 1);
    big_int power_shift(0);

    while (x > fraction(2, 1))
    {
        x /= fraction(2, 1);
        power_shift += big_int(1);
    }

    while (x < fraction(1, 2))
    {
        x *= fraction(2, 1);
        power_shift -= big_int(1);
    }

    fraction y = (x - fraction(1, 1)) / (x + fraction(1, 1));
    fraction y_squared = y * y;
    fraction term = y;
    fraction sum = term;
    big_int n(1);
    fraction prev_sum;
    int max_iterations = 1000;
    int iterations = 0;

    do
    {
        prev_sum = sum;
        term = (term * y_squared * fraction(big_int(2) * n - 1, 1)) / fraction(big_int(2) * n + 1, 1);
        sum += term;
        n += big_int(1);
        iterations++;

        if (iterations > 1 && (sum - prev_sum).abs() <= epsilon)
        {
            break;
        }
    } while (iterations < max_iterations && term.abs() > epsilon);

    fraction ln2 = fraction(big_int(6931471), big_int(10000000));

    return fraction(2, 1) * sum + fraction(power_shift, 1) * ln2;
}


fraction fraction::lg(fraction const &epsilon) const
{
    fraction temp = ln(epsilon) / (fraction(10_bi, 1_bi)).ln(epsilon);
    temp.optimise();
    return temp;
}

big_int fraction::find_denominator(const fraction *first, const fraction *second) {

    big_int gcd = first->_denominator;
    big_int second_denom = second->_denominator;

    if (gcd < second_denom) std::swap(gcd, second_denom);

    while (second_denom > big_int(0)) {
        big_int temp = second_denom;
        second_denom = gcd % second_denom;
        gcd = temp;
    }

    big_int lcm = (first->_denominator / gcd) * second->_denominator;
    return lcm;
}

fraction fraction::abs() const {
    fraction result = *this;

    if (result._numerator < 0) {
        result._numerator = result._numerator * big_int(-1);
    }

    return result;
}

fraction fraction::pi() const
{
    double pi = M_PI;
    pi *= 100000000;
    int p_i = floor(pi);
    return fraction(big_int(p_i), big_int(100000000));
}