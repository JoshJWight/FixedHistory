#ifndef __GAME_OBJECT_HH__
#define __GAME_OBJECT_HH__

#include "MathUtil.hh"
#include <SFML/Graphics/Sprite.hpp>

enum ColliderType{
    NONE,
    CIRCLE,
    //Axis-aligned box
    BOX
};

struct ObjectState
{
    ObjectState()
        : pos(0, 0)
        , cooldown(0)
        , patrolIdx(0)
        , angle_deg(0)
    {
    }

    ObjectState(const ObjectState& other)
        : pos(other.pos)
        , cooldown(other.cooldown)
        , patrolIdx(other.patrolIdx)
        , angle_deg(other.angle_deg)
    {
    }

    point_t pos;
    //Angle the entity is facing towards
    //0 is right, 90 is up, 180 is left, 270 is down
    float angle_deg;
    int cooldown; //Used by player

    //Enemy AI params
    int patrolIdx;
};

class GameObject
{
public:
    GameObject(int id);
    GameObject(int id, GameObject* ancestor);

    bool isColliding(GameObject& other);
    void bounceOutOf(GameObject& other);
    float radius();

    bool activeAt(int tick)
    {
        return tick >= beginning && (!hasEnding || tick <= ending);
    }

    ColliderType colliderType;

    int id;
    ObjectState state;
    point_t size;

    sf::Sprite sprite;

    bool backwards;

    int beginning;
    bool hasEnding;
    int ending;

    bool recorded;
};

#endif
