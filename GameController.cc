#include "GameController.hh"
#include <SFML/Window/Keyboard.hpp>

GameController::GameController()
    : m_graphics(800, 600)
    , m_currentTick(-1)//Start at -1 so that the first tick is 0
    , m_backwards(false)
    , m_lastBreakpoint(0)
    , m_lastID(0)
    , m_eUnpressed(true)
{
    std::shared_ptr<GameObject>player(new GameObject(nextID()));
    m_player = player.get();
    m_player->state.pos = point_t(0, 0);
    m_player->colliderType = CIRCLE;
    m_player->size = point_t(10, 10);
    m_player->sprite.setTexture(TextureBank::get("smiley.png"));
    m_player->moveSpeed = 1;

    std::shared_ptr<GameObject> obj(new GameObject(nextID()));
    obj->state.pos = point_t(10, 10);
    obj-> colliderType = BOX;
    obj-> size = point_t(20, 20);
    obj->sprite.setTexture(TextureBank::get("box.png"));

    std::shared_ptr<GameObject> obj2(new GameObject(nextID()));
    obj2->state.pos = point_t(-20, -30);
    obj2-> colliderType = BOX;
    obj2-> size = point_t(20, 30);
    obj2->sprite.setTexture(TextureBank::get("box.png"));


    m_objects[player->id] = player;
    m_objects[obj->id] = obj;
    m_objects[obj2->id] = obj2;

    m_historyBuffers.push_back(HistoryBuffer());

    m_historyBuffers.back().buffer[player->id] = std::vector<ObjectState>(1);
    m_historyBuffers.back().buffer[player->id][0] = player->state;
    m_historyBuffers.back().buffer[obj->id] = std::vector<ObjectState>(1);
    m_historyBuffers.back().buffer[obj->id][0] = obj->state;
    m_historyBuffers.back().buffer[obj2->id] = std::vector<ObjectState>(1);
    m_historyBuffers.back().buffer[obj2->id][0] = obj2->state;
}

void GameController::mainLoop()
{
    //~60 FPS
    int msPerFrame = 16;
    std::chrono::duration<int, std::milli> frameDuration(msPerFrame);
    while(true)
    {
        auto frameStart = std::chrono::system_clock::now();
        tick();
        m_graphics.draw(m_objects, m_currentTick);

        std::this_thread::sleep_until(frameStart + frameDuration);
    }
}

void GameController::restoreState(int tick)
{
    for(auto pair : m_objects)
    {
        std::shared_ptr<GameObject> obj = pair.second;
        obj->state = m_historyBuffers.back()[obj->id][m_currentTick];
    }
}

void GameController::popTimeline()
{
    //m_backwards now refers to the timeline we're popping to
    m_backwards = !m_backwards;

    //Delete the old player entity - it no longer exists on *any* timeline
    int oldPlayerID = m_player->id;
    m_player = m_player->ancestor;
    m_player->recorded = false;
    if(m_backwards)
    {
        m_player->beginning = 0;
    }
    else
    {
        m_player->hasEnding = false;
    }
    m_objects.erase(oldPlayerID);

    m_historyBuffers.pop_back();
    restoreState(m_currentTick);
}
    
void GameController::pushTimeline()
{
    //m_backwards now refers to the timeline we're pushing to
    m_backwards = !m_backwards;

    //Create a new player entity
    std::shared_ptr<GameObject> newPlayer(new GameObject(nextID(), m_player));
    m_player->recorded = true;
    if(m_backwards)
    {
        m_player->hasEnding = true;
        m_player->ending = m_currentTick;
        newPlayer->hasEnding = true;
        newPlayer->ending = m_currentTick;
    }
    else
    {
        m_player->beginning = m_currentTick;
        newPlayer->beginning = m_currentTick;
    }
    m_objects[newPlayer->id] = newPlayer;
    m_player = newPlayer.get();

    //Create a new history buffer copying the last one
    m_historyBuffers.push_back(HistoryBuffer(m_historyBuffers.back(), m_currentTick));

    //Add a buffer to the new history buffer for the new player
    m_historyBuffers.back().buffer[m_player->id] = std::vector<ObjectState>(m_currentTick+1);
    m_historyBuffers.back().buffer[m_player->id][m_currentTick] = m_player->state;
}

void GameController::playTick()
{
    //Player movement - maybe should go in a separate method or class
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        m_player->state.pos.y += m_player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        m_player->state.pos.y -= m_player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        m_player->state.pos.x -= m_player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        m_player->state.pos.x += m_player->moveSpeed;
    }

    //Store object states in history buffer
    
    for(auto pair : m_objects)
    {
        std::shared_ptr<GameObject> obj = pair.second;
        if(m_currentTick < obj->beginning || (obj->hasEnding && m_currentTick > obj->ending))
        {
            obj->state.active = false;
            continue;
        }
        else
        {
            obj->state.active = true;
        }

        if(obj->backwards != m_backwards || obj->recorded)
        {
            obj->state = m_historyBuffers.back()[obj->id][m_currentTick];
        }
        else if(m_currentTick >= m_historyBuffers.back()[obj->id].size())
        {  
            m_historyBuffers.back()[obj->id].push_back(obj->state);
        }
        else
        {
            m_historyBuffers.back()[obj->id][m_currentTick] = obj->state;
        }
    }
}

void GameController::tick()
{
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        //The above isn't checking for the key being pressed, it's checking for the key being held down
        //So only do this once per press
        if(m_eUnpressed)
        {
            m_eUnpressed = false;
            pushTimeline();
            return;
        }
    }
    else
    {
        m_eUnpressed = true;
    }


    point_t mouseWorldPos = m_graphics.getMousePos();
    if(m_backwards)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        {
            //Rewinding backwards time
            if(m_currentTick < m_historyBuffers.back().breakpoint)
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
        else
        {
            //Normal backwards time
            if(m_currentTick > 0)
            {
                m_currentTick--;

                playTick();
            }
        }
    }
    else
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        {
            //Rewind
            if(m_currentTick > m_historyBuffers.back().breakpoint)
            {
                m_currentTick--;
                restoreState(m_currentTick);
            }
            else if(m_historyBuffers.size() > 1)
            {
                popTimeline();
            }
            else
            {
                //Do nothing, we're at the beginning
            }
        }
        else
        {
            //Normal
            m_currentTick++;

            playTick();
        }
    }
}
