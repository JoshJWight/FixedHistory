#include "Graphics.hh"

#include <iostream>

Graphics::Graphics(int windowWidth, int windowHeight)
    :m_window(sf::VideoMode(windowWidth, windowHeight), "NemesisHeist"),
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
void Graphics::draw(const Level & level, std::map<int, std::shared_ptr<GameObject>>& objects, int tick, point_t cameraCenter)
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

    //Draw the level
    
    for(int x = 0; x < level.width; ++x)
    {
        for(int y = 0; y < level.height; ++y)
        {
            point_t worldPos = point_t(x * level.scale, y * level.scale) + level.bottomLeft;
            if(level.tiles[x][y] == Level::WALL)
            {
                m_wallSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_wallSprite, point_t(1, 1) * level.scale);
                m_window.draw(m_wallSprite);
            }
            else
            {
                m_floorSprite.setPosition(worldToCamera(worldPos));
                setSpriteScale(m_floorSprite, point_t(1, 1) * level.scale);
                m_window.draw(m_floorSprite);
            }
        }
    }

    //Draw other stuff
    for(auto it = objects.begin(); it != objects.end(); ++it)
    {
        std::shared_ptr<GameObject> obj = it->second;
        if(!it->second->activeAt(tick))
        {
            continue;
        }

        setSpriteScale(obj->sprite, obj->size);

        point_t cameraPos = worldToCamera(obj->state.pos);
        obj->sprite.setPosition(sf::Vector2f(cameraPos));
        m_window.draw(obj->sprite);
    }

    m_window.draw(m_reticleSprite);

    //Draw the tick number in the top left corner
    sf::Text text;
    text.setFont(TextureBank::getFont());
    text.setString(std::to_string(tick));
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(0, 0);
    m_window.draw(text);

    m_window.display();
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
