#ifndef __LOADLEVEL_HH__
#define __LOADLEVEL_HH__

#include <state/GameState.hh>

namespace textlevel
{

void loadLevel(GameState * state, const std::string & levelName);
void saveLevel(GameState * state, const std::string & levelName);

bool levelExists(const std::string & levelName);

}

#endif