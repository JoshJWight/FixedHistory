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
    , initialTimeline(0)
    , hasFinalTimeline(false)
    , finalTimeline(0)
    , recorded(false)
{

}

GameObject::GameObject(int id, GameObject* ancestor)
    : id(id)
    , colliderType(ancestor->colliderType)
    , size(ancestor->size)
    , state(ancestor->state)
    , sprites(ancestor->sprites)
    , backwards(!ancestor->backwards)
    , beginning(0)
    , hasEnding(false)
    , ending(0)
    , initialTimeline(0)
    , hasFinalTimeline(false)
    , finalTimeline(0)
    , recorded(false)
{

}

float GameObject::radius() const
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

bool GameObject::isColliding(point_t point)
{
    if(colliderType==CIRCLE)
    {
        return math_util::dist(state.pos, point) < radius();
    }
    else if(colliderType==BOX)
    {
        point_t tr = state.pos + (size / 2.0f);
        point_t bl = state.pos - (size / 2.0f);

        return (point.y > bl.y && point.y < tr.y && point.x > bl.x && point.x < tr.x);
    }

    //Generally you get here if the object has no collider
    return false;
}


int GameObject::drawPriority()
{
    switch(type())
    {
        case PLAYER:
            return 10;
        case ENEMY:
            return 10;
        case BULLET:
            return 20;
        case SWITCH:
            return 5;
        case TIMEBOX:
            return 0;
        case CLOSET:
            return 0;
        case TURNSTILE:
            return 0;
        case DOOR:
            return 5;
        case SPIKES:
            return 5;
        case OBJECTIVE:
            return 10;
        case KNIFE:
            return 10;
        case EXIT:
            return 0;
        default:
            return 0;
    }
}