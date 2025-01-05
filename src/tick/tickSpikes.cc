#include "tickSpikes.hh"

namespace tick{

void tickSpikes(GameState * state, Spikes* spikes)
{
    if(!spikes->activeAt(state->tick))
    {
        return;
    }

    if(spikes->backwards != state->backwards())
    {
        spikes->nextState = state->historyBuffer()[spikes->id][state->tick];
        return;
    }

    int pointInCycle = (state->tick + spikes->cycleOffset) % (spikes->downDuration + spikes->upDuration);
    if(pointInCycle < spikes->downDuration)
    {
        if(spikes->downDuration - pointInCycle < Spikes::WARNING_DURATION)
        {
            spikes->nextState.aiState = Spikes::WARNING;
        }
        else
        {
            spikes->nextState.aiState = Spikes::DOWN;
        }
    }
    else
    {
        if(spikes->downDuration > 0 && (spikes->upDuration - (pointInCycle - spikes->downDuration) < Spikes::WARNING_DURATION))
        {
            spikes->nextState.aiState = Spikes::WARNING;
        }
        else
        {
            spikes->nextState.aiState = Spikes::UP;
        }
    }
    spikes->nextState.animIdx = spikes->nextState.aiState;
}


}