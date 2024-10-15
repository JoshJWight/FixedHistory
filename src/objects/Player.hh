#ifndef __PLAYER_HH__
#define __PLAYER_HH__

#include "GameObject.hh"

class Player : public GameObject
{
public:
    constexpr static float INTERACT_RADIUS = 10;

    Player(int id);
    Player(int id, Player* ancestor);

    ObjectType type() override
    {
        return PLAYER;
    }

    float moveSpeed;
    int fireCooldown;

    struct Observation
    {
        ObjectType type;
        ObjectState state;
        //Technically the observation doesn't need to be fulfilled by this object
        //But using this for drawing observed objects in graphics
        int id;
    };
    typedef std::vector<Observation> ObservationFrame;

    std::vector<ObservationFrame> observations;
};

#endif