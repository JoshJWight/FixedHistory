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
    void tick(short encoded);

    short encode();

    bool up;
    bool down;
    bool left;
    bool right;

    bool fire;
    bool interact;
    bool throw_; //throw is a keyword in C++

    bool rewind;
    bool reverse;

    bool restart;

    bool promiseAbsence;

private:
    std::map<sf::Keyboard::Key, bool> m_lastStateMap;
    std::vector<sf::Keyboard::Key> m_actOnPressKeys;

    bool m_rightMouseLastState;
};

#endif