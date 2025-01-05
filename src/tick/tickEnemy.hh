#ifndef __TICKENEMY_HH__
#define __TICKENEMY_HH__


#include <GameState.hh>
#include <objects/Enemy.hh>
#include <objects/Player.hh>
#include <Search.hh>

namespace tick{

bool playerVisibleToEnemy(GameState * state, Player* player, Enemy* enemy);

void navigateEnemy(GameState * state, Enemy* enemy, point_t target);

void tickEnemy(GameState * state, Enemy* enemy);

}

#endif