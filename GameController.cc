#include "GameController.hh"
#include <SFML/Window/Keyboard.hpp>

#include <fstream>

GameController::GameController()
    : m_graphics(800, 600)
    , m_currentTick(-1)//Start at -1 so that the first tick is 0
    , m_currentTimeline(0)
    , m_backwards(false)
    , m_lastBreakpoint(0)
    , m_lastID(0)
    , m_eUnpressed(true)
    , m_level(20, 20, point_t(-200, -200), 20.0f)
{
    std::ifstream file("testlevel.txt");
    std::stringstream buffer;
    buffer << file.rdbuf();
    m_level.setFromString(buffer.str());
    file.close();

    m_historyBuffers.push_back(HistoryBuffer());

    std::shared_ptr<Player>player(new Player(nextID()));
    player->state.pos = point_t(0, 0);
    m_players.push_back(player.get());
    addObject(player);
/*
    std::shared_ptr<GameObject> obj(new GameObject(nextID()));
    obj->state.pos = point_t(10, 10);
    obj-> colliderType = BOX;
    obj-> size = point_t(20, 20);
    obj->sprite.setTexture(TextureBank::get("box.png"));
    addObject(obj);

    std::shared_ptr<GameObject> obj2(new GameObject(nextID()));
    obj2->state.pos = point_t(-20, -30);
    obj2-> colliderType = BOX;
    obj2-> size = point_t(20, 30);
    obj2->sprite.setTexture(TextureBank::get("box.png"));
    addObject(obj2);
*/
    std::shared_ptr<Enemy> enemy(new Enemy(nextID()));
    enemy->state.pos = point_t(50, 50);
    enemy->patrolPoints.push_back(point_t(50, 50));
    enemy->patrolPoints.push_back(point_t(50, 100));
    enemy->patrolPoints.push_back(point_t(100, 100));
    enemy->patrolPoints.push_back(point_t(100, 50));
    m_enemies.push_back(enemy.get());
    addObject(enemy);
}

//Only for initial setup
void GameController::addObject(std::shared_ptr<GameObject> obj)
{
    m_objects[obj->id] = obj;
    m_historyBuffers.back().buffer[obj->id] = std::vector<ObjectState>(1);
    m_historyBuffers.back().buffer[obj->id][0] = obj->state;
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
        point_t cameraCenter = m_players.back()->state.pos;
        m_graphics.draw(m_level, m_objects, m_currentTick, cameraCenter);

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
    if(!bullet->activeAt(m_currentTick) || bullet->backwards != m_backwards)
    {
        return;
    }
    bullet->state.pos += bullet->velocity;
}

void GameController::tickEnemy(Enemy* enemy)
{
    if(!enemy->activeAt(m_currentTick) || enemy->backwards != m_backwards)
    {
        return;
    }

    point_t patrolOffset = enemy->patrolPoints[enemy->state.patrolIdx] - enemy->state.pos;
    if(math_util::length(patrolOffset) <= enemy->moveSpeed)
    {
        enemy->state.patrolIdx = (enemy->state.patrolIdx + 1) % enemy->patrolPoints.size();
        patrolOffset = enemy->patrolPoints[enemy->state.patrolIdx] - enemy->state.pos;
    }
    enemy->state.pos += math_util::normalize(patrolOffset) * enemy->moveSpeed;
}

void GameController::playTick()
{
    tickPlayer(m_players.back());
    for(Bullet* bullet : m_bullets)
    {
        tickBullet(bullet);
    }

    for(Enemy* enemy : m_enemies)
    {
        tickEnemy(enemy);
    }

    //Store object states in history buffer
    for(auto pair : m_objects)
    {
        std::shared_ptr<GameObject> obj = pair.second;
        if(m_currentTick < obj->beginning || (obj->hasEnding && m_currentTick > obj->ending))
        {
            continue;
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
