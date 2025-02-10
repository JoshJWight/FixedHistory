#ifndef __EDITOR_HH__
#define __EDITOR_HH__

#include <state/GameState.hh>
#include <io/Graphics.hh>
#include <io/LoadLevel.hh>
#include <io/EditorControls.hh>

#include <memory>

class Editor
{
public:
    Editor(Graphics* graphics, const std::string & level);
    void mainLoop();

private:
    void handleInputs();

    std::shared_ptr<GameState> m_gameState;
    Graphics* m_graphics;
    std::string m_levelName;
    EditorControls m_controls;

    //Pointer to the one in the gameState, just to make it easier to refer to
    EditorState * m_state;

    point_t m_cameraCenter;


    bool m_hasUnsavedChanges;
};

#endif