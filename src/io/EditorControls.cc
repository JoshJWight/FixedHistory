#include "EditorControls.hh"

EditorControls::EditorControls()
    : up(false), down(false), left(false), right(false), drag(false), paint(false)
{
    m_actOnPressKeys.push_back(sf::Keyboard::E);
    m_actOnPressKeys.push_back(sf::Keyboard::F);
    m_actOnPressKeys.push_back(sf::Keyboard::R);
    m_actOnPressKeys.push_back(sf::Keyboard::Q);
    m_actOnPressKeys.push_back(sf::Keyboard::S);
    m_actOnPressKeys.push_back(sf::Keyboard::Q);
    m_actOnPressKeys.push_back(sf::Keyboard::X);
    m_actOnPressKeys.push_back(sf::Keyboard::C);

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

    select = sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && !m_lastStateMap[sf::Keyboard::Q];
    connect = sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !m_lastStateMap[sf::Keyboard::E];
    remove = sf::Keyboard::isKeyPressed(sf::Keyboard::X) && !m_lastStateMap[sf::Keyboard::X];

    addRow = sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !m_lastStateMap[sf::Keyboard::R] && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    addCol = sf::Keyboard::isKeyPressed(sf::Keyboard::C) && !m_lastStateMap[sf::Keyboard::C] && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    removeRow = sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !m_lastStateMap[sf::Keyboard::R] && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    removeCol = sf::Keyboard::isKeyPressed(sf::Keyboard::C) && !m_lastStateMap[sf::Keyboard::C] && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    save = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !m_lastStateMap[sf::Keyboard::S];

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = sf::Keyboard::isKeyPressed(key);
    }
}