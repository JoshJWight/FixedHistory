#ifndef __BULLET_HH__
#define __BULLET_HH__

#include "GameObject.hh"
#include "MathUtil.hh"

class Bullet : public GameObject
{
public:
    constexpr static int LIFETIME = 300;
    constexpr static double SPEED = 5;

    Bullet(int id);

    point_t velocity;

    int originTimeline;
};

#endif