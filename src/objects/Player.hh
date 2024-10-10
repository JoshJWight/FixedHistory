#ifndef __PLAYER_HH__
#define __PLAYER_HH__

#include "GameObject.hh"

class Player : public GameObject
{
public:
    constexpr static float INTERACT_RADIUS = 10;

    Player(int id);
    Player(int id, Player* ancestor);

    float moveSpeed;
    int fireCooldown;

    struct Observation
    {
        ObjectType type;
        ObjectState state;
    };
    typedef std::vector<Observation> ObservationFrame;

    std::vector<ObservationFrame> observations;
};

#endif