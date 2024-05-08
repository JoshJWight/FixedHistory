#ifndef __COLLISION_UTIL_HH__
#define __COLLISION_UTIL_HH__

#include "MathUtil.hh"
#include <cmath>

namespace collision{

//There's probably a much simpler way to do all this ...
static bool pointInsideBox(point_t point, point_t boxCenter, double boxLenX, double boxLenY)
{
    return abs(point.x - boxCenter.x) < boxLenX / 2.0 && abs(point.y - boxCenter.y) < boxLenY / 2.0;
}

static bool pointInsideCircle(point_t point, point_t circleCenter, double circleRadius)
{
    return math_util::dist(point, circleCenter) < circleRadius;
}

static bool verticalLineCircle(point_t bottom, point_t top, point_t circleCenter, double circleRadius)
{
    return abs(circleCenter.x - bottom.x) < circleRadius && (
        (circleCenter.y > bottom.y && circleCenter.y < top.y) ||
        pointInsideCircle(bottom, circleCenter, circleRadius) ||
        pointInsideCircle(top, circleCenter, circleRadius));
}

static bool horizontalLineCircle(point_t left, point_t right, point_t circleCenter, double circleRadius)
{
    return abs(circleCenter.y - left.y) < circleRadius && (
        (circleCenter.x > left.x && circleCenter.x < right.x) ||
        pointInsideCircle(left, circleCenter, circleRadius) ||
        pointInsideCircle(right, circleCenter, circleRadius));
}

static bool boxCircle(point_t boxCenter, point_t boxSize, point_t circleCenter, double circleRadius)
{
    if(pointInsideBox(circleCenter, boxCenter, boxSize.x, boxSize.y))
    {
        return true;
    }
    else if(pointInsideCircle(boxCenter, circleCenter, circleRadius))
    {
        return true;
    }
    else
    {
        point_t halfBox = boxSize / 2.0;
        point_t topLeft = boxCenter + point_t(-1.0 * halfBox.x, halfBox.y);
        point_t topRight = boxCenter + halfBox;
        point_t bottomLeft = boxCenter - halfBox;
        point_t bottomRight = boxCenter + point_t(halfBox.x, -1.0 * halfBox.y);

        return horizontalLineCircle(topLeft, topRight, circleCenter, circleRadius) ||
               horizontalLineCircle(bottomLeft, bottomRight, circleCenter, circleRadius) ||
               verticalLineCircle(bottomLeft, topLeft, circleCenter, circleRadius) ||
               verticalLineCircle(bottomRight, topRight, circleCenter, circleRadius);
    }

}

}//End namespace collision

#endif
