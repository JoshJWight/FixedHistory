#ifndef __TICKPLAYER_HH__
#define __TICKPLAYER_HH__

#include <GameState.hh>
#include <objects/Player.hh>
#include <Controls.hh>
#include <Search.hh>

namespace tick{

void tickPlayer(GameState* state, Player* player, Controls * controls);

}

#endif