#include "GameObject.hh"
#include "Level.hh"
#include "TextureBank.hh"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>

class Graphics
{
public:
    Graphics(int windowWidth, int windowHeight);

    void draw(const Level & level, std::map<int, std::shared_ptr<GameObject>>& objects, int tick, point_t cameraCenter, const std::string& statusString);

    point_t getMousePos();

private:
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
 
    sf::Text m_tickCounter;
    sf::Text m_statusText;
};
