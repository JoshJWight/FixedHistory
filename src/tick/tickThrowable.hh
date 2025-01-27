#ifndef __TICKTHROWABLE_HH__
#define __TICKTHROWABLE_HH__

#include <state/GameState.hh>
#include <objects/Throwable.hh>
#include <objects/Player.hh>
#include <procedures/Search.hh>

namespace tick{

void tickThrowable(GameState * state, Throwable* throwable);

}

#endif