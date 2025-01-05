#include "tickDoor.hh"

namespace tick{

void tickDoor(GameState * state, Door* door)
{
    if(!door->activeAt(state->tick))
    {
        return;
    }

    if(door->backwards != state->backwards())
    {
        door->nextState = state->historyBuffer()[door->id][state->tick];
        return;
    }

    int onSwitches = 0;
    for(int swId : door->getConnectedSwitches())
    {
        Switch* sw = dynamic_cast<Switch*>(state->objects().at(swId).get());
        if(sw->state.aiState == Switch::ON)
        {
            onSwitches++;
        }
    }

    door->nextState.aiState = (onSwitches % 2 == 1) ? Door::OPEN : Door::CLOSED;
    door->nextState.animIdx = door->nextState.aiState;
}

}