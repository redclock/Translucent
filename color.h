#pragma once

/**
 * Defines the color space.
 */
class Color
{
public:
    float r, g, b, a;

    Color(void) {};
    Color(float red, float green, float blue, float alpha)
        : r(red), g(green), b(blue), a(alpha) { };
    Color(float red, float green, float blue)
        : r(red), g(green), b(blue), a(1) { };
    Color(const Color& c)
        : r(c.r), g(c.g), b(c.b), a(c.a) {} ;

    ~Color(void) {};

// public member functions
public:
    /**
     * Operator=.
     */
    Color& operator=(const Color& c);
    /**
     * Operator==.
     */
    bool operator==(const Color& c) const;
    /**
     * Operator=.
     */
    bool operator!=(const Color& c) const;
    /**
     * Operator+.
     */
    Color operator+(const Color& c) const;
    /**
     * Operator+=.
     */
    Color& operator+=(const Color& c);
    /**
     * Operator-.
     */
    Color operator-(const Color& c) const;
    /**
     * Operator-=.
     */
    Color& operator-=(const Color& c);
    /**
     * Operator*.
     */
    Color operator*(float d) const;
    /**
     * Operator*=.
     */
    Color& operator*=(float d);
    /**
     * Operator/.
     */
    Color operator/(float d) const;
    /**
     * Operator/=.
     */
    Color& operator/=(float d);
    /**
     * Operator*(with another Color, means color filter).
     */
    Color operator*(const Color& c) const;
    /**
     * Operator*=(with another Color, means color filter).
     */
    Color& operator*=(const Color& c);
    /**
     * Operator/(with another Color, means color filter).
     */
    Color operator/(const Color& c) const;
    /**
     * Operator/=(with another Color, means color filter).
     */
    Color& operator/=(const Color& c);
    /** 
     * Computes exponent for every value.
     * @param coefficient in exponent.
     */
    Color exp(float a) const;
    /** 
     * Computes square root for every value.
     */
    Color sqrt() const;
    /** 
     * Gets the mean value of the RGB.
     */
    float meanValue() const;

// private member functions
private:
    friend Color operator+(float d, const Color& c);
    friend Color operator-(float d, const Color& c);
    friend Color operator*(float d, const Color& c);
    friend Color operator/(float d, const Color& c);
};

/**
 * Operator + for float and @c Color addition.
 */
Color operator+(float d, const Color& c);

/**
 * Operator - for float and @c Color subtraction.
 */
Color operator-(float d, const Color& c);

/**
 * Operator * for float and @c Color multiplying.
 */
Color operator*(float d, const Color& c);

/**
 * Operator / for float and @c Color division.
 */
Color operator/(float d, const Color& c);
