#include "GameController.hh"
#include <SFML/Window/Keyboard.hpp>

GameController::GameController()
    : m_graphics(800, 600)
    , m_currentTick(-1)//Start at -1 so that the first tick is 0
    , m_currentTimeline(0)
    , m_backwards(false)
    , m_lastBreakpoint(0)
    , m_lastID(0)
    , m_eUnpressed(true)
{
    std::shared_ptr<Player>player(new Player(nextID()));
    player->state.pos = point_t(0, 0);
    player->colliderType = CIRCLE;
    player->size = point_t(10, 10);
    player->sprite.setTexture(TextureBank::get("smiley.png"));
    m_players.push_back(player.get());

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

void GameController::checkBulletUndo()
{
    if(m_bullets.empty())
    {
        return;
    }

    bool undo = m_bullets.back()->originTimeline == m_currentTimeline;
    if(m_backwards)
    {
        undo &= m_bullets.back()->ending < m_currentTick;
    }
    else
    {
        undo &= m_bullets.back()->beginning > m_currentTick;
    }

    if(undo)
    {
        m_objects.erase(m_bullets.back()->id);
        m_bullets.pop_back();
    }
}

void GameController::restoreState(int tick)
{
    checkBulletUndo();
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
    m_currentTimeline--;

    //Delete the old player entity - it no longer exists on *any* timeline
    m_objects.erase(m_players.back()->id);
    m_players.pop_back();

    m_players.back()->recorded = false;
    if(m_backwards)
    {
        m_players.back()->beginning = 0;
    }
    else
    {
        m_players.back()->hasEnding = false;
    }

    m_historyBuffers.pop_back();
    restoreState(m_currentTick);
}
    
void GameController::pushTimeline()
{
    //m_backwards now refers to the timeline we're pushing to
    m_backwards = !m_backwards;
    m_currentTimeline++;

    //Create a new player entity
    std::shared_ptr<Player> newPlayer(new Player(nextID(), m_players.back()));
    m_players.back()->recorded = true;
    if(m_backwards)
    {
        m_players.back()->hasEnding = true;
        m_players.back()->ending = m_currentTick;
        newPlayer->hasEnding = true;
        newPlayer->ending = m_currentTick;
    }
    else
    {
        m_players.back()->beginning = m_currentTick;
        newPlayer->beginning = m_currentTick;
    }
    m_objects[newPlayer->id] = newPlayer;
    m_players.push_back(newPlayer.get());

    //Create a new history buffer copying the last one
    m_historyBuffers.push_back(HistoryBuffer(m_historyBuffers.back(), m_currentTick));

    //Add a buffer to the new history buffer for the new player
    m_historyBuffers.back().buffer[newPlayer->id] = std::vector<ObjectState>(m_currentTick+1);
    m_historyBuffers.back().buffer[newPlayer->id][m_currentTick] = newPlayer->state;
}

void GameController::tickPlayer(Player* player)
{
    if(player->state.cooldown > 0)
    {
        player->state.cooldown--;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        player->state.pos.y += player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        player->state.pos.y -= player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        player->state.pos.x -= player->moveSpeed;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        player->state.pos.x += player->moveSpeed;
    }

    if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && player->state.cooldown == 0)
    {
        point_t mouseWorldPos = m_graphics.getMousePos();
        point_t direction = math_util::normalize(mouseWorldPos - player->state.pos);
        point_t bulletPos = player->state.pos + direction * player->size.x;
        std::shared_ptr<Bullet> bullet(new Bullet(nextID()));
        bullet->state.pos = bulletPos;
        bullet->colliderType = CIRCLE;
        bullet->size = point_t(5, 5);
        bullet->sprite.setTexture(TextureBank::get("blam.png"));
        bullet->velocity = direction * Bullet::SPEED;

        bullet->originTimeline = m_currentTimeline;
        bullet->backwards = player->backwards;
        bullet->hasEnding = true;
        if(player->backwards)
        {
            bullet->ending = m_currentTick;
            bullet->beginning = m_currentTick - Bullet::LIFETIME;
        }
        else
        {
            bullet->beginning = m_currentTick;
            bullet->ending = m_currentTick + Bullet::LIFETIME;
        }

        player->state.cooldown = player->fireCooldown;

        m_bullets.push_back(bullet.get());
        m_objects[bullet->id] = bullet;
        m_historyBuffers.back().buffer[bullet->id] = std::vector<ObjectState>(m_currentTick+1);
        m_historyBuffers.back().buffer[bullet->id][m_currentTick] = bullet->state;
    }
}

void GameController::tickBullet(Bullet* bullet)
{
    if(!bullet->state.active || bullet->backwards != m_backwards)
    {
        return;
    }
    bullet->state.pos += bullet->velocity;
}

void GameController::playTick()
{
    tickPlayer(m_players.back());
    for(Bullet* bullet : m_bullets)
    {
        tickBullet(bullet);
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
