#ifndef __PLAYER_HH__
#define __PLAYER_HH__

#include "GameObject.hh"

class Player : public GameObject
{
public:
    Player(int id);
    Player(int id, Player* ancestor);

    float moveSpeed;
    int fireCooldown;

};

#endif