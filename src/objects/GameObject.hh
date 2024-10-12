#ifndef __GAME_OBJECT_HH__
#define __GAME_OBJECT_HH__

#include "MathUtil.hh"
#include "TextureBank.hh"
#include <SFML/Graphics/Sprite.hpp>
#include <initializer_list>
#include <vector>

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
        , angle_deg(0)
        , animIdx(0)
        , cooldown(0)
        , boxOccupied(false)
        , attachedObjectId(-1)
        , visible(true)
        , patrolIdx(0)
        , aiState(0)
        , targetId(-1)
        , chargeTime(0)
        , willInteract(false)
    {
    }

    ObjectState(const ObjectState& other)
        : pos(other.pos)
        , angle_deg(other.angle_deg)
        , animIdx(other.animIdx)
        , cooldown(other.cooldown)
        , boxOccupied(other.boxOccupied)
        , attachedObjectId(other.attachedObjectId)
        , visible(other.visible)
        , patrolIdx(other.patrolIdx)
        , aiState(other.aiState)
        , targetId(other.targetId)
        , chargeTime(other.chargeTime)
        , willInteract(other.willInteract)
    {
    }

    point_t pos;
    //Angle the entity is facing towards
    //0 is right, 90 is up, 180 is left, 270 is down
    float angle_deg;

    int animIdx;

    int cooldown; //Used by player

    //Time box related stuff - these are used by both the box and the occupant
    bool boxOccupied;
    int attachedObjectId;
    bool visible;

    //Enemy AI params
    int patrolIdx;
    int aiState;
    int targetId;
    point_t lastSeen;
    int chargeTime;

    //Player actions
    bool willInteract;
};

class GameObject
{
public:
    enum ObjectType
    {
        UNDEFINED,
        PLAYER,
        BULLET,
        ENEMY,
        TIMEBOX,
        SWITCH,
        DOOR,
        CLOSET
    };

    static std::string typeToString(ObjectType type)
    {
        switch(type)
        {
            case PLAYER:
                return "player";
            case BULLET:
                return "bullet";
            case ENEMY:
                return "enemy";
            case TIMEBOX:
                return "timebox";
            case SWITCH:
                return "switch";
            case DOOR:
                return "door";
            case CLOSET:
                return "closet";
            default:
                return "undefined";
        }
    }

    GameObject(int id);
    GameObject(int id, GameObject* ancestor);

    //A virtual function is necessary for polymorphism
    virtual ~GameObject() {}

    bool isColliding(GameObject& other);
    bool isColliding(point_t point);
    float radius() const;

    bool activeAt(int tick)
    {
        return tick >= beginning && (!hasEnding || tick <= ending);
    }

    sf::Sprite& getSprite()
    {
        return sprites[state.animIdx];
    }

    void setupSprites(std::initializer_list<const char*> filenames)
    {
        for(auto filename : filenames)
        {
            sf::Sprite sprite;
            sprite.setTexture(TextureBank::get(filename));
            sprites.push_back(sprite);
        }
    }

    void applyNextState()
    {
        state = nextState;
    }

    virtual bool isObstruction()
    {
        return false;
    }

    virtual ObjectType type(){
        return UNDEFINED;
    }

    ColliderType colliderType;

    int id;
    ObjectState state;
    point_t size;
    ObjectState nextState;

    std::vector<sf::Sprite> sprites;

    bool backwards;

    int beginning;
    bool hasEnding;
    int ending;

    bool recorded;    
};

#endif
