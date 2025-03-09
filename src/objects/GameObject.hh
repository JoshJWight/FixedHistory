#ifndef __GAME_OBJECT_HH__
#define __GAME_OBJECT_HH__

#include <utils/MathUtil.hh>
#include <io/TextureBank.hh>
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
        , willThrow(false)
        , holdingObject(false)
        , heldObjectId(-1)
        , speed(0)
        , searchStatus(0)
        , discovered(false)
        , alarmRadius(0)
        , assignedAlarm(-1)
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
        , lastSeen(other.lastSeen)
        , chargeTime(other.chargeTime)
        , willInteract(other.willInteract)
        , willThrow(other.willThrow)
        , holdingObject(other.holdingObject)
        , heldObjectId(other.heldObjectId)
        , speed(other.speed)
        , searchStatus(other.searchStatus)
        , discovered(other.discovered)
        , alarmRadius(other.alarmRadius)
        , assignedAlarm(other.assignedAlarm)
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
    bool willThrow;

    //Separate ID for held object since the player can hold an item and go in a box at the same time.
    bool holdingObject;
    int heldObjectId;

    //Used for throwables
    float speed;


    //Crime/alarm related params
    uint64_t searchStatus;
    bool discovered; //For dead enemies
    float alarmRadius;
    int assignedAlarm;
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
        CLOSET,
        TURNSTILE,
        SPIKES,
        OBJECTIVE,
        KNIFE,
        GUN,
        EXIT,
        CRIME,
        ALARM
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
            case TURNSTILE:
                return "turnstile";
            case SPIKES:
                return "spikes";
            case OBJECTIVE:
                return "objective";
            case KNIFE:
                return "knife";
            case GUN:
                return "gun";
            case EXIT:
                return "exit";
            case CRIME:
                return "crime";
            case ALARM:
                return "alarm";
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

    virtual bool isTransient()
    {
        return false;
    }

    virtual bool isDebugGraphic()
    {
        return false;
    }

    virtual ObjectType type(){
        return UNDEFINED;
    }

    int drawPriority();

    ColliderType colliderType;

    int id;
    ObjectState state;
    point_t size;
    ObjectState nextState;

    std::vector<sf::Sprite> sprites;

    bool backwards;

    //First tick where this object appears
    int beginning;
    //Has this object reached an ending in the current continuity?
    bool hasEnding;
    //Last tick where this object appears
    int ending;

    //Timeline where this object first appeared
    int initialTimeline;
    //Has this object reached an ending/beginning in the current continuity?
    bool hasFinalTimeline;
    //Timeline where this object's end state was established
    int finalTimeline;

    bool recorded;
};

#endif
