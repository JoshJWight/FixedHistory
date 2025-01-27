#include "Controls.hh"

Controls::Controls()
    : up(false), down(false), left(false), right(false), fire(false), interact(false), rewind(false), reverse(false)
    , restart(false), throw_(false), promiseAbsence(false), m_rightMouseLastState(false)
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
    restart = sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !m_lastStateMap[sf::Keyboard::R];

    promiseAbsence = sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && !m_lastStateMap[sf::Keyboard::Q];

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = sf::Keyboard::isKeyPressed(key);
    }
}

void Controls::tick(short encoded)
{
    up = encoded & 1 << 0;
    down = encoded & 1 << 1;
    left = encoded & 1 << 2;
    right = encoded & 1 << 3;
    fire = encoded & 1 << 4;
    interact = (encoded & 1 << 5) && !m_lastStateMap[sf::Keyboard::F];
    m_lastStateMap[sf::Keyboard::F] = encoded & 1 << 5;
    throw_ = (encoded & 1 << 6) && !m_rightMouseLastState;
    m_rightMouseLastState = encoded & 1 << 6;
    rewind = encoded & 1 << 7;
    reverse = (encoded & 1 << 8) && !m_lastStateMap[sf::Keyboard::E];
    m_lastStateMap[sf::Keyboard::E] = encoded & 1 << 8;
    restart = (encoded & 1 << 9) && !m_lastStateMap[sf::Keyboard::R];
    m_lastStateMap[sf::Keyboard::R] = encoded & 1 << 9;
    promiseAbsence = (encoded & 1 << 10) && !m_lastStateMap[sf::Keyboard::Q];
    m_lastStateMap[sf::Keyboard::Q] = encoded & 1 << 10;
}

short Controls::encode()
{
    short encoded = 0;

    //Up
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        encoded |= 1 << 0;
    }
    //Down
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        encoded |= 1 << 1;
    }
    //Left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        encoded |= 1 << 2;
    }
    //Right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        encoded |= 1 << 3;
    }
    //Fire
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        encoded |= 1 << 4;
    }
    //Interact
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::F))
    {
        encoded |= 1 << 5;
    }
    //Throw
    if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        encoded |= 1 << 6;
    }
    //Rewind
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
    {
        encoded |= 1 << 7;
    }
    //Reverse
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        encoded |= 1 << 8;
    }
    //Restart
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
        encoded |= 1 << 9;
    }
    //Promise Absence
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        encoded |= 1 << 10;
    }

    return encoded;
}