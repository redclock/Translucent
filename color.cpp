#include "color.h"

// C++ headers
#include <cmath>
#include <cassert>

Color& Color::operator=(const Color& c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;

    return *this;
}

Color Color::operator+(const Color& c) const
{
    Color temp(r + c.r, g + c.g, b + c.b, 1.0);
    return temp;
}

Color& Color::operator+=(const Color& c)
{
    r += c.r;
    g += c.g;
    b += c.b;
    return *this;
}

Color Color::operator-(const Color& c) const
{
    Color temp(r - c.r, g - c.g, b - c.b, 1.0);
    return temp;
}

Color& Color::operator-=(const Color& c)
{
    r -= c.r;
    g -= c.g;
    b -= c.b;

    return *this;
}

Color Color::operator*(float d) const
{
    Color temp(r * d, g * d, b * d, 1.0);
    return temp;
}

Color& Color::operator*=(float d)
{
    r *= d;
    g *= d;
    b *= d;

    return *this;
}

Color Color::operator/(float d) const
{
    Color temp(r / d, g / d, b / d, 1.0);
    return temp;
}

Color& Color::operator/=(float d)
{
    r /= d;
    g /= d;
    b /= d;

    return *this;
}

Color Color::operator*(const Color& c) const
{
    Color temp(r * c.r, g * c.g, b * c.b, 1.0);
    return temp;
}

Color& Color::operator*=(const Color& c)
{
    r *= c.r;
    g *= c.g;
    b *= c.b;

    return *this;
}

Color Color::operator/(const Color& c) const
{
    Color temp(r / c.r, g / c.g, b / c.b, 1.0);
    return temp;
}

Color& Color::operator/=(const Color& c)
{
    r /= c.r;
    g /= c.g;
    b /= c.b;
    return *this;
}

bool Color::operator==(const Color& color) const
{
    if ((fabs(r - color.r) < 1e-6f)
            && (fabs(g - color.g) < 1e-6f)
            && (fabs(b - color.b) < 1e-6f)
            && (fabs(a - color.a) < 1e-6f))
        return true;
    else
        return false;
}

bool Color::operator!=(const Color& color) const
{
    return !(*this == color);
}

Color Color::exp(float a) const
{
    return Color(::exp(a * r), ::exp(a * g), ::exp(a * b));
}

Color Color::sqrt() const
{
    return Color(::sqrt(r), ::sqrt(g), ::sqrt(b));
}

float Color::meanValue() const
{
    return (r + g + b) / 3.0f;
}

Color operator+(float value, const Color& color)
{
    Color temp(value + color.r,
               value + color.g,
               value + color.b,
               1.0);
    return temp;
}

Color operator-(float value, const Color& color)
{
    Color temp(value - color.r,
               value - color.g,
               value - color.b,
               1.0);
    return temp;
}

Color operator*(float value, const Color& color)
{
    Color temp(value * color.r,
               value * color.g,
               value * color.b,
               1.0);
    return temp;
}

Color operator/(float value, const Color& color)
{
    Color temp(value / color.r,
               value / color.g,
               value / color.b,
               1.0);
    return temp;
}
