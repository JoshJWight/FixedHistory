#include "GameObject.hh"
#include <iostream>
#include "CollisionUtil.hh"

GameObject::GameObject(int id)
    : id(id)
    , colliderType(NONE)
    , backwards(false)
    , beginning(0)
    , hasEnding(false)
    , ending(0)
    , recorded(false)
{

}

GameObject::GameObject(int id, GameObject* ancestor)
    : id(id)
    , colliderType(ancestor->colliderType)
    , size(ancestor->size)
    , state(ancestor->state)
    , sprite(ancestor->sprite)
    , backwards(!ancestor->backwards)
    , beginning(0)
    , hasEnding(false)
    , ending(0)
    , recorded(false)
{

}

float GameObject::radius()
{
    return size.x / 2.0f;
}

bool GameObject::isColliding(GameObject& other)
{
    if(colliderType==CIRCLE)
    {
        if(other.colliderType==CIRCLE)
        {
            return math_util::dist(state.pos, other.state.pos) < (radius() + other.radius());
        }
        else if(other.colliderType==BOX)
        {
            return collision::boxCircle(other.state.pos, other.size, state.pos, radius());
        }
    }
    else if(colliderType==BOX)
    {
        if(other.colliderType==CIRCLE)
        {
            return collision::boxCircle(state.pos, size, other.state.pos, other.radius());
        }
        else if(other.colliderType==BOX)
        {
            //Top right, bottom left
            point_t tr1 = state.pos + (size / 2.0f);
            point_t bl1 = state.pos - (size / 2.0f);
            point_t tr2 = other.state.pos + (other.size / 2.0f);
            point_t bl2 = other.state.pos - (other.size / 2.0f);

            return (tr1.y > bl2.y && bl2.y < tr2.y && bl1.x < tr2.x && tr1.x > bl2.x);
        }
    }

    //Generally you get here if one object or other has no collider
    return false;
}

void GameObject::bounceOutOf(GameObject& other)
{
    //TODO
}
