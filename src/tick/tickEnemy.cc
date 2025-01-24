#include "tickEnemy.hh"

namespace tick{

void createCrime(GameState * state, Enemy* enemy, Crime::CrimeType crimeType, GameObject* subject)
{
    //Check if this report matches an existing crime
    for(Crime * crime : state->crimes())
    {
        if(!crime->activeAt(state->tick) || crime->backwards != enemy->backwards)
        {
            continue;
        }

        if(crime->crimeType == crimeType
           && crime->subjectId == subject->id)
        {
            if(crime->crimeType == Crime::TRESPASSING)
            {
                crime->nextState.pos = subject->state.pos;
                crime->nextState.searchStatus = 0;
            }
            //Murder does not need to be updated based on the enemy still seeing the body

            //No need to create a new crime for the same thing, but assign this enemy to that alarm
            enemy->nextState.assignedAlarm = crime->state.assignedAlarm;
            return;
        }
    }

    int alarmId = enemy->state.assignedAlarm;
    if(alarmId == -1)
    {
        //Search for an existing alarm whose radius contains this crime
        for(Alarm * alarm : state->alarms())
        {
            if(alarm->state.alarmRadius > math_util::dist(alarm->state.pos, subject->state.pos))
            {
                alarmId = alarm->id;
                break;
            }
        }

        //If no alarm found, create a new one
        std::shared_ptr<Alarm> alarm(new Alarm(state->nextID()));
        alarm->state.pos = enemy->state.pos;
        alarm->initialTimeline = state->currentTimeline();
        alarm->backwards = enemy->backwards;
        alarm->state.alarmRadius = Alarm::DEFAULT_RADIUS;
        alarm->nextState = alarm->state;
        if(enemy->backwards)
        {
            alarm->ending = state->tick;
            alarm->hasEnding = true;
        }
        else
        {
            alarm->beginning = state->tick;
        }

        state->alarms().push_back(alarm.get());
        state->objects()[alarm->id] = alarm;
        state->historyBuffer().buffer[alarm->id] = std::vector<ObjectState>(state->tick+1);
        state->historyBuffer().buffer[alarm->id][state->tick] = alarm->state;

        alarmId = alarm->id;

        std::cout << "Alarm " << alarm->id << " created on tick " << state->tick << std::endl;
    }



    std::shared_ptr<Crime> crime(new Crime(state->nextID()));
    crime->state.pos = subject->state.pos;
    crime->crimeType = crimeType;
    crime->subjectId = subject->id;
    crime->state.assignedAlarm = alarmId;
    crime->initialTimeline = state->currentTimeline();
    crime->backwards = enemy->backwards;
    crime->nextState = crime->state;
    if(enemy->backwards)
    {
        crime->ending = state->tick;
        crime->hasEnding = true;
    }
    else
    {
        crime->beginning = state->tick;
    }

    state->crimes().push_back(crime.get());
    state->objects()[crime->id] = crime;
    state->historyBuffer().buffer[crime->id] = std::vector<ObjectState>(state->tick+1);
    state->historyBuffer().buffer[crime->id][state->tick] = crime->state;

    std::cout << "Crime " << crime->id << " created on tick " << state->tick << std::endl;
}

void reportCrimes(GameState * state, Enemy* enemy)
{
    //Trespassing
    for(Player* player : state->players())
    {
        if(!player->activeAt(state->tick))
        {
            continue;
        }
        if(playerVisibleToEnemy(state, player, enemy))
        {
            createCrime(state, enemy, Crime::TRESPASSING, player);   
        }
    }

    //Murder
    for(Enemy* other : state->enemies())
    {
        if(other->activeAt(state->tick) 
            && other->state.aiState == Enemy::AI_DEAD
            && !other->state.discovered
            && pointVisibleToEnemy(state, other->state.pos, enemy))
        {
            createCrime(state, enemy, Crime::MURDER, other);
            //Note this may cause problems if there are enemies with opposite arrows of time
            other->nextState.discovered = true;
        }
    }
}

void reportSearches(GameState * state, Enemy* enemy)
{
    if(enemy->state.assignedAlarm == -1)
    {
        return;
    }

    Alarm * alarm = dynamic_cast<Alarm*>(state->objects().at(enemy->state.assignedAlarm).get());
    for(Crime * crime : state->crimes())
    {
        for(int x = -Crime::SEARCH_RADIUS; x <= Crime::SEARCH_RADIUS; x++)
        {
            for(int y = -Crime::SEARCH_RADIUS; y <= Crime::SEARCH_RADIUS; y++)
            {
                if(crime->isSearched(x, y))
                {
                    continue;
                }
                point_t crimePos = state->level->toLevelCoords(crime->state.pos);
                point_t searchPos = crimePos + point_t(x, y);
                if(searchPos.x < 0 || searchPos.x >= state->level->width || searchPos.y < 0 || searchPos.y >= state->level->height)
                {
                    //Out of bounds, so just check it off
                    crime->submitSearch(x, y);
                }
                else if(state->obstructionGrid[searchPos.x][searchPos.y])
                {
                    //Obstructed, so just check it off
                    crime->submitSearch(x, y);
                }
                else if(pointVisibleToEnemy(state, state->level->fromLevelCoords(searchPos), enemy))
                {
                    crime->submitSearch(x, y);
                    if(crime->nextState.searchStatus == Crime::FULLY_SEARCHED())
                    {
                        std::cout << "Crime " << crime->id << " finished on tick " << state->tick << std::endl;
                        if(crime->backwards)
                        {
                            crime->beginning = state->tick;
                        }
                        else
                        {
                            crime->hasEnding = true;
                            crime->ending = state->tick;
                        }
                    }
                }
            }
        }
    }
}

float crimePriority(GameState * state, Crime* crime, Enemy* enemy)
{
    const int BASE_PRIORITY = 100;
    const int TRESPASSING_MULTIPLIER = 5;
    const int MURDER_MULTIPLIER = 1;
    const float DISTANCE_MULTIPLIER = 1;
    const float ANGLE_MULTIPLIER = 1;


    float priority = 0;

    if(crime->crimeType == Crime::TRESPASSING)
    {
        priority = BASE_PRIORITY * TRESPASSING_MULTIPLIER;
    }
    else if(crime->crimeType == Crime::MURDER)
    {
        priority = BASE_PRIORITY * MURDER_MULTIPLIER;
    }

    priority -= math_util::dist(enemy->state.pos, crime->state.pos) * DISTANCE_MULTIPLIER;

    priority -= abs(math_util::angleDiff(
        math_util::angleBetween(enemy->state.pos, crime->state.pos),
        enemy->state.angle_deg)) * ANGLE_MULTIPLIER;

    return priority;
}

float searchPriority(GameState * state, point_t pos, Enemy* enemy)
{
    const float DISTANCE_MULTIPLIER = 1;
    const float ANGLE_MULTIPLIER = 1;

    float priority = 0;

    priority -= math_util::dist(enemy->state.pos, pos) * DISTANCE_MULTIPLIER;

    priority -= abs(math_util::angleDiff(
        math_util::angleBetween(enemy->state.pos, pos),
        enemy->state.angle_deg)) * ANGLE_MULTIPLIER;

    return priority;
}

bool pointVisibleToEnemy(GameState * state, point_t point, Enemy * enemy)
{
    bool visible = true;

    float angleToPoint = math_util::angleBetween(enemy->state.pos, point);
    float angleDiff = math_util::angleDiff(enemy->state.angle_deg, angleToPoint);

    //Within view angle of enemy
    visible &= (std::abs(angleDiff) < (Enemy::VIEW_ANGLE / 2.0f));
    //Close enough to see
    visible &= (math_util::dist(enemy->state.pos, point) < Enemy::VIEW_RADIUS);
    //Not obstructed
    visible &= search::checkVisibility(state, enemy->state.pos, enemy->radius(), point, 0);

    return visible;
}

bool playerVisibleToEnemy(GameState * state, Player* player, Enemy* enemy)
{
    return player->state.visible && pointVisibleToEnemy(state, player->state.pos, enemy);
}

void navigateEnemy(GameState * state, Enemy* enemy, point_t target)
{
    point_t moveToward = search::navigate(state, enemy->state.pos, target);
    //If already at destination, or navigation failed, don't move
    if(moveToward == enemy->state.pos)
    {
        return;
    }

    point_t moveVec = math_util::normalize(moveToward - enemy->state.pos);

    for(Enemy * enemy2 : state->enemies())
    {
        if(enemy2 != enemy 
           && enemy2->state.aiState != Enemy::AI_DEAD 
           && math_util::dist(enemy->state.pos, enemy2->state.pos) < enemy->radius() * 2)
        {
            moveVec += 0.5f * math_util::normalize(enemy->state.pos - enemy2->state.pos);
            moveVec = math_util::normalize(moveVec);
        }
    }
    
    enemy->nextState.pos += moveVec * enemy->moveSpeed;
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

    for(auto promise: state->promises)
    {
        if(promise->target == enemy->id && promise->activatedTimeline < 0)
        {
            enemy->nextState.visible = false;
            return;
        }
    }

    enemy->nextState.visible = true;

    if(enemy->state.aiState != Enemy::AI_DEAD)
    {
        reportCrimes(state, enemy);
        reportSearches(state, enemy);

        if(enemy->state.assignedAlarm == -1)
        {
            for(Alarm * alarm : state->alarms())
            {
                if(alarm->activeAt(state->tick) && math_util::dist(enemy->state.pos, alarm->state.pos) < alarm->state.alarmRadius)
                {
                    enemy->nextState.assignedAlarm = alarm->id;
                    break;
                }
            }
        }
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

        if(enemy->state.assignedAlarm != -1)
        {
            enemy->nextState.aiState = Enemy::AI_SEARCH;
        }

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
    else if(enemy->state.aiState == Enemy::AI_SEARCH)
    {
        Alarm * alarm = dynamic_cast<Alarm*>(state->objects().at(enemy->state.assignedAlarm).get());
        if(!alarm->activeAt(state->tick) || alarm->crimes.size() == 0)
        {
            enemy->nextState.aiState = Enemy::AI_PATROL;
            enemy->nextState.assignedAlarm = -1;
        }
        else
        {
            float bestPriority = -1e12;
            Crime * bestCrime = nullptr;
            for(int crimeId : alarm->crimes)
            {
                Crime * crime = dynamic_cast<Crime*>(state->objects().at(crimeId).get());
                if(crimePriority(state, crime, enemy) > bestPriority)
                {
                    bestPriority = crimePriority(state, crime, enemy);
                    bestCrime = crime;
                }
            }

            if(bestCrime == nullptr)
            {
                throw std::runtime_error("No best crime found. This should never happen!");
            }
            
            bestPriority = -1e12;
            point_t bestSearchPos = enemy->state.pos;
            for(int x=-Crime::SEARCH_RADIUS; x<=Crime::SEARCH_RADIUS; x++)
            {
                for(int y=-Crime::SEARCH_RADIUS; y<=Crime::SEARCH_RADIUS; y++)
                {
                    if(bestCrime->isSearched(x, y))
                    {
                        continue;
                    }
                    point_t searchPos = bestCrime->state.pos + (point_t(x * state->level->scale, y * state->level->scale));
                    float priority = searchPriority(state, searchPos, enemy);
                    if(priority > bestPriority)
                    {
                        bestPriority = priority;
                        bestSearchPos = searchPos;
                    }
                }
            }

            navigateEnemy(state, enemy, bestSearchPos);
        }

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
    else
    {
        throw std::runtime_error("Unknown AI state " + std::to_string(enemy->state.aiState) + " for enemy " + std::to_string(enemy->id));
    }
}

}