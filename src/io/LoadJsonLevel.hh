#ifndef LOADJSONLEVEL_HH
#define LOADJSONLEVEL_HH

#include <state/GameState.hh>
#include <string>

namespace jsonlevel
{

void loadLevel(GameState * state, const std::string & levelName);
void saveLevel(GameState * state, const std::string & levelName);

bool levelExists(const std::string & levelName);

}


#endif