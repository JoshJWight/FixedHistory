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
    , m_cameraCenter(0, 0)
    , m_selectedObject(nullptr)
    , m_hasUnsavedChanges(false)
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

    GameObject * highlightedObject = nullptr;
    float minDist = 1000000;
    for(auto pair: m_gameState->objects())
    {
        GameObject * obj = pair.second.get();
        float dist = math_util::dist(obj->state.pos, m_gameState->mousePos);
        if(dist < obj->size.x && dist < minDist)
        {
            highlightedObject = obj;
            minDist = dist;
        }
    }


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
        if(highlightedObject != nullptr)
        {
            m_isDragging = true;
            m_draggedObject = highlightedObject;
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

    if(m_controls.select)
    {
        if(highlightedObject != nullptr)
        {
            m_selectedObject = highlightedObject;
            std::cout << "Selected object with ID " << m_selectedObject->id << std::endl;
            if(m_selectedObject->type() == GameObject::ENEMY)
            {
                Enemy * enemy = static_cast<Enemy*>(m_selectedObject);
                enemy->patrolPoints.clear();
                enemy->patrolPoints.push_back(enemy->state.pos);
            }
        }
        else
        {
            std::cout << "Deselected object" << std::endl;
            m_selectedObject = nullptr;
        }
    }

    if(m_controls.remove)
    {
        if(highlightedObject != nullptr)
        {
            std::cout << "Deleted object with ID " << highlightedObject->id << std::endl;

            //This could cause segfaults if this pointer is being dragged or something
            //but it's basically ok for the editor to have edge cases like that
            m_gameState->objects().erase(highlightedObject->id);
            m_hasUnsavedChanges = true;
        }
    }

    if(m_controls.connect && m_selectedObject != nullptr)
    {
        switch(m_selectedObject->type())
        {
            case GameObject::DOOR:
            {
                Door * door = static_cast<Door*>(m_selectedObject);
                if(highlightedObject != nullptr && highlightedObject->type() == GameObject::SWITCH)
                {
                    Switch * sw = static_cast<Switch*>(highlightedObject);
                    if(std::find(door->getConnectedSwitches().begin(), door->getConnectedSwitches().end(), sw->id) == door->getConnectedSwitches().end())
                    {
                        door->addSwitch(sw);
                    }
                    else
                    {
                        std::erase_if(door->connectedSwitches, [sw](int id) { return id == sw->id; });
                    }
                    m_hasUnsavedChanges = true;
                }
                break;
            }
            case GameObject::ENEMY:
            {
                Enemy * enemy = static_cast<Enemy*>(m_selectedObject);
                enemy->patrolPoints.push_back(m_gameState->mousePos);
                m_hasUnsavedChanges = true;
                std::cout << "Added patrol point " << m_gameState->mousePos.x << ", " << m_gameState->mousePos.y << std::endl;
                break;
            }
            case GameObject::SWITCH:
            {
                Switch * sw = static_cast<Switch*>(m_selectedObject);
                sw->state.aiState = sw->state.aiState == Switch::ON ? Switch::OFF : Switch::ON;
                m_hasUnsavedChanges = true;
                break;
            }
            default:
            {
                std::cout << "Cannot connect with selected object type " << GameObject::typeToString(m_selectedObject->type()) << std::endl;
            }
        }
    }


    if(m_controls.save)
    {
        saveLevel(m_gameState.get(), m_levelName);
        m_hasUnsavedChanges = false;
    }
}