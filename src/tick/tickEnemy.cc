#include "tickEnemy.hh"

namespace tick{

bool playerVisibleToEnemy(GameState * state, Player* player, Enemy* enemy)
{
    bool visible = true;

    float angleToPlayer = math_util::angleBetween(enemy->state.pos, player->state.pos);
    float angleDiff = math_util::angleDiff(enemy->state.angle_deg, angleToPlayer);

    //Not in a box
    visible &= player->state.visible;
    //Within view angle of enemy
    visible &= (std::abs(angleDiff) < (Enemy::VIEW_ANGLE / 2.0f));
    //Close enough to see
    visible &= (math_util::dist(enemy->state.pos, player->state.pos) < Enemy::VIEW_RADIUS);
    //Not obstructed
    visible &= search::checkVisibility(state, enemy->state.pos, enemy->radius(), player->state.pos, player->radius());

    return visible;
}

void navigateEnemy(GameState * state, Enemy* enemy, point_t target)
{
    point_t moveToward = search::navigate(state, enemy->state.pos, target);

    for(Enemy * enemy2 : state->enemies())
    {
        if(enemy2 != enemy 
           && enemy2->state.aiState != Enemy::AI_DEAD 
           && math_util::dist(enemy->state.pos, enemy2->state.pos) < enemy->radius() * 2)
        {
            moveToward += math_util::normalize(enemy->state.pos - enemy2->state.pos) * (state->level->scale / 5);
        }
    }

    //If already at destination, or navigation failed, don't move
    if(moveToward == enemy->state.pos)
    {
        return;
    }
    enemy->nextState.pos += math_util::normalize(moveToward - enemy->state.pos) * enemy->moveSpeed;
    enemy->nextState.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, moveToward, 5.0f);

    if(search::checkObstruction(state, enemy->nextState.pos) && !search::checkObstruction(state, enemy->state.pos))
    {
        enemy->nextState.pos = enemy->state.pos;
    }
}

void tickEnemy(GameState * state, Enemy* enemy)
{
    if(!enemy->activeAt(state->tick))
    {
        return;
    }

    if(enemy->backwards != state->backwards())
    {
        enemy->nextState = state->historyBuffer()[enemy->id][state->tick];
        return;
    }

    enemy->nextState.animIdx = enemy->state.aiState;

    for(Bullet* bullet: state->bullets())
    {
        if(!bullet->activeAt(state->tick))
        {
            continue;
        }
        if(state->objects().at(bullet->creatorId)->type() != GameObject::PLAYER)
        {
            continue;
        }

        if(enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*bullet))
        {
            enemy->nextState.aiState = Enemy::AI_DEAD;
        }
    }
    for(Throwable* throwable: state->throwables())
    {
        if(!throwable->activeAt(state->tick))
        {
            continue;
        }

        if(throwable->deadly && throwable->state.aiState == Throwable::THROWN && enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*throwable))
        {
            enemy->nextState.aiState = Enemy::AI_DEAD;
        }
    }

    if(enemy->state.aiState == Enemy::AI_PATROL)
    {
        if(math_util::dist(enemy->state.pos, enemy->patrolPoints[enemy->state.patrolIdx]) <= state->level->scale / 3)
        {
            enemy->nextState.patrolIdx = (enemy->state.patrolIdx + 1) % enemy->patrolPoints.size();
        }

        navigateEnemy(state, enemy, enemy->patrolPoints[enemy->state.patrolIdx]);

        //Check if a player is seen
        for(Player* player : state->players())
        {
            if(playerVisibleToEnemy(state, player, enemy))
            {
                enemy->nextState.aiState = Enemy::AI_CHASE;
                enemy->nextState.targetId = player->id;
                enemy->nextState.lastSeen = player->state.pos;
                break;
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_CHASE)
    {
        Player* target = dynamic_cast<Player*>(state->objects().at(enemy->state.targetId).get());
        if(!target->activeAt(state->tick))
        {
            enemy->nextState.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(state, target, enemy))
        {
            enemy->nextState.lastSeen = target->state.pos;
            if(math_util::dist(enemy->state.pos, target->state.pos) < Enemy::ATTACK_RADIUS)
            {
                enemy->nextState.aiState = Enemy::AI_ATTACK;
            }
            else
            {
                navigateEnemy(state, enemy, target->state.pos);
            }
        }
        else
        {
            if(math_util::dist(enemy->state.pos, enemy->state.lastSeen) < enemy->moveSpeed)
            {
                enemy->nextState.aiState = Enemy::AI_PATROL;
            }
            else
            {
                navigateEnemy(state, enemy, enemy->state.lastSeen);
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_ATTACK)
    {
        Player* target = dynamic_cast<Player*>(state->objects().at(enemy->state.targetId).get());
        if(!target->activeAt(state->tick))
        {
            enemy->nextState.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(state, target, enemy))
        {
            enemy->nextState.lastSeen = target->state.pos;
            enemy->nextState.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, target->state.pos, 5.0f);

            if(enemy->state.chargeTime >= Enemy::ATTACK_CHARGE_TIME)
            {
                point_t direction = math_util::normalize(target->state.pos - enemy->state.pos);
                point_t bulletPos = enemy->state.pos + direction * enemy->size.x;
                std::shared_ptr<Bullet> bullet(new Bullet(state->nextID()));
                bullet->creatorId = enemy->id;
                bullet->state.pos = bulletPos;
                bullet->velocity = direction * Bullet::SPEED / 4.0f; //Enemy bullets are slower
                bullet->state.angle_deg = math_util::angleBetween(enemy->state.pos, target->state.pos);
                bullet->initialTimeline = state->currentTimeline();
                bullet->backwards = enemy->backwards;
                bullet->nextState = bullet->state;
                if(enemy->backwards)
                {
                    bullet->ending = state->tick;
                    bullet->hasEnding = true;
                }
                else
                {
                    bullet->beginning = state->tick;
                }

                state->bullets().push_back(bullet.get());
                state->objects()[bullet->id] = bullet;
                state->historyBuffer().buffer[bullet->id] = std::vector<ObjectState>(state->tick+1);
                state->historyBuffer().buffer[bullet->id][state->tick] = bullet->state;

                enemy->nextState.chargeTime = 0;

                std::cout << "Enemy " << enemy->id << " fired bullet " << bullet->id << std::endl;
            }
            //Continue an attack in progress as long as we can see the target
            else if(enemy->state.chargeTime > 0)
            {
                enemy->nextState.chargeTime++;
            }
            //But if not shooting, close distance first if needed
            else if(math_util::dist(enemy->state.pos, target->state.pos) > Enemy::ATTACK_RADIUS)
            {
                enemy->nextState.aiState = Enemy::AI_CHASE;
            }
            //If target is close enough, start charging
            else
            {
                enemy->nextState.chargeTime++;
            }
        }
        else
        {
            enemy->nextState.chargeTime = 0;
            enemy->nextState.aiState = Enemy::AI_CHASE;
            //If there is another visible target, swap to that.
            for(Player* player : state->players())
            {
                if(playerVisibleToEnemy(state, player, enemy))
                {
                    
                    enemy->nextState.targetId = player->id;
                    enemy->nextState.lastSeen = player->state.pos;
                    break;
                }
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_DEAD)
    {
        //Do nothing
    }
    else
    {
        throw std::runtime_error("Unknown AI state " + std::to_string(enemy->state.aiState) + " for enemy " + std::to_string(enemy->id));
    }
}

}