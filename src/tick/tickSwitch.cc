#include "tickSwitch.hh"

namespace tick{

void tickSwitch(GameState * state, Switch* sw)
{
    if(sw->backwards != state->backwards())
    {
        sw->nextState = state->historyBuffer()[sw->id][state->tick];
        return;
    }

    for(Player* player : state->players())
    {
        if(player->state.willInteract
            && math_util::dist(player->state.pos, sw->state.pos) < (sw->size.x + Player::INTERACT_RADIUS)
            && player->backwards == state->backwards())
        {
            if(sw->state.aiState == Switch::OFF)
            {
                sw->nextState.aiState = Switch::ON;
            }
            else
            {
                sw->nextState.aiState = Switch::OFF;
            }
            break;
        }
    }

    sw->nextState.animIdx = sw->nextState.aiState;
}

}