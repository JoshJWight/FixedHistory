#ifndef CONTROLS_HH
#define CONTROLS_HH

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <map>
#include <vector>

class Controls {
public:
    Controls();

    void tick();

    bool up;
    bool down;
    bool left;
    bool right;

    bool fire;
    bool interact;

    bool rewind;
    bool reverse;

private:
    std::map<sf::Keyboard::Key, bool> m_lastStateMap;
    std::vector<sf::Keyboard::Key> m_actOnPressKeys;
};

#endif