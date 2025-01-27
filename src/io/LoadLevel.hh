#ifndef __LOADLEVEL_HH__
#define __LOADLEVEL_HH__

#include <state/GameState.hh>

void loadLevel(GameState * state, const std::string & levelName);

bool levelExists(const std::string & levelName);

#endif