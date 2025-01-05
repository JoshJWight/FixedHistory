#include "tickBullet.hh"

namespace tick{

void tickBullet(GameState * state, Bullet* bullet)
{
    if(!bullet->activeAt(state->tick))
    {
        return;
    }

    if(bullet->backwards != state->backwards())
    {
        bullet->nextState = state->historyBuffer()[bullet->id][state->tick];
        return;
    }

    bullet->nextState.pos += bullet->velocity;

    if(state->level->tileAt(bullet->state.pos) == Level::WALL)
    {

        bullet->finalTimeline = state->currentTimeline();
        bullet->hasFinalTimeline = true;
        if(bullet->backwards)
        {
            bullet->beginning = state->tick;
        }
        else
        {
            bullet->ending = state->tick;
            bullet->hasEnding = true;
        }
    }
}

}