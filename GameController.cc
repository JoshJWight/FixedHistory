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

    std::shared_ptr<Enemy> enemy(new Enemy(nextID()));
    enemy->state.pos = point_t(50, 50);
    enemy->patrolPoints.push_back(point_t(-175, -175));
    enemy->patrolPoints.push_back(point_t(175, -175));
    enemy->patrolPoints.push_back(point_t(-175, 175));
    enemy->patrolPoints.push_back(point_t(175, 175));
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
        m_statusString = "";
        bool rewind = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        TickType type = rewind ? REWIND : ADVANCE;
        if(type == ADVANCE && checkParadoxes())
        {
            type = PAUSE;
        }
        tick(type);
        point_t cameraCenter = m_players.back()->state.pos;
        m_graphics.draw(m_level, m_objects, m_currentTick, cameraCenter, m_statusString);

        std::this_thread::sleep_until(frameStart + frameDuration);
    }
}

bool GameController::checkParadoxes()
{
    //Death of any player
    for(Player* player : m_players)
    {
        if(!player->activeAt(m_currentTick))
        {
            continue;
        }

        for(Bullet* bullet : m_bullets)
        {
            if(bullet->activeAt(m_currentTick) && player->isColliding(*bullet))
            {
                if(player->id == m_players.back()->id)
                {
                    m_statusString = "YOU DIED";
                }
                else{
                    m_statusString = "A PAST YOU DIED";
                }
                return true;
            }
        }
    }

    //If we got here, no paradoxes
    return false;
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

    point_t mouseWorldPos = m_graphics.getMousePos();
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && player->state.cooldown == 0)
    {
        point_t direction = math_util::normalize(mouseWorldPos - player->state.pos);
        point_t bulletPos = player->state.pos + direction * player->size.x;
        std::shared_ptr<Bullet> bullet(new Bullet(nextID()));
        bullet->state.pos = bulletPos;
        bullet->velocity = direction * Bullet::SPEED;
        bullet->state.angle_deg = math_util::angleBetween(player->state.pos, mouseWorldPos);
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

    player->state.angle_deg = math_util::rotateTowardsPoint(player->state.angle_deg, player->state.pos, mouseWorldPos, 5.0f);
}

void GameController::tickBullet(Bullet* bullet)
{
    if(!bullet->activeAt(m_currentTick) || bullet->backwards != m_backwards)
    {
        return;
    }
    bullet->state.pos += bullet->velocity;

    if(m_level.tileAt(bullet->state.pos) == Level::WALL)
    {
        if(bullet->backwards)
        {
            bullet->beginning = m_currentTick;
        }
        else
        {
            bullet->ending = m_currentTick;
        }
    }
}

bool GameController::playerVisibleToEnemy(Player* player, Enemy* enemy)
{
    bool visible = true;

    float angleToPlayer = math_util::angleBetween(enemy->state.pos, player->state.pos);
    float angleDiff = math_util::angleDiff(enemy->state.angle_deg, angleToPlayer);

    //Within view angle
    visible &= (std::abs(angleDiff) < (Enemy::VIEW_ANGLE / 2.0f));
    //Close enough to see
    visible &= (math_util::dist(enemy->state.pos, player->state.pos) < Enemy::VIEW_RADIUS);
    //Not obstructed
    visible &= m_level.checkVisibility(enemy->state.pos, player->state.pos);

    return visible;
}

void GameController::navigateEnemy(Enemy* enemy, point_t target)
{
    point_t moveToward = m_level.navigate(enemy->state.pos, target);
    enemy->state.pos += math_util::normalize(moveToward - enemy->state.pos) * enemy->moveSpeed;
    enemy->state.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, moveToward, 5.0f);
}

void GameController::tickEnemy(Enemy* enemy)
{
    if(!enemy->activeAt(m_currentTick) || enemy->backwards != m_backwards)
    {
        return;
    }

    enemy->state.animIdx = enemy->state.aiState;

    for(Bullet* bullet: m_bullets)
    {
        if(!bullet->activeAt(m_currentTick))
        {
            continue;
        }

        if(enemy->state.aiState != Enemy::AI_DEAD && enemy->isColliding(*bullet))
        {
            enemy->state.aiState = Enemy::AI_DEAD;
        }
    }

    if(enemy->state.aiState == Enemy::AI_PATROL)
    {
        if(math_util::dist(enemy->state.pos, enemy->patrolPoints[enemy->state.patrolIdx]) <= enemy->moveSpeed)
        {
            enemy->state.patrolIdx = (enemy->state.patrolIdx + 1) % enemy->patrolPoints.size();
        }

        navigateEnemy(enemy, enemy->patrolPoints[enemy->state.patrolIdx]);

        //Check if a player is seen
        for(Player* player : m_players)
        {
            if(playerVisibleToEnemy(player, enemy))
            {
                enemy->state.aiState = Enemy::AI_CHASE;
                enemy->state.targetId = player->id;
                enemy->state.lastSeen = player->state.pos;
                break;
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_CHASE)
    {
        Player* target = dynamic_cast<Player*>(m_objects[enemy->state.targetId].get());
        if(!target->activeAt(m_currentTick))
        {
            enemy->state.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(target, enemy))
        {
            enemy->state.lastSeen = target->state.pos;
            if(math_util::dist(enemy->state.pos, target->state.pos) < Enemy::ATTACK_RADIUS)
            {
                enemy->state.aiState = Enemy::AI_ATTACK;
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
                enemy->state.aiState = Enemy::AI_PATROL;
            }
            else
            {
                navigateEnemy(enemy, enemy->state.lastSeen);
            }
        }
    }
    else if(enemy->state.aiState == Enemy::AI_ATTACK)
    {
        Player* target = dynamic_cast<Player*>(m_objects[enemy->state.targetId].get());
        if(!target->activeAt(m_currentTick))
        {
            enemy->state.aiState = Enemy::AI_PATROL;
            return;
        }

        if(playerVisibleToEnemy(target, enemy))
        {
            enemy->state.lastSeen = target->state.pos;
            enemy->state.angle_deg = math_util::rotateTowardsPoint(enemy->state.angle_deg, enemy->state.pos, target->state.pos, 5.0f);

            if(enemy->state.chargeTime >= Enemy::ATTACK_CHARGE_TIME)
            {
                point_t direction = math_util::normalize(target->state.pos - enemy->state.pos);
                point_t bulletPos = enemy->state.pos + direction * enemy->size.x;
                std::shared_ptr<Bullet> bullet(new Bullet(nextID()));
                bullet->state.pos = bulletPos;
                bullet->velocity = direction * Bullet::SPEED;
                bullet->state.angle_deg = math_util::angleBetween(enemy->state.pos, target->state.pos);
                bullet->originTimeline = m_currentTimeline;
                bullet->backwards = enemy->backwards;
                bullet->hasEnding = true;
                if(enemy->backwards)
                {
                    bullet->ending = m_currentTick;
                    bullet->beginning = m_currentTick - Bullet::LIFETIME;
                }
                else
                {
                    bullet->beginning = m_currentTick;
                    bullet->ending = m_currentTick + Bullet::LIFETIME;
                }

                m_bullets.push_back(bullet.get());
                m_objects[bullet->id] = bullet;
                m_historyBuffers.back().buffer[bullet->id] = std::vector<ObjectState>(m_currentTick+1);
                m_historyBuffers.back().buffer[bullet->id][m_currentTick] = bullet->state;

                enemy->state.chargeTime = 0;
            }
            //Continue an attack in progress as long as we can see the target
            else if(enemy->state.chargeTime > 0)
            {
                enemy->state.chargeTime++;
            }
            //But if not shooting, close distance first if needed
            else if(math_util::dist(enemy->state.pos, target->state.pos) > Enemy::ATTACK_RADIUS)
            {
                enemy->state.aiState = Enemy::AI_CHASE;
            }
            //If target is close enough, start charging
            else
            {
                enemy->state.chargeTime++;
            }
        }
        else
        {
            enemy->state.chargeTime = 0;
            enemy->state.aiState = Enemy::AI_CHASE;
            //If there is another visible target, swap to that.
            for(Player* player : m_players)
            {
                if(playerVisibleToEnemy(player, enemy))
                {
                    
                    enemy->state.targetId = player->id;
                    enemy->state.lastSeen = player->state.pos;
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
        throw std::runtime_error("Unknown AI state");
    }
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

void GameController::tick(TickType type)
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
        if(type == REWIND)
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
}
