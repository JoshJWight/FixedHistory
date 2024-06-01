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
        if(mag == 0)
        {
            //Pick an arbitrary direction
            return sf::Vector2<T>(0, 1);
        }
        return sf::Vector2<T>(a.x / mag, a.y / mag);
    }

    template<typename T>
    static T length(sf::Vector2<T> a)
    {
        return sqrt(a.x * a.x + a.y * a.y);
    }

    static float angleBetween(point_t a, point_t b)
    {
        return atan2(b.y - a.y, b.x - a.x) * 180.0f / M_PI;
    }

    static float angleDiff(float a, float b)
    {
        float diff = b - a;
        if (diff > 180) {
            diff -= 360;
        }
        if (diff < -180) {
            diff += 360;
        }
        return diff;
    }

    static float rotateAngleTowards(float currentAngle, float targetAngle, float maxRotate)
    {
        float diff = targetAngle - currentAngle;
        if (diff > 180) {
            diff -= 360;
        }
        if (diff < -180) {
            diff += 360;
        }
        if (diff > maxRotate) {
            diff = maxRotate;
        }
        if (diff < -maxRotate) {
            diff = -maxRotate;
        }

        float result = currentAngle + diff;
        if (result > 180) {
            result -= 360;
        }
        if (result < -180) {
            result += 360;
        }
        return result;
    }

    static float rotateTowardsPoint(float currentAngle, point_t currentPos, point_t targetPos, float maxRotate)
    {
        float targetAngle = angleBetween(currentPos, targetPos);
        return rotateAngleTowards(currentAngle, targetAngle, maxRotate);
    }
}//end namespace math

#endif
