#include "Controls.hh"

Controls::Controls()
    : up(false), down(false), left(false), right(false), fire(false), interact(false), rewind(false), reverse(false)
{
    m_actOnPressKeys.push_back(sf::Keyboard::E);
    m_actOnPressKeys.push_back(sf::Keyboard::F);

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = false;
    }
}

void Controls::tick()
{
    up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

    fire = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    interact = sf::Keyboard::isKeyPressed(sf::Keyboard::F) && !m_lastStateMap[sf::Keyboard::F];
    bool rightMouse = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    throw_ = rightMouse && !m_rightMouseLastState;
    m_rightMouseLastState = rightMouse;

    rewind = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    reverse = sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !m_lastStateMap[sf::Keyboard::E];


    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = sf::Keyboard::isKeyPressed(key);
    }
}