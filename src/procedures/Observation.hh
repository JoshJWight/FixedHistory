#ifndef __OBSERVATION_HH__
#define __OBSERVATION_HH__

#include <state/GameState.hh>
#include <procedures/Search.hh>
#include <objects/Player.hh>

#include <vector>
#include <stddef.h>

namespace observation{

void recordObservations(GameState * state, Player * player, int tick);

bool observablyEqual(const ObjectState & a, const ObjectState & b);

std::string checkObservations(GameState * state, Player * player, int tick);

}//namespace observation

#endif