#include "GameController.hh"

GameController::GameController(const std::string & levelPath, Graphics * graphics, DemoReader * demoReader, DemoWriter * demoWriter)
    : m_graphics(graphics)
    , m_demoReader(demoReader)
    , m_demoWriter(demoWriter)
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

        if(m_demoReader != nullptr)
        {
            DemoFrame frame;
            if(m_demoReader->getNextFrame(frame))
            {
                m_controls.tick(frame.controls);
                m_gameState->mousePos = point_t(frame.mouseX, frame.mouseY);
            }
            else
            {
                m_demoReader = nullptr;
            }
        }
        else
        {
            m_controls.tick();
            m_gameState->mousePos = m_graphics->getMousePos();
            if(m_demoWriter != nullptr)
            {
                DemoFrame frame;
                frame.controls = m_controls.encode();
                frame.mouseX = m_gameState->mousePos.x;
                frame.mouseY = m_gameState->mousePos.y;
                m_demoWriter->writeFrame(frame);
            }
        }

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
            m_gameState->statusString = "";
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
            m_gameState->statusString = "";
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
            m_graphics->draw(m_gameState.get(), cameraCenter);
            std::this_thread::sleep_until(lastDraw + frameDuration);
            lastDraw = std::chrono::system_clock::now();
        }
    }

    return true;
}

bool GameController::checkParadoxes()
{
    //Note: current tick is still the tick we just finished doing

    //Player being hit by a bullet
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_gameState->tick))
        {
            continue;
        }

        for(Bullet* bullet : m_gameState->bullets())
        {
            if(bullet->activeAt(m_gameState->tick) && player->isColliding(*bullet))
            {
                if(player->id == m_gameState->currentPlayer()->id)
                {
                    m_gameState->statusString = "YOU GOT SHOT";
                }
                else{
                    m_gameState->statusString = "A PAST YOU GOT SHOT";
                }
                return true;
            }
        }
    }
    //Player standing on spikes
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_gameState->tick))
        {
            continue;
        }

        for(Spikes* spikes : m_gameState->spikes())
        {
            if(spikes->activeAt(m_gameState->tick) && spikes->state.aiState == Spikes::UP && player->isColliding(*spikes))
            {
                if(player->id == m_gameState->currentPlayer()->id)
                {
                    m_gameState->statusString = "YOU GOT SPIKED";
                }
                else{
                    m_gameState->statusString = "A PAST YOU GOT SPIKED";
                }
                return true;
            }
        }
    }

    //Violation of observations
    for(Player* player : m_gameState->players())
    {
        if(!player->activeAt(m_gameState->tick))
        {
            continue;
        }
        if(player->recorded)
        {
            std::string result = observation::checkObservations(m_gameState.get(), player, m_gameState->tick);
            if(result != "")
            {
                m_gameState->statusString = result;
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
        if(exit->activeAt(m_gameState->tick) && player->isColliding(*exit))
        {
            m_gameState->statusString = "YOU WIN";
            return true;
        }
    }

    return false;
}

void GameController::restoreState()
{
    m_gameState->restoreState();
    m_gameState->doRewindCleanup();
}

void GameController::popTimeline()
{
    std::cout << "Popping timeline on tick " << m_gameState->tick << std::endl;

    if(m_gameState->timelines.size() == 1)
    {
        throw std::runtime_error("Cannot pop the last timeline");
    }

    //Simply blow away the current timeline, which will return us to how things were before the push
    m_gameState->timelines.pop_back();

    restoreState();
}
    
void GameController::pushTimeline()
{
    std::cout << "Pushing timeline on tick " << m_gameState->tick << std::endl;

    m_gameState->timelines.emplace_back(m_gameState->timelines.back(), m_gameState->tick, !m_gameState->backwards());

    //Create a new player entity
    Player* oldPlayer = m_gameState->currentPlayer();
    std::shared_ptr<Player> newPlayer(new Player(m_gameState->nextID(), oldPlayer));
    newPlayer->backwards = m_gameState->backwards();
    std::cout << "Creating new player with ID " << newPlayer->id << std::endl;
    oldPlayer->recorded = true;

    oldPlayer->hasFinalTimeline = true;
    oldPlayer->finalTimeline = m_gameState->currentTimeline() - 1;
    newPlayer->initialTimeline = m_gameState->currentTimeline();
    if(m_gameState->backwards())
    {
        oldPlayer->hasEnding = true;
        oldPlayer->ending = m_gameState->tick;
        newPlayer->hasEnding = true;
        newPlayer->ending = m_gameState->tick;
        newPlayer->beginning = 0;
    }
    else
    {
        oldPlayer->beginning = m_gameState->tick;
        newPlayer->beginning = m_gameState->tick;
        newPlayer->hasEnding = false;
    }
    m_gameState->objects()[newPlayer->id] = newPlayer;
    m_gameState->players().push_back(newPlayer.get());

    if(m_gameState->boxToEnter > -1)
    {
        Container* box = dynamic_cast<Container*>(m_gameState->objects().at(m_gameState->boxToEnter).get());
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
        oldHeldObject->finalTimeline = m_gameState->currentTimeline() - 1;
        newHeldObject->initialTimeline = m_gameState->currentTimeline();
        if(m_gameState->backwards())
        {
            oldHeldObject->ending = m_gameState->tick;
            oldHeldObject->hasEnding = true;
            newHeldObject->ending = m_gameState->tick;
            newHeldObject->hasEnding = true;
            newHeldObject->beginning = 0;
        }
        else
        {
            oldHeldObject->beginning = m_gameState->tick;
            newHeldObject->beginning = m_gameState->tick;
            newHeldObject->hasEnding = false;
        }
        m_gameState->objects()[newHeldObject->id] = newHeldObject;
        m_gameState->historyBuffer().buffer[newHeldObject->id] = std::vector<ObjectState>(m_gameState->tick+1);
        m_gameState->historyBuffer().buffer[newHeldObject->id][m_gameState->tick] = newHeldObject->state;
        m_gameState->throwables().push_back(newHeldObject.get());
    }

    //Add a buffer to the new history buffer for the new player
    m_gameState->historyBuffer().buffer[newPlayer->id] = std::vector<ObjectState>(m_gameState->tick+1);
    m_gameState->historyBuffer().buffer[newPlayer->id][m_gameState->tick] = newPlayer->state;

    newPlayer->observations.resize(m_gameState->tick+1);
    observation::recordObservations(m_gameState.get(), newPlayer.get(), m_gameState->tick);

    //Delete all transient objects whose origin is "after" the breakpoint
    std::vector<int> toDelete;
    for(auto pair : m_gameState->objects())
    {
        GameObject* obj = pair.second.get();
        if(m_gameState->backwards())
        {
            if(obj->isTransient() && obj->backwards && obj->hasEnding && obj->ending < m_gameState->tick)
            {
                toDelete.push_back(obj->id);
            }
        }
        else
        {
            if(obj->isTransient() && !obj->backwards && obj->beginning > m_gameState->tick)
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

void GameController::updateAlarmConnections()
{
    for(Alarm * alarm : m_gameState->alarms())
    {
        alarm->crimes.clear();
        alarm->enemies.clear();
    }
    for(Crime * crime : m_gameState->crimes())
    {
        if(!crime->activeAt(m_gameState->tick))
        {
            continue;
        }
        if(crime->state.assignedAlarm == -1)
        {
            throw std::runtime_error("Crime has no assigned alarm!");
        }

        Alarm * alarm = dynamic_cast<Alarm*>(m_gameState->objects().at(crime->state.assignedAlarm).get());
        alarm->crimes.push_back(crime->id);
    }
    for(Enemy * enemy : m_gameState->enemies())
    {
        if(!enemy->activeAt(m_gameState->tick))
        {
            continue;
        }
        if(enemy->state.assignedAlarm == -1)
        {
            continue;
        }

        Alarm * alarm = dynamic_cast<Alarm*>(m_gameState->objects().at(enemy->state.assignedAlarm).get());
        alarm->enemies.push_back(enemy->id);
    }

    for(Alarm * alarm : m_gameState->alarms())
    {
        if(alarm->crimes.size() == 0)
        {
            if(alarm->backwards)
            {
                alarm->beginning = m_gameState->tick;
            }
            else
            {
                alarm->ending = m_gameState->tick;
                alarm->hasEnding = true;
            }
        }
    }

    
}


void GameController::playTick()
{
    updateAlarmConnections();

    tick::tickPlayer(m_gameState.get(), m_gameState->currentPlayer(), &m_controls);
    for(Bullet* bullet : m_gameState->bullets())
    {
        tick::tickBullet(m_gameState.get(), bullet);
    }

    for(Enemy* enemy : m_gameState->enemies())
    {
        tick::tickEnemy(m_gameState.get(), enemy);
    }

    for(Container* container : m_gameState->containers())
    {
        tick::tickContainer(m_gameState.get(), container);
    }

    for(Switch* sw : m_gameState->switches())
    {
        tick::tickSwitch(m_gameState.get(), sw);
    }

    for(Door* door : m_gameState->doors())
    {
        tick::tickDoor(m_gameState.get(), door);
    }

    for(Spikes* spikes : m_gameState->spikes())
    {
        tick::tickSpikes(m_gameState.get(), spikes);
    }

    for(Throwable* throwable : m_gameState->throwables())
    {
        tick::tickThrowable(m_gameState.get(), throwable);
    }

    //Apply next states to current states
    for(auto pair : m_gameState->objects())
    {
        std::shared_ptr<GameObject> obj = pair.second;
        if(!obj->activeAt(m_gameState->tick))
        {
            continue;
        }

        if(!obj->recorded)
        {
            obj->applyNextState();
            if(m_gameState->tick == m_gameState->historyBuffer()[obj->id].size())
            {  
                m_gameState->historyBuffer()[obj->id].push_back(obj->state);
            }
            else if(m_gameState->tick > m_gameState->historyBuffer()[obj->id].size())
            {
                throw std::runtime_error("It is tick " + std::to_string(m_gameState->tick) + " but object " + std::to_string(obj->id) + " has history buffer size " + std::to_string(m_gameState->historyBuffer()[obj->id].size()));
            }
            else
            {
                m_gameState->historyBuffer()[obj->id][m_gameState->tick] = obj->state;
            }
        }
        else
        {
            obj->state = m_gameState->historyBuffer()[obj->id][m_gameState->tick];
        }
    }
    //Creating it both here and elsewhere because we want it to be right before recoring observations
    m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());
    observation::recordObservations(m_gameState.get(), m_gameState->currentPlayer(), m_gameState->tick);
}

void GameController::tick(TickType type)
{
    for(auto pair : m_gameState->objects())
    {
        std::shared_ptr<GameObject> obj = pair.second;
        obj->nextState = obj->state;
    }

    //Reset per-tick flags
    m_gameState->shouldReverse = false;
    m_gameState->boxToEnter = -1;

    
    if(m_gameState->backwards())
    {
        if(type == REWIND)
        {
            //Rewinding backwards time
            if(m_gameState->tick < m_gameState->historyBuffer().breakpoint)
            {
                m_gameState->tick++;
                restoreState();
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
            if(m_gameState->tick > 0)
            {
                m_gameState->tick--;

                playTick();
            }
            else
            {
                m_gameState->statusString = "TIME'S BOUNDARY";
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
            if(m_gameState->tick > m_gameState->historyBuffer().breakpoint)
            {
                m_gameState->tick--;
                restoreState();
            }
            else if(m_gameState->timelines.size() > 1)
            {
                popTimeline();
            }
            else
            {
                m_gameState->statusString = "TIME'S BOUNDARY";
            }
        }
        else if(type == ADVANCE)
        {
            //Normal
            m_gameState->tick++;

            playTick();
        }
        else
        {
            //Paused
        }
    }

    if(m_controls.reverse || m_gameState->shouldReverse)
    {
        pushTimeline();
    }
    m_gameState->shouldReverse = false;

    m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());
}
