#include "EditorControls.hh"

EditorControls::EditorControls()
    : up(false)
    , down(false)
    , left(false)
    , right(false)
    , drag(false)
    , paint(false)
    , select(false)
    , connect(false)
    , remove(false)
    , addRow(false)
    , addCol(false)
    , removeRow(false)
    , removeCol(false)
    , placePlayer(false)
    , placeEnemy(false)
    , placeTimeBox(false)
    , placeSwitch(false)
    , placeDoor(false)
    , placeCloset(false)
    , placeTurnstile(false)
    , placeSpikes(false)
    , placeObjective(false)
    , placeKnife(false)
    , placeExit(false)
    , placeGun(false)
    , save(false)
    , toggleSnapToGrid(false)
{
    m_actOnPressKeys.push_back(sf::Keyboard::E);
    m_actOnPressKeys.push_back(sf::Keyboard::F);
    m_actOnPressKeys.push_back(sf::Keyboard::R);
    m_actOnPressKeys.push_back(sf::Keyboard::Q);
    m_actOnPressKeys.push_back(sf::Keyboard::S);
    m_actOnPressKeys.push_back(sf::Keyboard::Q);
    m_actOnPressKeys.push_back(sf::Keyboard::X);
    m_actOnPressKeys.push_back(sf::Keyboard::C);
    m_actOnPressKeys.push_back(sf::Keyboard::Num0);
    m_actOnPressKeys.push_back(sf::Keyboard::Num1);
    m_actOnPressKeys.push_back(sf::Keyboard::Num2);
    m_actOnPressKeys.push_back(sf::Keyboard::Num3);
    m_actOnPressKeys.push_back(sf::Keyboard::Num4);
    m_actOnPressKeys.push_back(sf::Keyboard::Num5);
    m_actOnPressKeys.push_back(sf::Keyboard::Num6);
    m_actOnPressKeys.push_back(sf::Keyboard::Num7);
    m_actOnPressKeys.push_back(sf::Keyboard::Num8);
    m_actOnPressKeys.push_back(sf::Keyboard::Num9);
    m_actOnPressKeys.push_back(sf::Keyboard::Dash);
    m_actOnPressKeys.push_back(sf::Keyboard::BackSlash);
    m_actOnPressKeys.push_back(sf::Keyboard::Equal);

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

    placePlayer = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) && !m_lastStateMap[sf::Keyboard::Num1];
    placeEnemy = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) && !m_lastStateMap[sf::Keyboard::Num2];
    placeTimeBox = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) && !m_lastStateMap[sf::Keyboard::Num3];
    placeSwitch = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4) && !m_lastStateMap[sf::Keyboard::Num4];
    placeDoor = sf::Keyboard::isKeyPressed(sf::Keyboard::Num5) && !m_lastStateMap[sf::Keyboard::Num5];
    placeCloset = sf::Keyboard::isKeyPressed(sf::Keyboard::Num6) && !m_lastStateMap[sf::Keyboard::Num6];
    placeTurnstile = sf::Keyboard::isKeyPressed(sf::Keyboard::Num7) && !m_lastStateMap[sf::Keyboard::Num7];
    placeSpikes = sf::Keyboard::isKeyPressed(sf::Keyboard::Num8) && !m_lastStateMap[sf::Keyboard::Num8];
    placeObjective = sf::Keyboard::isKeyPressed(sf::Keyboard::Num9) && !m_lastStateMap[sf::Keyboard::Num9];
    placeKnife = sf::Keyboard::isKeyPressed(sf::Keyboard::Num0) && !m_lastStateMap[sf::Keyboard::Num0];
    placeExit = sf::Keyboard::isKeyPressed(sf::Keyboard::Dash) && !m_lastStateMap[sf::Keyboard::Dash];
    placeGun = sf::Keyboard::isKeyPressed(sf::Keyboard::Equal) && !m_lastStateMap[sf::Keyboard::Equal];

    save = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !m_lastStateMap[sf::Keyboard::S];

    toggleSnapToGrid = sf::Keyboard::isKeyPressed(sf::Keyboard::BackSlash) && !m_lastStateMap[sf::Keyboard::BackSlash];

    for(auto key : m_actOnPressKeys)
    {
        m_lastStateMap[key] = sf::Keyboard::isKeyPressed(key);
    }
}