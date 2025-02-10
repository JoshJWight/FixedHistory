#include "Graphics.hh"

#include <iostream>
#include <algorithm>

Graphics::Graphics(int windowWidth, int windowHeight)
    :shouldDrawDebug(false),
     m_window(sf::VideoMode(windowWidth, windowHeight), "Fixed History"),
     m_windowSize(windowWidth, windowHeight),
     m_cameraScale(windowWidth / 500.0)
{
    m_reticleSprite.setTexture(TextureBank::get("reticle.png"));
    m_reticleSprite.setOrigin(sf::Vector2f(m_reticleSprite.getTexture()->getSize() / (unsigned int)2));
    float cursorScale = 0.4;
    m_reticleSprite.setScale(cursorScale, cursorScale);

    m_floorSprite.setTexture(TextureBank::get("floor.png"));
    m_wallSprite.setTexture(TextureBank::get("wall.png"));
    m_hiddenEnemySprite.setTexture(TextureBank::get("hiddenenemy.png"));

    m_window.setMouseCursorVisible(false);

    m_tickCounter.setFont(TextureBank::getFont());
    m_tickCounter.setString("0");
    m_tickCounter.setCharacterSize(24);
    m_tickCounter.setFillColor(sf::Color::White);
    m_tickCounter.setPosition(0, 0);

    m_statusText.setFont(TextureBank::getFont());
    m_statusText.setCharacterSize(40);
    m_statusText.setFillColor(sf::Color::Red);
    m_statusText.setPosition(0, 0);
    m_statusText.setString("Default text. This should not be seen.");

    m_infoText.setFont(TextureBank::getFont());
    m_infoText.setCharacterSize(20);    
    m_infoText.setFillColor(sf::Color::White);
    m_infoText.setPosition(1600, 0);
    m_infoText.setString("Default text. This should not be seen.");
}

void Graphics::setSpriteScale(sf::Sprite & sprite, point_t worldSize)
{
    const sf::Texture * texture = sprite.getTexture();
    sf::Vector2f texSize(texture->getSize());
    sf::Vector2f scale(m_cameraScale * worldSize);
    scale = math_util::elementwise_divide(scale, texSize);

    sprite.setScale(scale);
    sprite.setOrigin(texSize.x / 2.0f, texSize.y / 2.0f);
}

//TODO objects should be sorted by some kind of z-level
//That probably waits until we have a better ontology for objects
void Graphics::draw(GameState * state, point_t cameraCenter)
{
    m_cameraWorldPos = cameraCenter;

    sf::Event event;
    while(m_window.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            m_window.close();
            exit(0);
        }
    }

    m_window.clear();

    VisibilityGrid visibilityGrid = search::createVisibilityGrid(state, m_cameraWorldPos, 0, 360);

    VisibilityGrid crimeSearchGrid(state->level->width, std::vector<bool>(state->level->height, false));
    for(Crime * crime : state->crimes())
    {
        if(!crime->activeAt(state->tick))
        {
            continue;
        }
        for(int x=-Crime::SEARCH_RADIUS; x<=Crime::SEARCH_RADIUS; x++)
        {
            for(int y=-Crime::SEARCH_RADIUS; y<=Crime::SEARCH_RADIUS; y++)
            {
                if(crime->isSearched(x, y))
                {
                    continue;
                }
                point_t searchPos = crime->state.pos + (point_t(x * state->level->scale, y * state->level->scale));
                point_t levelCoords = state->level->toLevelCoords(searchPos);
                if(levelCoords.x < 0 || levelCoords.x >= state->level->width || levelCoords.y < 0 || levelCoords.y >= state->level->height)
                {
                    continue;
                }
                crimeSearchGrid[levelCoords.x][levelCoords.y] = true;
            }
        }
    }

    //Draw each tile of the level
    for(int x = 0; x < state->level->width; ++x)
    {
        for(int y = 0; y < state->level->height; ++y)
        {
            point_t worldPos = state->level->tiles[x][y].node.pos;
            
            if(state->level->tiles[x][y].type == Level::WALL)
            {
                if(crimeSearchGrid[x][y])
                {
                    m_wallSprite.setColor(RED_TINT);
                }
                else if(visibilityGrid[x][y])
                {
                    m_wallSprite.setColor(NORMAL_COLOR);
                }
                else
                {
                    m_wallSprite.setColor(GREYED_OUT);
                }

                m_wallSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_wallSprite, point_t(1, 1) * state->level->scale);
                m_window.draw(m_wallSprite);
            }
            else
            {
                if(crimeSearchGrid[x][y])
                {
                    m_floorSprite.setColor(RED_TINT);
                }
                else if(visibilityGrid[x][y])
                {
                    m_floorSprite.setColor(NORMAL_COLOR);
                }
                else
                {
                    m_floorSprite.setColor(GREYED_OUT);
                }


                m_floorSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_floorSprite, point_t(1, 1) * state->level->scale);
                m_window.draw(m_floorSprite);
            }
        }
    }

    drawObjects(state, visibilityGrid);

    if(shouldDrawDebug)
    {
        drawDebug(state);
    }

    m_reticleSprite.setPosition(worldToCamera(state->mousePos));
    m_window.draw(m_reticleSprite);

    m_tickCounter.setString(std::to_string(state->tick));
    m_window.draw(m_tickCounter);

    if(state->statusString!="")
    {
        m_statusText.setString(state->statusString);
        int charSize = std::max(10.0, 200 / std::sqrt((double)state->statusString.size()));
        m_statusText.setCharacterSize(charSize);
        m_window.draw(m_statusText);
    }
    if(state->infoString!="")
    {
        m_infoText.setString(state->infoString);
        m_infoText.setCharacterSize(20);
        m_window.draw(m_infoText);
    }

    m_window.display();
}

void Graphics::drawObj(GameObject* obj)
{
    sf::Sprite & sprite = obj->getSprite();

    setSpriteScale(sprite, obj->size);

    point_t cameraPos = worldToCamera(obj->state.pos);
    sprite.setPosition(sf::Vector2f(cameraPos));
    sprite.setRotation(obj->state.angle_deg * -1.0f);
    m_window.draw(sprite);
}

void Graphics::drawObjAs(GameObject* obj, sf::Sprite & sprite)
{
    setSpriteScale(sprite, obj->size);

    point_t cameraPos = worldToCamera(obj->state.pos);
    sprite.setPosition(sf::Vector2f(cameraPos));
    sprite.setRotation(obj->state.angle_deg * -1.0f);
    m_window.draw(sprite);
}

void Graphics::drawObjects(GameState* state, const VisibilityGrid & visibilityGrid)
{
    auto compare = [](GameObject* a, GameObject* b)
    {
        return a->drawPriority() > b->drawPriority();
    };
    std::priority_queue<GameObject*, std::vector<GameObject*>, decltype(compare)> drawQueue(compare);
    for(auto it = state->objects().begin(); it != state->objects().end(); ++it)
    {
        GameObject * obj = it->second.get();
        if(obj->activeAt(state->tick))
        {
            drawQueue.push(obj);
        }
    }

    std::map<int, GameObject*> toDraw;

    //Find objects directly visible to the camera
    for(auto it = state->objects().begin(); it != state->objects().end(); ++it)
    {
        std::shared_ptr<GameObject> obj = it->second;
        if(!obj->activeAt(state->tick))
        {
            continue;
        }
        if(obj->isDebugGraphic())
        {
            if(shouldDrawDebug)
            {
                toDraw[obj->id] = obj.get();
            }
            continue;
        }

        if(!obj->state.visible)
        {
            continue;
        }
        point_t levelCoords = state->level->toLevelCoords(obj->state.pos);
        if(!visibilityGrid[levelCoords.x][levelCoords.y])
        {
            continue;
        }
        toDraw[obj->id] = obj.get();
    }

    //Find objects observed by recorded players
    for(auto it = state->players().begin(); it != state->players().end(); ++it)
    {
        Player * player = *it;
        if(!player->recorded)
        {
            continue;
        }
        if(!player->activeAt(state->tick))
        {
            continue;
        }

        if(player->state.visible)
        {
            toDraw[player->id] = player;
        }

        for(auto & obj : player->observations[state->tick])
        {
            //The object observed this timeline didn't match the originally observed object's ID
            //Mostly happens to bullets that get removed and recreated
            if(state->objects().find(obj.id) == state->objects().end())
            {
                continue;
            }

            if(toDraw.find(obj.id) != toDraw.end())
            {
                continue;
            }
            GameObject * objPtr = state->objects().at(obj.id).get();
            toDraw[obj.id] = objPtr;
        }
    }

    while(!drawQueue.empty())
    {
        GameObject * obj = drawQueue.top();
        drawQueue.pop();
        if(toDraw.find(obj->id) != toDraw.end())
        {
            if(obj == state->editorState.selectedObject)
            {
                obj->getSprite().setColor(ORANGE_TINT);
            }
            else
            {
                obj->getSprite().setColor(NORMAL_COLOR);
            }

            drawObj(obj);
        }
        else if(obj->type() == GameObject::ENEMY)
        {
            drawObjAs(obj, m_hiddenEnemySprite);
        }
    }
}

void Graphics::drawDebug(GameState * state)
{
    //Draw patrol paths
    for(Enemy* enemy: state->enemies())
    {
        if(enemy->patrolPoints.size() > 1)
        {
            for(int i=0; i<enemy->patrolPoints.size(); i++)
            {
                size_t nextIndex = (i+1) % enemy->patrolPoints.size();
                point_t start = worldToCamera(enemy->patrolPoints[i]);
                point_t end = worldToCamera(enemy->patrolPoints[nextIndex]);
                sf::Vertex line[] =
                {
                    sf::Vertex(sf::Vector2f(start.x, start.y)),
                    sf::Vertex(sf::Vector2f(end.x, end.y))
                };
                if(enemy == state->editorState.selectedObject)
                {
                    line[0].color = sf::Color::Magenta;
                    line[1].color = sf::Color::Magenta;
                }
                else
                {
                    line[0].color = sf::Color::Blue;
                    line[1].color = sf::Color::Blue;
                }
                m_window.draw(line, 2, sf::Lines);
            }
        }
    }

    //Draw door-switch connections
    for(Door * door: state->doors())
    {
        for(int sw : door->getConnectedSwitches())
        {
            if(state->objects().find(sw) == state->objects().end())
            {
                continue;
            }
            Switch * swObj = static_cast<Switch*>(state->objects().at(sw).get());
            point_t doorPos = worldToCamera(door->state.pos);
            point_t swPos = worldToCamera(swObj->state.pos);
            sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(doorPos.x, doorPos.y)),
                sf::Vertex(sf::Vector2f(swPos.x, swPos.y))
            };
            line[0].color = sf::Color::Green;
            line[1].color = sf::Color::Green;
            m_window.draw(line, 2, sf::Lines);
        }
    }

    //Draw selection area for painting
    if(state->editorState.isPainting)
    {
        point_t start = worldToCamera(state->editorState.paintOrigin);
        point_t end = worldToCamera(state->mousePos);
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(start.x, start.y)),
            sf::Vertex(sf::Vector2f(end.x, start.y)),
            sf::Vertex(sf::Vector2f(end.x, start.y)),
            sf::Vertex(sf::Vector2f(end.x, end.y)),
            sf::Vertex(sf::Vector2f(end.x, end.y)),
            sf::Vertex(sf::Vector2f(start.x, end.y)),
            sf::Vertex(sf::Vector2f(start.x, end.y)),
            sf::Vertex(sf::Vector2f(start.x, start.y))
        };
        for(int i=0; i<8; i++)
        {
            line[i].color = sf::Color::Yellow;
        }
        m_window.draw(line, 8, sf::Lines);
    }
}

point_t Graphics::getMousePos()
{
    sf::Vector2f mouseCameraPos(sf::Mouse::getPosition(m_window));

    return cameraToWorld(mouseCameraPos);
}

point_t Graphics::worldToCamera(point_t worldPoint)
{
    point_t worldDiff = worldPoint - m_cameraWorldPos;
    point_t cameraDiff = worldDiff * m_cameraScale;
    //In SFML the +y axis is downwards on the screen, but my world space
    //has +y as the up direction, because I find it easier to think in
    cameraDiff.y *= -1.0f;
    return cameraDiff + (m_windowSize / 2.0f);
}


point_t Graphics::cameraToWorld(sf::Vector2f cameraPoint)
{
    point_t point(cameraPoint);
    point -= m_windowSize / 2.0f;
    point.y *= -1.0f;
    point /= m_cameraScale;
    point += m_cameraWorldPos;
    return point;
}
