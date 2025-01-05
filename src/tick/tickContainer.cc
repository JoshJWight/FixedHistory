#include "tickContainer.hh"

namespace tick{

void tickContainer(GameState * state, Container* container)
{
    if(container->activeOccupant > -1)
    {
        container->nextState.boxOccupied = true;
        container->nextState.attachedObjectId = container->activeOccupant;

        //If about to run into a point where someone else was in the box, kick the current occupant out
        for(int i=1; i<Container::OCCUPANCY_SPACING + 300; i++)
        {
            int timestepToCheck;
            if(state->backwards())
            {
                timestepToCheck = state->tick - i;
            }
            else
            {
                timestepToCheck = state->tick + i;
            }

            if(timestepToCheck < 0 || timestepToCheck >= state->historyBuffer()[container->id].size())
            {
                break;
            }

            ObjectState & objState = state->historyBuffer()[container->id][timestepToCheck];
            if(objState.boxOccupied && objState.attachedObjectId != container->activeOccupant)
            {
                if(i<Container::OCCUPANCY_SPACING)
                {
                    if(container->reverseOnExit)
                    {
                        state->shouldReverse = true;
                    }
                    else
                    {
                        GameObject* occupant = state->objects().at(container->activeOccupant).get();
                        if(occupant->state.holdingObject)
                        {
                            Throwable* throwable = dynamic_cast<Throwable*>(state->objects().at(occupant->state.heldObjectId).get());
                            throwable->nextState.visible = true;
                        }

                        occupant->nextState.boxOccupied = false;
                        occupant->nextState.attachedObjectId = -1;
                        occupant->nextState.visible = true;
                        container->activeOccupant = -1;

                        container->nextState.boxOccupied = false;
                        container->nextState.attachedObjectId = -1;
                    }
                }
                else
                {
                    int secondsLeft = (i - Container::OCCUPANCY_SPACING) / 60;
                    state->statusString = "AUTO-EJECT IN " + std::to_string(secondsLeft);
                }
                break;
            }
        }
    } 
    else if(state->tick >= state->historyBuffer()[container->id].size())
    {
        container->nextState.boxOccupied = false;
        container->nextState.attachedObjectId = -1;
    }
    else
    {
        container->nextState = state->historyBuffer()[container->id][state->tick];
    }
}

}