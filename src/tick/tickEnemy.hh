#ifndef __TICKENEMY_HH__
#define __TICKENEMY_HH__


#include <state/GameState.hh>
#include <objects/Enemy.hh>
#include <objects/Player.hh>
#include <procedures/Search.hh>

namespace tick{

void createCrime(GameState * state, Enemy* enemy, Crime::CrimeType crimeType, GameObject* subject);

void reportCrimes(GameState * state, Enemy* enemy);

void reportSearches(GameState * state, Enemy* enemy);

float crimePriority(GameState * state, Crime* crime, Enemy* enemy);

float searchPriority(GameState * state, point_t pos, Enemy* enemy);

bool pointVisibleToEnemy(GameState * state, point_t point, Enemy* enemy);

bool playerVisibleToEnemy(GameState * state, Player* player, Enemy* enemy);

void navigateEnemy(GameState * state, Enemy* enemy, point_t target);

void tickEnemy(GameState * state, Enemy* enemy);

}

#endif