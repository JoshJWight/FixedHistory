#ifndef __GRAPHICS_HH__
#define __GRAPHICS_HH__

#include "GameObject.hh"
#include "Level.hh"
#include "TextureBank.hh"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include "GameState.hh"
#include "Search.hh"

class Graphics
{
public:
    Graphics(int windowWidth, int windowHeight);

    void draw(GameState* state, int tick, point_t cameraCenter, const std::string& statusString);

    point_t getMousePos();

private:
    //Giving the camera some radius helps if the player is near the edge of a tile
    const float CAMERA_RADIUS = 10.0f;

    const sf::Color GREYED_OUT = sf::Color(100, 100, 100, 255);
    const sf::Color NORMAL_COLOR = sf::Color(255, 255, 255, 255);

    void drawObj(GameObject* obj);
    void drawObjAs(GameObject* obj, sf::Sprite & sprite);
    void drawObjects(GameState* state, int tick, const VisibilityGrid & visibilityGrid);

    void setSpriteScale(sf::Sprite & sprite, point_t worldSize);

    point_t worldToCamera(point_t worldPoint);
    point_t cameraToWorld(sf::Vector2f cameraPoint);

    sf::RenderWindow m_window;

    point_t m_cameraWorldPos;
    //Number of pixels per world unit
    float m_cameraScale;

    point_t m_windowSize;

    sf::Sprite m_reticleSprite;

    sf::Sprite m_wallSprite;
    sf::Sprite m_floorSprite;

    sf::Sprite m_hiddenEnemySprite;
 
    sf::Text m_tickCounter;
    sf::Text m_statusText;
};

#endif
