#include "Graphics.hh"

#include <iostream>
#include <algorithm>

Graphics::Graphics(int windowWidth, int windowHeight)
    :m_window(sf::VideoMode(windowWidth, windowHeight), "Fixed History"),
     m_windowSize(windowWidth, windowHeight),
     m_cameraScale(windowWidth / 200.0)
{
    m_reticleSprite.setTexture(TextureBank::get("reticle.png"));
    m_reticleSprite.setOrigin(sf::Vector2f(m_reticleSprite.getTexture()->getSize() / (unsigned int)2));
    float cursorScale = 0.4;
    m_reticleSprite.setScale(cursorScale, cursorScale);

    m_floorSprite.setTexture(TextureBank::get("floor.png"));
    m_wallSprite.setTexture(TextureBank::get("wall.png"));

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
void Graphics::draw(GameState * state, int tick, point_t cameraCenter, const std::string& statusString)
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

    //Draw each tile of the level
    for(int x = 0; x < state->level->width; ++x)
    {
        for(int y = 0; y < state->level->height; ++y)
        {
            point_t worldPos = state->level->tiles[x][y].node.pos;
            
            if(!search::checkVisibility(state, m_cameraWorldPos, CAMERA_RADIUS, worldPos, state->level->scale * 0.499f))
            {
                continue;
            }
            if(state->level->tiles[x][y].type == Level::WALL)
            {
                m_wallSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_wallSprite, point_t(1, 1) * state->level->scale);
                m_window.draw(m_wallSprite);
            }
            else
            {
                m_floorSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_floorSprite, point_t(1, 1) * state->level->scale);
                m_window.draw(m_floorSprite);
            }
        }
    }

    drawObjects(state, tick);

    m_window.draw(m_reticleSprite);

    m_tickCounter.setString(std::to_string(tick));
    m_window.draw(m_tickCounter);

    if(statusString!="")
    {
        m_statusText.setString(statusString);
        int charSize = std::max(10.0, 200 / std::sqrt(statusString.size()));
        m_statusText.setCharacterSize(charSize);
        m_window.draw(m_statusText);
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
void Graphics::drawObjects(GameState* state, int tick)
{
    auto compare = [](GameObject* a, GameObject* b)
    {
        return a->drawPriority() > b->drawPriority();
    };
    std::priority_queue<GameObject*, std::vector<GameObject*>, decltype(compare)> drawQueue(compare);
    std::map<int, GameObject*> toDraw;

    //Find objects directly visible to the camera
    for(auto it = state->objects.begin(); it != state->objects.end(); ++it)
    {
        std::shared_ptr<GameObject> obj = it->second;
        if(!obj->activeAt(tick))
        {
            continue;
        }
        if(!obj->state.visible)
        {
            continue;
        }
        if(!search::checkVisibility(state, m_cameraWorldPos, CAMERA_RADIUS, obj->state.pos, obj->size.x / 2.0f))
        {
            continue;
        }
        toDraw[obj->id] = obj.get();
        drawQueue.push(obj.get());
    }

    //Find objects observed by recorded players
    for(auto it = state->players.begin(); it != state->players.end(); ++it)
    {
        Player * player = *it;
        if(!player->recorded)
        {
            continue;
        }
        if(!player->activeAt(tick))
        {
            continue;
        }

        if(player->state.visible)
        {
            toDraw[player->id] = player;
            drawQueue.push(player);
        }

        for(auto & obj : player->observations[tick])
        {
            if(toDraw.find(obj.id) != toDraw.end())
            {
                continue;
            }
            GameObject * objPtr = state->objects[obj.id].get();
            toDraw[obj.id] = objPtr;
            drawQueue.push(objPtr);
        }
    }

    while(!drawQueue.empty())
    {
        GameObject * obj = drawQueue.top();
        drawQueue.pop();
        drawObj(obj);
    }
}

point_t Graphics::getMousePos()
{
    sf::Vector2f mouseCameraPos(sf::Mouse::getPosition(m_window));
    //TODO move this
    m_reticleSprite.setPosition(mouseCameraPos);

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
