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
    point_t pos;
};

class GameObject
{
public:
    GameObject(int id);

    bool isColliding(GameObject& other);
    void bounceOutOf(GameObject& other);
    double radius();

    ColliderType colliderType;

    int id;
    ObjectState state;
    point_t size;

    sf::Sprite sprite;

    double moveSpeed;

};

#endif
