#include "EditorControls.hh"

EditorControls::EditorControls()
    : up(false), down(false), left(false), right(false), drag(false), paint(false)
{
    m_actOnPressKeys.push_back(sf::Keyboard::E);
    m_actOnPressKeys.push_back(sf::Keyboard::F);
    m_actOnPressKeys.push_back(sf::Keyboard::R);
    m_actOnPressKeys.push_back(sf::Keyboard::Q);

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = false;
    }
}

void EditorControls::tick()
{
    up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

    drag = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    paint = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = sf::Keyboard::isKeyPressed(key);
    }
}