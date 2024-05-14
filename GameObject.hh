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
        , active(true)
        , cooldown(0)
    {
    }

    ObjectState(const ObjectState& other)
        : pos(other.pos)
        , active(other.active)
        , cooldown(other.cooldown)
    {
    }

    point_t pos;
    bool active;
    int cooldown;
};

class GameObject
{
public:
    GameObject(int id);
    GameObject(int id, GameObject* ancestor);

    bool isColliding(GameObject& other);
    void bounceOutOf(GameObject& other);
    double radius();

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
