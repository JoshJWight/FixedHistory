#include "tickPlayer.hh"

namespace tick{

void tickPlayer(GameState* state, Player* player, Controls * controls)
{
    player->nextState.willInteract = controls->interact;
    player->nextState.willThrow = controls->throw_;

    if(player->state.boxOccupied)
    {
        if(player->state.willInteract)
        {
            std::cout << "Exiting box" << std::endl;
            Container * container = dynamic_cast<Container*>(state->objects().at(player->state.attachedObjectId).get());
            if(container->reverseOnExit)
            {
                state->shouldReverse = true;
            }
            else
            {
                container->activeOccupant = -1;
                container->nextState.boxOccupied = false;
                container->nextState.attachedObjectId = -1;
                player->nextState.boxOccupied = false;
                player->nextState.attachedObjectId = -1;
                player->nextState.visible = true;

                if(player->state.holdingObject)
                {
                    Throwable* throwable = dynamic_cast<Throwable*>(state->objects().at(player->state.heldObjectId).get());
                    throwable->nextState.visible = true;
                }
            }
        }

        //Player can't move or act while in a box
        return;
    }
    
    if(player->state.willInteract)
    {
        for(Container* container: state->containers())
        {
            if(math_util::dist(player->state.pos, container->state.pos) < (container->size.x + Player::INTERACT_RADIUS)
                && !container->state.boxOccupied)
            {
                if(container->reverseOnEnter)
                {
                    state->shouldReverse = true;
                    state->boxToEnter = container->id;
                    break;
                }
                else
                {
                    container->activeOccupant = player->id;
                    player->nextState.boxOccupied = true;
                    player->nextState.attachedObjectId = container->id;
                    player->nextState.visible = false;

                    if(player->state.holdingObject)
                    {
                        Throwable* throwable = dynamic_cast<Throwable*>(state->objects().at(player->state.heldObjectId).get());
                        throwable->nextState.visible = false;
                    }
                    break;
                }
            }
        }
    }


    if(player->state.cooldown > 0)
    {
        player->nextState.cooldown--;
    }

    if(controls->up)
    {
        player->nextState.pos.y += player->moveSpeed;
    }
    if(controls->down)
    {
        player->nextState.pos.y -= player->moveSpeed;
    }
    if(controls->left)
    {
        player->nextState.pos.x -= player->moveSpeed;
    }
    if(controls->right)
    {
        player->nextState.pos.x += player->moveSpeed;
    }

    //No walking through walls or obstructions
    if(search::checkObstruction(state, player->nextState.pos) && !search::checkObstruction(state, player->state.pos))
    {
        player->nextState.pos = player->state.pos;
    }

    if(controls->fire && player->state.cooldown == 0)
    {
        point_t direction = math_util::normalize(state->mousePos - player->state.pos);
        point_t bulletPos = player->state.pos + direction * player->size.x;
        std::shared_ptr<Bullet> bullet(new Bullet(state->nextID()));
        bullet->creatorId = player->id;
        bullet->state.pos = bulletPos;
        bullet->velocity = direction * Bullet::SPEED;
        bullet->state.angle_deg = math_util::angleBetween(player->state.pos, state->mousePos);
        bullet->initialTimeline = state->currentTimeline();
        bullet->backwards = player->backwards;
        bullet->nextState = bullet->state;
        if(player->backwards)
        {
            bullet->ending = state->tick;
            bullet->hasEnding = true;
        }
        else
        {
            bullet->beginning = state->tick;
        }

        player->nextState.cooldown = player->fireCooldown;

        state->bullets().push_back(bullet.get());
        state->objects()[bullet->id] = bullet;
        state->historyBuffer().buffer[bullet->id] = std::vector<ObjectState>(state->tick+1);
        state->historyBuffer().buffer[bullet->id][state->tick] = bullet->state;
    }

    player->nextState.angle_deg = math_util::rotateTowardsPoint(player->state.angle_deg, player->state.pos, state->mousePos, 5.0f);
}


}