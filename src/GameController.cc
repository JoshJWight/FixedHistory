#include "GameController.hh"

GameController::GameController(const std::string & levelPath, Graphics * graphics)
    : m_graphics(graphics)
    , m_currentTick(-1)//Start at -1 so that the first tick is 0
    , m_currentTimeline(0)
    , m_shouldReverse(false)
    , m_backwards(false)
    , m_boxToEnter(-1)
{
    m_gameState = loadGameState(levelPath);
    m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());
}

bool GameController::mainLoop()
{
    size_t tickCounter = 0;

    int playbackSpeed = 1;

    //~60 FPS
    int msPerFrame = 16;
    std::chrono::duration<int, std::milli> frameDuration(msPerFrame);

    bool paradox = false;
    bool win = false;
    int winTimer = 0;

    bool rewinding = false;
    int timeRewinding = 0;

    auto lastDraw = std::chrono::system_clock::now();

    while(true)
    {
        tickCounter++;

        m_controls.tick();

        if(m_controls.restart)
        {
            return false;
        }

        TickType type = PAUSE;

        if(m_controls.rewind)
        {
            rewinding = true;
        }

        if(rewinding)
        {
            m_statusString = "";
            type = REWIND;
            paradox = false;
            win = false;
            winTimer = 0;
            playbackSpeed = 3;

            timeRewinding++;
            if(timeRewinding > 150 && !m_controls.rewind)
            {
                rewinding = false;
                timeRewinding = 0;
                playbackSpeed = 1;
            }
        }
        else if(!paradox && !win)
        {
            m_statusString = "";
            paradox = checkParadoxes();
            win = checkWin();
            if(!paradox && !win)
            {
                type = ADVANCE;

                if(m_gameState->currentPlayer()->state.boxOccupied)
                {
                    playbackSpeed = 2;
                }
                else
                {
                    playbackSpeed = 1;
                }
            }
        }

        tick(type);

        point_t cameraCenter = m_gameState->currentPlayer()->state.pos;

        if(win)
        {
            winTimer++;
            if(winTimer > 200)
            {
                return true;
            }
        }

        if(tickCounter % playbackSpeed == 0)
        {
            m_graphics->draw(m_gameState.get(), m_currentTick, cameraCenter, m_statusString);
            std::this_thread::sleep_until(lastDraw + frameDuration);
            lastDraw = std::chrono::system_clock::now();
        }
    }

    return true;
}

bool GameController::checkParadoxes()
{
    //Note: m_currentTick is still the tick we just finished doing

    //Player being hit by a bullet
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_currentTick))
        {
            continue;
        }

        for(Bullet* bullet : m_gameState->bullets())
        {
            if(bullet->activeAt(m_currentTick) && player->isColliding(*bullet))
            {
                if(player->id == m_gameState->currentPlayer()->id)
                {
                    m_statusString = "YOU GOT SHOT";
                }
                else{
                    m_statusString = "A PAST YOU GOT SHOT";
                }
                return true;
            }
        }
    }
    //Player standing on spikes
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_currentTick))
        {
            continue;
        }

        for(Spikes* spikes : m_gameState->spikes())
        {
            if(spikes->activeAt(m_currentTick) && spikes->state.aiState == Spikes::UP && player->isColliding(*spikes))
            {
                if(player->id == m_gameState->currentPlayer()->id)
                {
                    m_statusString = "YOU GOT SPIKED";
                }
                else{
                    m_statusString = "A PAST YOU GOT SPIKED";
                }
                return true;
            }
        }
    }

    //Violation of observations
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_currentTick))
        {
            continue;
        }
        if(player->recorded)
        {
            std::string result = observation::checkObservations(m_gameState.get(), player, m_currentTick);
            if(result != "")
            {
                m_statusString = result;
                return true;
            }
        }
    }

    //If we got here, no paradoxes
    return false;
}

bool GameController::checkWin()
{
    Player* player = m_gameState->currentPlayer();

    if(!player->state.holdingObject || m_gameState->objects().at(player->state.heldObjectId)->type() != GameObject::OBJECTIVE)
    {
        return false;
    }

    for(Exit* exit : m_gameState->exits())
    {
        if(exit->activeAt(m_currentTick) && player->isColliding(*exit))
        {
            m_statusString = "YOU WIN";
            return true;
        }
    }

    return false;
}

void GameController::restoreState(int tick)
{
    m_gameState->restoreState(tick);
    m_gameState->doRewindCleanup(tick, m_currentTimeline);
}

void GameController::popTimeline()
{
    std::cout << "Popping timeline on tick " << m_currentTick << std::endl;

    if(m_gameState->timelines.size() == 1)
    {
        throw std::runtime_error("Cannot pop the last timeline");
    }

    //m_backwards now refers to the timeline we're popping to
    m_backwards = !m_backwards;
    m_currentTimeline--;
    //Simply blow away the current timeline, which will return us to how things were before the push
    m_gameState->timelines.pop_back();

    restoreState(m_currentTick);
}
    
void GameController::pushTimeline()
{
    std::cout << "Pushing timeline on tick " << m_currentTick << std::endl;

    //m_backwards now refers to the timeline we're pushing to
    m_backwards = !m_backwards;
    m_currentTimeline++;
    m_gameState->timelines.emplace_back(m_gameState->timelines.back(), m_currentTick, m_backwards);

    //Create a new player entity
    Player* oldPlayer = m_gameState->currentPlayer();
    std::shared_ptr<Player> newPlayer(new Player(m_gameState->nextID(), oldPlayer));
    newPlayer->backwards = m_backwards;
    std::cout << "Creating new player with ID " << newPlayer->id << std::endl;
    oldPlayer->recorded = true;

    oldPlayer->hasFinalTimeline = true;
    oldPlayer->finalTimeline = m_currentTimeline - 1;
    newPlayer->initialTimeline = m_currentTimeline;
    if(m_backwards)
    {
        oldPlayer->hasEnding = true;
        oldPlayer->ending = m_currentTick;
        newPlayer->hasEnding = true;
        newPlayer->ending = m_currentTick;
        newPlayer->beginning = 0;
    }
    else
    {
        oldPlayer->beginning = m_currentTick;
        newPlayer->beginning = m_currentTick;
        newPlayer->hasEnding = false;
    }
    m_gameState->objects()[newPlayer->id] = newPlayer;
    m_gameState->players().push_back(newPlayer.get());

    if(m_boxToEnter > -1)
    {
        Container* box = dynamic_cast<Container*>(m_gameState->objects().at(m_boxToEnter).get());
        box->activeOccupant = newPlayer->id;

        newPlayer->state.boxOccupied = true;
        newPlayer->state.attachedObjectId = box->id;
        newPlayer->state.pos = box->state.pos;
        newPlayer->state.visible = false;
    }
    else if(oldPlayer->state.boxOccupied)
    {
        Container* box = dynamic_cast<Container*>(m_gameState->objects().at(oldPlayer->state.attachedObjectId).get());

        newPlayer->state.boxOccupied = false;
        newPlayer->state.attachedObjectId = -1;
        newPlayer->state.visible = true;
        //newPlayer->state.pos = math_util::moveInDirection(oldPlayer->state.pos, box->state.angle_deg, box->size.x);

        box->activeOccupant = -1;
    }

    if(oldPlayer->state.holdingObject)
    {
        std::cout << "Holding object when timeline was pushed" << std::endl;
        Throwable* oldHeldObject;
        std::shared_ptr<Throwable> newHeldObject;

        if(m_gameState->objects().at(oldPlayer->state.heldObjectId)->type() == GameObject::OBJECTIVE)
        {
            oldHeldObject = dynamic_cast<Objective*>(m_gameState->objects().at(oldPlayer->state.heldObjectId).get());
            newHeldObject = std::make_shared<Objective>(m_gameState->nextID(), dynamic_cast<Objective*>(m_gameState->objects().at(oldPlayer->state.heldObjectId).get()));
        }
        else if(m_gameState->objects().at(oldPlayer->state.heldObjectId)->type() == GameObject::KNIFE)
        {
            oldHeldObject = dynamic_cast<Knife*>(m_gameState->objects().at(oldPlayer->state.heldObjectId).get());
            newHeldObject = std::make_shared<Knife>(m_gameState->nextID(), dynamic_cast<Knife*>(m_gameState->objects().at(oldPlayer->state.heldObjectId).get()));
        }
        else
        {
            throw std::runtime_error("Unknown object type held by player");
        }
        newHeldObject->backwards = newPlayer->backwards;
        newPlayer->state.heldObjectId = newHeldObject->id;
        //newHeldObject->state.pos = newPlayer->state.pos;
        newHeldObject->state.visible = newPlayer->state.visible;
        newHeldObject->state.attachedObjectId = newPlayer->id;

        oldHeldObject->hasFinalTimeline = true;
        oldHeldObject->finalTimeline = m_currentTimeline - 1;
        newHeldObject->initialTimeline = m_currentTimeline;
        if(m_backwards)
        {
            oldHeldObject->ending = m_currentTick;
            oldHeldObject->hasEnding = true;
            newHeldObject->ending = m_currentTick;
            newHeldObject->hasEnding = true;
            newHeldObject->beginning = 0;
        }
        else
        {
            oldHeldObject->beginning = m_currentTick;
            newHeldObject->beginning = m_currentTick;
            newHeldObject->hasEnding = false;
        }
        m_gameState->objects()[newHeldObject->id] = newHeldObject;
        m_gameState->historyBuffer().buffer[newHeldObject->id] = std::vector<ObjectState>(m_currentTick+1);
        m_gameState->historyBuffer().buffer[newHeldObject->id][m_currentTick] = newHeldObject->state;
        m_gameState->throwables().push_back(newHeldObject.get());
    }

    //Add a buffer to the new history buffer for the new player
    m_gameState->historyBuffer().buffer[newPlayer->id] = std::vector<ObjectState>(m_currentTick+1);
    m_gameState->historyBuffer().buffer[newPlayer->id][m_currentTick] = newPlayer->state;

    newPlayer->observations.resize(m_currentTick+1);
    observation::recordObservations(m_gameState.get(), newPlayer.get(), m_currentTick);

    //Delete all transient objects whose origin is "after" the breakpoint
    std::vector<int> toDelete;
    for(auto pair : m_gameState->objects())
    {
        GameObject* obj = pair.second.get();
        if(m_backwards)
        {
            if(obj->isTransient() && obj->backwards && obj->hasEnding && obj->ending < m_currentTick)
            {
                toDelete.push_back(obj->id);
            }
        }
        else
        {
            if(obj->isTransient() && !obj->backwards && obj->beginning > m_currentTick)
            {
                toDelete.push_back(obj->id);
            }
        }
    }
    for(int id : toDelete)
    {
        m_gameState->deleteObject(id);
    }

}

void GameController::tickPlayer(Player* player)
{
    player->nextState.willInteract = m_controls.interact;
    player->nextState.willThrow = m_controls.throw_;

    if(player->state.boxOccupied)
    {
        if(player->state.willInteract)
        {
            std::cout << "Exiting box" << std::endl;
            Container * container = dynamic_cast<Container*>(m_gameState->objects().at(player->state.attachedObjectId).get());
            if(container->reverseOnExit)
            {
                m_shouldReverse = true;
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
                    Throwable* throwable = dynamic_cast<Throwable*>(m_gameState->objects().at(player->state.heldObjectId).get());
                    throwable->nextState.visible = true;
                }
            }
        }

        //Player can't move or act while in a box
        return;
    }
    
    if(player->state.willInteract)
    {
        for(Container* container: m_gameState->containers())
        {
            if(math_util::dist(player->state.pos, container->state.pos) < (container->size.x + Player::INTERACT_RADIUS)
                && !container->state.boxOccupied)
            {
                if(container->reverseOnEnter)
                {
                    m_shouldReverse = true;
                    m_boxToEnter = container->id;
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
                        Throwable* throwable = dynamic_cast<Throwable*>(m_gameState->objects().at(player->state.heldObjectId).get());
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

    if(m_controls.up)
    {
        player->nextState.pos.y += player->moveSpeed;
    }
    if(m_controls.down)
    {
        player->nextState.pos.y -= player->moveSpeed;
    }
    if(m_controls.left)
    {
        player->nextState.pos.x -= player->moveSpeed;
    }
    if(m_controls.right)
    {
        player->nextState.pos.x += player->moveSpeed;
    }

    //No walking through walls or obstructions
    if(search::checkObstruction(m_gameState.get(), player->nextState.pos) && !search::checkObstruction(m_gameState.get(), player->state.pos))
    {
        player->nextState.pos = player->state.pos;
    }

    point_t mouseWorldPos = m_graphics->getMousePos();
    if(m_controls.fire && player->state.cooldown == 0)
    {
        point_t direction = math_util::normalize(mouseWorldPos - player->state.pos);
        point_t bulletPos = player->state.pos + direction * player->size.x;
        std::shared_ptr<Bullet> bullet(new Bullet(m_gameState->nextID()));
        bullet->creatorId = player->id;
        bullet->state.pos = bulletPos;
        bullet->velocity = direction * Bullet::SPEED;
        bullet->state.angle_deg = math_util::angleBetween(player->state.pos, mouseWorldPos);
        bullet->initialTimeline = m_currentTimeline;
        bullet->backwards = player->backwards;
        bullet->nextState = bullet->state;
        if(player->backwards)
        {
            bullet->ending = m_currentTick;
            bullet->hasEnding = true;
        }
        else
        {
            bullet->beginning = m_currentTick;
        }

        player->nextState.cooldown = player->fireCooldown;

        m_gameState->bullets().push_back(bullet.get());
        m_gameState->objects()[bullet->id] = bullet;
        m_gameState->historyBuffer().buffer[bullet->id] = std::vector<ObjectState>(m_currentTick+1);
        m_gameState->historyBuffer().buffer[bullet->id][m_currentTick] = bullet->state;
    }

    player->nextState.angle_deg = math_util::rotateTowardsPoint(player->state.angle_deg, player->state.pos, mouseWorldPos, 5.0f);
}

void GameController::tickBullet(Bullet* bullet)
{
    if(!bullet->activeAt(m_currentTick))
    {
        return;
    }

    if(bullet->backwards != m_backwards)
    {
        bullet->nextState = m_gameState->historyBuffer()[bullet->id][m_currentTick];
        return;
    }

    bullet->nextState.pos += bullet->velocity;

    if(m_gameState->level->tileAt(bullet->state.pos) == Level::WALL)
    {

        bullet->finalTimeline = m_currentTimeline;
        bullet->hasFinalTimeline = true;
        if(bullet->backwards)
        {
            bullet->beginning = m_currentTick;
        }
        else
        {
            bullet->ending = m_currentTick;
            bullet->hasEnding = true;
        }
    }
}

bool GameController::playerVisibleToEnemy(Player* player, Enemy* enemy)
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
    visible &= search::checkVisibility(m_gameState.get(), enemy->state.pos, enemy->radius(), player->state.pos, player->radius());

    return visible;
}

void GameController::navigateEnemy(Enemy* enemy, point_t target)
{
    point_t moveToward = search::navigate(m_gameState.get(), enemy->state.pos, target);

    for(Enemy * enemy2 : m_gameState->enemies())
    {
        if(enemy2 != enemy 
           && enemy2->state.aiState != Enemy::AI_DEAD 
           && math_util::dist(enemy->state.pos, enemy2->state.pos) < enemy->radius() * 2)
        {
            moveToward += math_util::normalize(enemy->state.pos - enemy2->state.pos) * (m_gameState->level->scale / 5);
        }
    }

    //If already at destination, or navigation failed, don't move
    if(moveToward == enemy->state.pos)
    {
        return;
    }
    enemy->nextState.pos += math_util::normalize(moveToward - enemy->state.pos) * enemy->moveSpeed;
    enemy->nextState.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, moveToward, 5.0f);

    if(search::checkObstruction(m_gameState.get(), enemy->nextState.pos) && !search::checkObstruction(m_gameState.get(), enemy->state.pos))
    {
        enemy->nextState.pos = enemy->state.pos;
    }
}

void GameController::tickEnemy(Enemy* enemy)
{
    if(!enemy->activeAt(m_currentTick))
    {
        return;
    }

    if(enemy->backwards != m_backwards)
    {
        enemy->nextState = m_gameState->historyBuffer()[enemy->id][m_currentTick];
        return;
    }

    enemy->nextState.animIdx = enemy->state.aiState;

    for(Bullet* bullet: m_gameState->bullets())
    {
        if(!bullet->activeAt(m_currentTick))
        {
            continue;
        }
        if(m_gameState->objects().at(bullet->creatorId)->type() != GameObject::PLAYER)
        {
            continue;
        }

        if(enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*bullet))
        {
            enemy->nextState.aiState = Enemy::AI_DEAD;
        }
    }
    for(Throwable* throwable: m_gameState->throwables())
    {
        if(!throwable->activeAt(m_currentTick))
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
        if(math_util::dist(enemy->state.pos, enemy->patrolPoints[enemy->state.patrolIdx]) <= m_gameState->level->scale / 3)
        {
            enemy->nextState.patrolIdx = (enemy->state.patrolIdx + 1) % enemy->patrolPoints.size();
        }

        navigateEnemy(enemy, enemy->patrolPoints[enemy->state.patrolIdx]);

        //Check if a player is seen
        for(Player* player : m_gameState->players())
        {
            if(playerVisibleToEnemy(player, enemy))
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
        Player* target = dynamic_cast<Player*>(m_gameState->objects().at(enemy->state.targetId).get());
        if(!target->activeAt(m_currentTick))
        {
            enemy->nextState.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(target, enemy))
        {
            enemy->nextState.lastSeen = target->state.pos;
            if(math_util::dist(enemy->state.pos, target->state.pos) < Enemy::ATTACK_RADIUS)
            {
                enemy->nextState.aiState = Enemy::AI_ATTACK;
            }
            else
            {
                navigateEnemy(enemy, target->state.pos);
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
                navigateEnemy(enemy, enemy->state.lastSeen);
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_ATTACK)
    {
        Player* target = dynamic_cast<Player*>(m_gameState->objects().at(enemy->state.targetId).get());
        if(!target->activeAt(m_currentTick))
        {
            enemy->nextState.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(target, enemy))
        {
            enemy->nextState.lastSeen = target->state.pos;
            enemy->nextState.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, target->state.pos, 5.0f);

            if(enemy->state.chargeTime >= Enemy::ATTACK_CHARGE_TIME)
            {
                point_t direction = math_util::normalize(target->state.pos - enemy->state.pos);
                point_t bulletPos = enemy->state.pos + direction * enemy->size.x;
                std::shared_ptr<Bullet> bullet(new Bullet(m_gameState->nextID()));
                bullet->creatorId = enemy->id;
                bullet->state.pos = bulletPos;
                bullet->velocity = direction * Bullet::SPEED / 4.0f; //Enemy bullets are slower
                bullet->state.angle_deg = math_util::angleBetween(enemy->state.pos, target->state.pos);
                bullet->initialTimeline = m_currentTimeline;
                bullet->backwards = enemy->backwards;
                bullet->nextState = bullet->state;
                if(enemy->backwards)
                {
                    bullet->ending = m_currentTick;
                    bullet->hasEnding = true;
                }
                else
                {
                    bullet->beginning = m_currentTick;
                }

                m_gameState->bullets().push_back(bullet.get());
                m_gameState->objects()[bullet->id] = bullet;
                m_gameState->historyBuffer().buffer[bullet->id] = std::vector<ObjectState>(m_currentTick+1);
                m_gameState->historyBuffer().buffer[bullet->id][m_currentTick] = bullet->state;

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
            for(Player* player : m_gameState->players())
            {
                if(playerVisibleToEnemy(player, enemy))
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


void GameController::tickContainer(Container* container)
{
    if(container->activeOccupant > -1)
    {
        container->nextState.boxOccupied = true;
        container->nextState.attachedObjectId = container->activeOccupant;

        //If about to run into a point where someone else was in the box, kick the current occupant out
        for(int i=1; i<Container::OCCUPANCY_SPACING + 300; i++)
        {
            int timestepToCheck;
            if(m_backwards)
            {
                timestepToCheck = m_currentTick - i;
            }
            else
            {
                timestepToCheck = m_currentTick + i;
            }

            if(timestepToCheck < 0 || timestepToCheck >= m_gameState->historyBuffer()[container->id].size())
            {
                break;
            }

            ObjectState & state = m_gameState->historyBuffer()[container->id][timestepToCheck];
            if(state.boxOccupied && state.attachedObjectId != container->activeOccupant)
            {
                if(i<Container::OCCUPANCY_SPACING)
                {
                    if(container->reverseOnExit)
                    {
                        m_shouldReverse = true;
                    }
                    else
                    {
                        GameObject* occupant = m_gameState->objects().at(container->activeOccupant).get();
                        if(occupant->state.holdingObject)
                        {
                            Throwable* throwable = dynamic_cast<Throwable*>(m_gameState->objects().at(occupant->state.heldObjectId).get());
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
                    m_statusString = "AUTO-EJECT IN " + std::to_string(secondsLeft);
                }
                break;
            }
        }
    } 
    else if(m_currentTick >= m_gameState->historyBuffer()[container->id].size())
    {
        container->nextState.boxOccupied = false;
        container->nextState.attachedObjectId = -1;
    }
    else
    {
        container->nextState = m_gameState->historyBuffer()[container->id][m_currentTick];
    }
}

void GameController::tickSwitch(Switch* sw)
{
    if(sw->backwards != m_backwards)
    {
        sw->nextState = m_gameState->historyBuffer()[sw->id][m_currentTick];
        return;
    }

    for(Player* player : m_gameState->players())
    {
        if(player->state.willInteract
            && math_util::dist(player->state.pos, sw->state.pos) < (sw->size.x + Player::INTERACT_RADIUS)
            && player->backwards == m_backwards)
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

void GameController::tickDoor(Door* door)
{
    if(!door->activeAt(m_currentTick))
    {
        return;
    }

    if(door->backwards != m_backwards)
    {
        door->nextState = m_gameState->historyBuffer()[door->id][m_currentTick];
        return;
    }

    int onSwitches = 0;
    for(int swId : door->getConnectedSwitches())
    {
        Switch* sw = dynamic_cast<Switch*>(m_gameState->objects().at(swId).get());
        if(sw->state.aiState == Switch::ON)
        {
            onSwitches++;
        }
    }

    door->nextState.aiState = (onSwitches % 2 == 1) ? Door::OPEN : Door::CLOSED;
    door->nextState.animIdx = door->nextState.aiState;
}

void GameController::tickSpikes(Spikes* spikes)
{
    if(!spikes->activeAt(m_currentTick))
    {
        return;
    }

    if(spikes->backwards != m_backwards)
    {
        spikes->nextState = m_gameState->historyBuffer()[spikes->id][m_currentTick];
        return;
    }

    int pointInCycle = (m_currentTick + spikes->cycleOffset) % (spikes->downDuration + spikes->upDuration);
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

void GameController::tickThrowable(Throwable* throwable)
{
    if(!throwable->activeAt(m_currentTick))
    {
        return;
    }

    if(throwable->backwards != m_backwards)
    {
        //TODO we may want to have backwards throwables be usable in the future
        throwable->nextState = m_gameState->historyBuffer()[throwable->id][m_currentTick];
        return;
    }

    if(throwable->state.aiState == Throwable::STILL)
    {
        for(Player* player : m_gameState->players())
        {
            if(player->state.willThrow
                && math_util::dist(player->state.pos, throwable->state.pos) < (throwable->size.x + Player::INTERACT_RADIUS)
                && player->backwards == m_backwards
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
        if(search::checkObstruction(m_gameState.get(), nextPos))
        {
            float bounceAngle = search::bounceOffWall(m_gameState.get(), throwable->state.pos, nextPos);
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

        for(Enemy* enemy : m_gameState->enemies())
        {
            if(enemy->activeAt(m_currentTick) && enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*throwable))
            {
                throwable->nextState.aiState = Throwable::STILL;
                throwable->nextState.speed = 0.0f;
                break;
            }
        }
    }
    else if(throwable->state.aiState == Throwable::HELD)
    {
        Player * holder = dynamic_cast<Player*>(m_gameState->objects().at(throwable->state.attachedObjectId).get());
        throwable->nextState.pos = math_util::moveInDirection(holder->state.pos, holder->state.angle_deg - 30, holder->size.x);
        throwable->nextState.angle_deg = holder->state.angle_deg;
        if(holder->state.willThrow && !holder->state.boxOccupied)
        {
            throwable->nextState.aiState = Throwable::THROWN;
            throwable->nextState.attachedObjectId = -1;
            holder->nextState.heldObjectId = -1;
            holder->nextState.holdingObject = false;
            throwable->nextState.speed = throwable->throwSpeed;
        }
    }
    else
    {
        throw std::runtime_error("Unknown throwable state");
    }
}

void GameController::playTick()
{
    tickPlayer(m_gameState->currentPlayer());
    for(Bullet* bullet : m_gameState->bullets())
    {
        tickBullet(bullet);
    }

    for(Enemy* enemy : m_gameState->enemies())
    {
        tickEnemy(enemy);
    }

    for(Container* container : m_gameState->containers())
    {
        tickContainer(container);
    }

    for(Switch* sw : m_gameState->switches())
    {
        tickSwitch(sw);
    }

    for(Door* door : m_gameState->doors())
    {
        tickDoor(door);
    }

    for(Spikes* spikes : m_gameState->spikes())
    {
        tickSpikes(spikes);
    }

    for(Throwable* throwable : m_gameState->throwables())
    {
        tickThrowable(throwable);
    }

    //Apply next states to current states
    for(auto pair : m_gameState->objects())
    {
        std::shared_ptr<GameObject> obj = pair.second;
        if(!obj->activeAt(m_currentTick))
        {
            continue;
        }

        if(!obj->recorded)
        {
            obj->applyNextState();
            if(m_currentTick == m_gameState->historyBuffer()[obj->id].size())
            {  
                m_gameState->historyBuffer()[obj->id].push_back(obj->state);
            }
            else if(m_currentTick > m_gameState->historyBuffer()[obj->id].size())
            {
                throw std::runtime_error("It is tick " + std::to_string(m_currentTick) + " but object " + std::to_string(obj->id) + " has history buffer size " + std::to_string(m_gameState->historyBuffer()[obj->id].size()));
            }
            else
            {
                m_gameState->historyBuffer()[obj->id][m_currentTick] = obj->state;
            }
        }
        else
        {
            obj->state = m_gameState->historyBuffer()[obj->id][m_currentTick];
        }
    }
    //Creating it both here and elsewhere because we want it to be right before recoring observations
    m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());
    observation::recordObservations(m_gameState.get(), m_gameState->currentPlayer(), m_currentTick);
}

void GameController::tick(TickType type)
{
    for(auto pair : m_gameState->objects())
    {
        std::shared_ptr<GameObject> obj = pair.second;
        obj->nextState = obj->state;
    }

    //Reset per-tick flags
    m_shouldReverse = false;
    m_boxToEnter = -1;

    point_t mouseWorldPos = m_graphics->getMousePos();
    if(m_backwards)
    {
        if(type == REWIND)
        {
            //Rewinding backwards time
            if(m_currentTick < m_gameState->historyBuffer().breakpoint)
            {
                m_currentTick++;
                restoreState(m_currentTick);
            }
            //If going backwards there is always another timeline to pop to
            else
            {
                popTimeline();
            }
        }
        else if(type == ADVANCE)
        {
            //Normal backwards time
            if(m_currentTick > 0)
            {
                m_currentTick--;

                playTick();
            }
            else
            {
                m_statusString = "TIME'S BOUNDARY";
            }
        }
        else
        {
            //Paused
        }
    }
    else
    {
        if(type == REWIND)
        {
            //Rewind
            if(m_currentTick > m_gameState->historyBuffer().breakpoint)
            {
                m_currentTick--;
                restoreState(m_currentTick);
            }
            else if(m_gameState->timelines.size() > 1)
            {
                popTimeline();
            }
            else
            {
                m_statusString = "TIME'S BOUNDARY";
            }
        }
        else if(type == ADVANCE)
        {
            //Normal
            m_currentTick++;

            playTick();
        }
        else
        {
            //Paused
        }
    }

    if(m_controls.reverse || m_shouldReverse)
    {
        pushTimeline();
    }
    m_shouldReverse = false;

    m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());
}
