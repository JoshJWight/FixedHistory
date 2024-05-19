#ifndef __MATH_UTIL_HH__
#define __MATH_UTIL_HH__

#include <cmath>
#include <SFML/System/Vector2.hpp>

typedef sf::Vector2<float> point_t;

namespace math_util{
    static float dist(point_t p1, point_t p2)
    {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        return sqrt(dx * dx + dy * dy);
    }

    template<typename T>
    static sf::Vector2<T> elementwise_multiply(sf::Vector2<T> a, sf::Vector2<T> b)
    {
        return sf::Vector2<T>(a.x * b.x, a.y * b.y);
    }

    template<typename T>
    static sf::Vector2<T> elementwise_divide(sf::Vector2<T> a, sf::Vector2<T> b)
    {
        return sf::Vector2<T>(a.x / b.x, a.y / b.y);
    }

    template<typename T>
    static sf::Vector2<T> normalize(sf::Vector2<T> a)
    {
        T mag = sqrt(a.x * a.x + a.y * a.y);
        return sf::Vector2<T>(a.x / mag, a.y / mag);
    }

    template<typename T>
    static T length(sf::Vector2<T> a)
    {
        return sqrt(a.x * a.x + a.y * a.y);
    }
}//end namespace math

#endif
