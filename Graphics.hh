#include "GameObject.hh"
#include "TextureBank.hh"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Graphics
{
public:
    Graphics(int windowWidth, int windowHeight);

    void draw(std::vector<std::shared_ptr<GameObject>>& objects);

    point_t getMousePos();

private:
    point_t worldToCamera(point_t worldPoint);
    point_t cameraToWorld(sf::Vector2f cameraPoint);

    sf::RenderWindow m_window;

    point_t m_cameraWorldPos;
    //Number of pixels per world unit
    double m_cameraScale;

    point_t m_windowSize;

    sf::Sprite m_reticleSprite;


};
