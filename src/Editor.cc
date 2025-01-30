#include "Editor.hh"
#include <thread>

Editor::Editor(Graphics* graphics, const std::string & level)
    : m_graphics(graphics)
    , m_levelName(level)
    , m_gameState(new GameState())
    , m_controls()
    , m_isPainting(false)
    , m_paintType(Level::EMPTY)
    , m_isDragging(false)
    , m_draggedObject(nullptr)
{
    if(levelExists(level))
    {
        loadLevel(m_gameState.get(), level);
    }
    else
    {
        m_gameState->level = std::make_shared<Level>(5, 5, point_t(0, 0), 20.0f);
    }
    m_gameState->tick = 0;

    float startCamera = m_gameState->level->scale * 0.5 * std::min(m_gameState->level->width, m_gameState->level->height);

    m_cameraCenter = point_t(startCamera, startCamera);
}
    
void Editor::mainLoop()
{
    while(true)
    {
        handleInputs();
        m_gameState->obstructionGrid = search::createObstructionGrid(m_gameState.get());

        if(m_hasUnsavedChanges)
        {
            m_gameState->statusString = "Unsaved changes";
        }
        else
        {
            m_gameState->statusString = "";
        }

        m_graphics->draw(m_gameState.get(), m_cameraCenter);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Editor::handleInputs()
{
    m_controls.tick();
    m_gameState->mousePos = m_graphics->getMousePos();

    const float CAMERA_SPEED = 2.0f;

    if(m_controls.up)
    {
        m_cameraCenter.y += CAMERA_SPEED;
    }
    if(m_controls.down)
    {
        m_cameraCenter.y -= CAMERA_SPEED;
    }
    if(m_controls.left)
    {
        m_cameraCenter.x -= CAMERA_SPEED;
    }
    if(m_controls.right)
    {
        m_cameraCenter.x += CAMERA_SPEED;
    }


    if(m_isDragging)
    {
        if(m_controls.drag)
        {
            m_draggedObject->state.pos = m_gameState->mousePos;
        }
        else
        {
            m_isDragging = false;
            m_draggedObject = nullptr;
            m_hasUnsavedChanges = true;
        }
    }
    else if(m_controls.drag)
    {
        //TODO revise this
        for(auto pair: m_gameState->objects())
        {
            GameObject * obj = pair.second.get();
            if(math_util::dist(obj->state.pos, m_gameState->mousePos) < obj->size.x)
            {
                m_isDragging = true;
                m_draggedObject = obj;
                break;
            }
        }
    }

    if(m_isPainting)
    {
        if(m_controls.paint)
        {
            point_t levelCoords = m_gameState->level->toLevelCoords(m_gameState->mousePos);
            if(levelCoords.x >= 0 && levelCoords.x < m_gameState->level->width && levelCoords.y >= 0 && levelCoords.y < m_gameState->level->height)
            {
                m_gameState->level->tiles[levelCoords.x][levelCoords.y].type = m_paintType;
            }
        }
        else
        {
            m_isPainting = false;
        }
    }
    else if(m_controls.paint)
    {
        point_t levelCoords = m_gameState->level->toLevelCoords(m_gameState->mousePos);
        if(levelCoords.x >= 0 && levelCoords.x < m_gameState->level->width && levelCoords.y >= 0 && levelCoords.y < m_gameState->level->height)
        {
            m_isPainting = true;
            m_paintType = m_gameState->level->tiles[levelCoords.x][levelCoords.y].type == Level::WALL ? Level::EMPTY : Level::WALL;
            m_gameState->level->tiles[levelCoords.x][levelCoords.y].type = m_paintType;
            m_hasUnsavedChanges = true;
        }
    }


    if(m_controls.save)
    {
        saveLevel(m_gameState.get(), m_levelName);
        m_hasUnsavedChanges = false;
    }
}