#ifndef EDITORCONTROLS_HH
#define EDITORCONTROLS_HH

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <map>
#include <vector>

class EditorControls {
public:
    EditorControls();

    void tick();

    bool up;
    bool down;
    bool left;
    bool right;

    bool drag;
    bool paint;

private:
    std::map<sf::Keyboard::Key, bool> m_lastStateMap;
    std::vector<sf::Keyboard::Key> m_actOnPressKeys;
};

#endif