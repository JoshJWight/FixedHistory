#ifndef __TICKPLAYER_HH__
#define __TICKPLAYER_HH__

#include <state/GameState.hh>
#include <objects/Player.hh>
#include <io/Controls.hh>
#include <procedures/Search.hh>

namespace tick{

void tickPlayer(GameState* state, Player* player, Controls * controls);

}

#endif