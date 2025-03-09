#include "tickThrowable.hh"

namespace tick{

void tickThrowable(GameState * state, Throwable* throwable)
{
    if(!throwable->activeAt(state->tick))
    {
        return;
    }

    if(throwable->backwards != state->backwards())
    {
        //TODO we may want to have backwards throwables be usable in the future
        throwable->nextState = state->historyBuffer()[throwable->id][state->tick];
        return;
    }

    if(throwable->state.aiState == Throwable::STILL)
    {
        for(Player* player : state->players())
        {
            if(player->state.willThrow
                && math_util::dist(player->state.pos, throwable->state.pos) < (throwable->size.x + Player::INTERACT_RADIUS)
                && player->backwards == state->backwards()
                && !player->state.holdingObject
                && !(player->nextState.holdingObject && player->nextState.heldObjectId != throwable->id))
            {
                throwable->nextState.aiState = Throwable::HELD;
                throwable->nextState.attachedObjectId = player->id;
                player->nextState.heldObjectId = throwable->id;
                player->nextState.holdingObject = true;
                break;
            }
        }
    }
    else if(throwable->state.aiState == Throwable::THROWN)
    {
        point_t nextPos = math_util::moveInDirection(throwable->state.pos, throwable->state.angle_deg, throwable->state.speed);
        if(search::checkObstruction(state, nextPos))
        {
            float bounceAngle = search::bounceOffWall(state, throwable->state.pos, nextPos);
            throwable->nextState.angle_deg = bounceAngle;
            throwable->nextState.speed *= throwable->bounciness;
            throwable->nextState.pos = math_util::moveInDirection(throwable->state.pos, bounceAngle, throwable->nextState.speed);
        }
        else
        {
            throwable->nextState.pos = nextPos;
        }
        throwable->nextState.speed -= throwable->drag;

        if(throwable->nextState.speed < 0.0f)
        {
            throwable->nextState.aiState = Throwable::STILL;
            throwable->nextState.speed = 0.0f;
        }

        for(Enemy* enemy : state->enemies())
        {
            if(enemy->activeAt(state->tick) && enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*throwable))
            {
                throwable->nextState.aiState = Throwable::STILL;
                throwable->nextState.speed = 0.0f;
                break;
            }
        }
    }
    else if(throwable->state.aiState == Throwable::HELD)
    {
        Player * holder = dynamic_cast<Player*>(state->objects().at(throwable->state.attachedObjectId).get());
        throwable->nextState.pos = math_util::moveInDirection(holder->state.pos, holder->state.angle_deg - 30, holder->size.x);

        if(throwable->type() == GameObject::GUN)
        {
            throwable->nextState.angle_deg = math_util::angleBetween(throwable->state.pos, state->mousePos);
        }
        else
        {
            throwable->nextState.angle_deg = holder->state.angle_deg;
        }

        if(holder->state.willFire && !holder->state.boxOccupied)
        {
            throwable->nextState.aiState = Throwable::USED;
            throwable->nextState.chargeTime = 0;
        }
        else if(holder->state.willThrow && !holder->state.boxOccupied)
        {
            throwable->nextState.aiState = Throwable::THROWN;
            throwable->nextState.attachedObjectId = -1;
            holder->nextState.heldObjectId = -1;
            holder->nextState.holdingObject = false;
            throwable->nextState.speed = throwable->throwSpeed;
        }
    }
    else if(throwable->state.aiState == Throwable::USED)
    {
        throwable->nextState.chargeTime = throwable->state.chargeTime + 1;
        if(throwable->nextState.chargeTime > throwable->useDuration)
        {
            throwable->nextState.aiState = Throwable::HELD;
        }

        Player * holder = dynamic_cast<Player*>(state->objects().at(throwable->state.attachedObjectId).get());

        throwable->nextState.pos = math_util::moveInDirection(holder->state.pos, holder->state.angle_deg - 30, holder->size.x);
        throwable->nextState.angle_deg = holder->state.angle_deg;

        switch(throwable->type())
        {
            case GameObject::KNIFE:
            {
                Knife * knife = dynamic_cast<Knife*>(throwable);

                float animationProgress = knife->state.chargeTime / (float)knife->useDuration;

                float angle = -40.0f * cos(animationProgress * M_PI * 2.0) + holder->state.angle_deg;

                throwable->nextState.pos = math_util::moveInDirection(holder->state.pos, angle, holder->size.x);
                throwable->nextState.angle_deg = angle;
                
                break;
            }
            case GameObject::GUN:
            {
                Gun * gun = dynamic_cast<Gun*>(throwable);
                throwable->nextState.angle_deg = math_util::angleBetween(throwable->state.pos, state->mousePos);

                if(throwable->nextState.chargeTime == 1)
                {
                    point_t direction = math_util::normalize(state->mousePos - gun->state.pos);
                    point_t bulletPos = gun->state.pos + direction * gun->size.x;
                    std::shared_ptr<Bullet> bullet(new Bullet(state->nextID()));
                    bullet->creatorId = holder->id;
                    bullet->state.pos = bulletPos;
                    bullet->velocity = direction * Bullet::SPEED;
                    bullet->state.angle_deg = math_util::angleBetween(gun->state.pos, state->mousePos);
                    bullet->initialTimeline = state->currentTimeline();
                    bullet->backwards = holder->backwards;
                    bullet->nextState = bullet->state;
                    if(holder->backwards)
                    {
                        bullet->ending = state->tick;
                        bullet->hasEnding = true;
                    }
                    else
                    {
                        bullet->beginning = state->tick;
                    }

                    holder->nextState.cooldown = holder->fireCooldown;

                    state->bullets().push_back(bullet.get());
                    state->objects()[bullet->id] = bullet;
                    state->historyBuffer().buffer[bullet->id] = std::vector<ObjectState>(state->tick+1);
                    state->historyBuffer().buffer[bullet->id][state->tick] = bullet->state;
                }

                break;
            }
            case GameObject::OBJECTIVE:
            {
                //Objectives don't do anything when used
                break;
            }
            default:
            {
                throw std::runtime_error("Unknown throwable type in tickThrowable");
            }
        }
    }
    else
    {
        throw std::runtime_error("Unknown throwable state");
    }
}

}