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
    {
    }

    ObjectState(const ObjectState& other)
        : pos(other.pos)
        , cooldown(other.cooldown)
        , patrolIdx(other.patrolIdx)
    {
    }

    point_t pos;
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
    double radius();

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
