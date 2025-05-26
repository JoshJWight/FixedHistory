#include "Controls.hh"

Controls::Controls()
    : up(false), down(false), left(false), right(false), fire(false), interact(false), rewind(false), reverse(false)
    , restart(false), throw_(false), promiseAbsence(false)
    , m_leftMouse(false), m_rightMouse(false), m_rightMouseLastState(false), m_leftMouseLastState(false)
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
    m_currentKeys[sf::Keyboard::W] = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    m_currentKeys[sf::Keyboard::S] = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    m_currentKeys[sf::Keyboard::A] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    m_currentKeys[sf::Keyboard::D] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    m_currentKeys[sf::Keyboard::F] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
    m_currentKeys[sf::Keyboard::E] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    m_currentKeys[sf::Keyboard::R] = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
    m_currentKeys[sf::Keyboard::Q] = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
    m_currentKeys[sf::Keyboard::LShift] = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    m_currentKeys[sf::Keyboard::Space] = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    m_leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    m_rightMouse = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    doTick();
}

void Controls::tick(short encoded)
{
    m_currentKeys[sf::Keyboard::W] = encoded & 1 << 0;
    m_currentKeys[sf::Keyboard::S] = encoded & 1 << 1;
    m_currentKeys[sf::Keyboard::A] = encoded & 1 << 2;
    m_currentKeys[sf::Keyboard::D] = encoded & 1 << 3;
    m_leftMouse = encoded & 1 << 4;
    m_currentKeys[sf::Keyboard::F] = encoded & 1 << 5;
    m_rightMouse = encoded & 1 << 6;
    m_currentKeys[sf::Keyboard::LShift] = encoded & 1 << 7;
    m_currentKeys[sf::Keyboard::E] = encoded & 1 << 8;
    m_currentKeys[sf::Keyboard::R] = encoded & 1 << 9;
    m_currentKeys[sf::Keyboard::Q] = encoded & 1 << 10;
    m_currentKeys[sf::Keyboard::Space] = encoded & 1 << 11;

    doTick();
}

void Controls::doTick()
{
    up = m_currentKeys[sf::Keyboard::W];
    down = m_currentKeys[sf::Keyboard::S];
    left = m_currentKeys[sf::Keyboard::A];
    right = m_currentKeys[sf::Keyboard::D];

    fire = m_leftMouse && !m_leftMouseLastState;
    m_leftMouseLastState = m_leftMouse;
    interact = m_currentKeys[sf::Keyboard::F] && !m_lastStateMap[sf::Keyboard::F];
    throw_ = m_rightMouse && !m_rightMouseLastState;
    m_rightMouseLastState = m_rightMouse;

    rewind = m_currentKeys[sf::Keyboard::LShift];
    reverse = m_currentKeys[sf::Keyboard::E] && !m_lastStateMap[sf::Keyboard::E];
    restart = m_currentKeys[sf::Keyboard::R] && !m_lastStateMap[sf::Keyboard::R];

    promiseAbsence = m_currentKeys[sf::Keyboard::Q] && !m_lastStateMap[sf::Keyboard::Q];

    slowMotion = m_currentKeys[sf::Keyboard::Space];

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = m_currentKeys[key];
    }
}

short Controls::encode()
{
    short encoded = 0;

    //Up
    if(m_currentKeys[sf::Keyboard::W])
    {
        encoded |= 1 << 0;
    }
    //Down
    if(m_currentKeys[sf::Keyboard::S])
    {
        encoded |= 1 << 1;
    }
    //Left
    if(m_currentKeys[sf::Keyboard::A])
    {
        encoded |= 1 << 2;
    }
    //Right
    if(m_currentKeys[sf::Keyboard::D])
    {
        encoded |= 1 << 3;
    }
    //Fire
    if(m_leftMouse)
    {
        encoded |= 1 << 4;
    }
    //Interact
    if(m_currentKeys[sf::Keyboard::F])
    {
        encoded |= 1 << 5;
    }
    //Throw
    if(m_rightMouse)
    {
        encoded |= 1 << 6;
    }
    //Rewind
    if(m_currentKeys[sf::Keyboard::LShift])
    {
        encoded |= 1 << 7;
    }
    //Reverse
    if(m_currentKeys[sf::Keyboard::E])
    {
        encoded |= 1 << 8;
    }
    //Restart
    if(m_currentKeys[sf::Keyboard::R])
    {
        encoded |= 1 << 9;
    }
    //Promise Absence
    if(m_currentKeys[sf::Keyboard::Q])
    {
        encoded |= 1 << 10;
    }
    //Slow Motion
    if(m_currentKeys[sf::Keyboard::Space])
    {
        encoded |= 1 << 11;
    }

    return encoded;
}