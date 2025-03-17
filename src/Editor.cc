#include "Editor.hh"
#include <thread>
#include <tick/tickSwitch.hh>
#include <tick/tickDoor.hh>

Editor::Editor(Graphics* graphics, const std::string & level)
    : m_graphics(graphics)
    , m_levelName(level)
    , m_gameState(new GameState())
    , m_controls()
    , m_state(&m_gameState->editorState)
    , m_cameraCenter(0, 0)
    , m_hasUnsavedChanges(false)
{
    m_gameState->infoString = 
        "select: Q\n"
        "connect: E\n"
        "remove: X\n"
        "add row: R\n"
        "add col: C\n"
        "remove row: Shift + R\n"
        "remove col: Shift + C\n"
        "player: 1\n"
        "enemy: 2\n"
        "time box: 3\n"
        "switch: 4\n"
        "door: 5\n"
        "closet: 6\n"
        "turnstile: 7\n"
        "spikes: 8\n"
        "objective: 9\n"
        "knife: 0\n"
        "exit: -\n"
        "gun: =\n"
        "snap to grid: \\\n";

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

    m_graphics->shouldDrawDebug = true;
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

        //Update certain objects
        for(Switch * sw : m_gameState->switches())
        {
            sw->nextState = sw->state;
            tick::tickSwitch(m_gameState.get(), sw);
            sw->state = sw->nextState;
        }
        for(Door * door : m_gameState->doors())
        {
            door->nextState = door->state;
            tick::tickDoor(m_gameState.get(), door);
            door->state = door->nextState;
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

    //CAMERA PAN
    float CAMERA_SPEED = 2.0f / m_graphics->m_cameraScale * 20.0;

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

    //DRAG
    if(m_state->isDragging)
    {
        if(m_controls.drag)
        {
            m_state->draggedObject->state.pos = m_gameState->mousePos;
        }
        else
        {
            m_state->draggedObject->state.pos = placement(m_gameState->mousePos);

            if(m_state->draggedObject->type() == GameObject::ENEMY)
            {
                Enemy * enemy = static_cast<Enemy*>(m_state->draggedObject);
                enemy->patrolPoints[0].x = enemy->state.pos.x;
                enemy->patrolPoints[0].y = enemy->state.pos.y;
            }

            m_state->isDragging = false;
            m_state->draggedObject = nullptr;
            m_hasUnsavedChanges = true;
        }
    }
    else if(m_controls.drag)
    {
        if(highlightedObject != nullptr)
        {
            m_state->isDragging = true;
            m_state->draggedObject = highlightedObject;
            m_state->selectedObject = highlightedObject;
        }
    }

    //PAINT
    if(m_state->isPainting)
    {
        if(m_controls.paint)
        {
            //Nothing to do until paint is released
        }
        else
        {
            //When paint button is released, apply paint to every tile in the selected area
            point_t start = m_gameState->level->toLevelCoords(m_state->paintOrigin);
            point_t end = m_gameState->level->toLevelCoords(m_gameState->mousePos);

            for(int x = std::min(start.x, end.x); x <= std::max(start.x, end.x); x++)
            {
                for(int y = std::min(start.y, end.y); y <= std::max(start.y, end.y); y++)
                {
                    if(x >= 0 && x < m_gameState->level->width && y >= 0 && y < m_gameState->level->height)
                    {
                        m_gameState->level->tiles[x][y].type = m_state->paintType;
                    }
                }
            }
            m_hasUnsavedChanges = true;
            m_state->isPainting = false;
        }
    }
    else if(m_controls.paint)
    {
        point_t levelCoords = m_gameState->level->toLevelCoords(m_gameState->mousePos);
        if(levelCoords.x >= 0 && levelCoords.x < m_gameState->level->width && levelCoords.y >= 0 && levelCoords.y < m_gameState->level->height)
        {
            m_state->isPainting = true;
            m_state->paintType = m_gameState->level->tiles[levelCoords.x][levelCoords.y].type == Level::WALL ? Level::EMPTY : Level::WALL;
            m_state->paintOrigin = m_gameState->mousePos;
        }
    }

    //SELECT
    if(m_controls.select)
    {
        if(highlightedObject != nullptr)
        {
            m_state->selectedObject = highlightedObject;
            m_state->hasConnected = false;
            std::cout << "Selected object with ID " << m_state->selectedObject->id << std::endl;
        }
        else
        {
            std::cout << "Deselected object" << std::endl;
            m_state->selectedObject = nullptr;
        }
    }

    //REMOVE
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

    //CONNECT
    if(m_controls.connect && m_state->selectedObject != nullptr)
    {
        switch(m_state->selectedObject->type())
        {
            case GameObject::DOOR:
            {
                Door * door = static_cast<Door*>(m_state->selectedObject);
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
                Enemy * enemy = static_cast<Enemy*>(m_state->selectedObject);

                //Reset patrol points when placing the first new patrol point
                if(!m_state->hasConnected)
                {
                    enemy->patrolPoints.clear();
                    enemy->patrolPoints.push_back(enemy->state.pos);
                }

                enemy->patrolPoints.push_back(placement(m_gameState->mousePos));
                m_hasUnsavedChanges = true;
                std::cout << "Added patrol point " << m_gameState->mousePos.x << ", " << m_gameState->mousePos.y << std::endl;
                break;
            }
            case GameObject::SWITCH:
            {
                Switch * sw = static_cast<Switch*>(m_state->selectedObject);
                sw->state.aiState = sw->state.aiState == Switch::ON ? Switch::OFF : Switch::ON;
                m_hasUnsavedChanges = true;
                break;
            }
            default:
            {
                std::cout << "Cannot connect with selected object type " << GameObject::typeToString(m_state->selectedObject->type()) << std::endl;
            }
        }//End switch
        m_state->hasConnected = true;
    }

    //ROW/COLUMN MANAGEMENT
    if(m_controls.addRow)
    {
        m_gameState->level->height++;
        for(int x = 0; x < m_gameState->level->width; x++)
        {
            m_gameState->level->tiles[x].push_back(Level::Tile(Level::WALL));
        }
        m_gameState->level->setupNavMesh();
        m_hasUnsavedChanges = true;
    }
    if(m_controls.addCol)
    {
        m_gameState->level->width++;
        m_gameState->level->tiles.push_back(std::vector<Level::Tile>(m_gameState->level->height, Level::Tile(Level::WALL)));
        m_gameState->level->setupNavMesh();
        m_hasUnsavedChanges = true;
    }
    if(m_controls.removeRow)
    {
        if(m_gameState->level->height > 1)
        {
            m_gameState->level->height--;
            for(int x = 0; x < m_gameState->level->width; x++)
            {
                m_gameState->level->tiles[x].pop_back();
            }
            m_hasUnsavedChanges = true;
        }
    }
    if(m_controls.removeCol)
    {
        if(m_gameState->level->width > 1)
        {
            m_gameState->level->width--;
            m_gameState->level->tiles.pop_back();
            m_hasUnsavedChanges = true;
        }
    }

    //PLACE OBJECTS
    if(m_controls.placePlayer)
    {
        if(m_gameState->players().size() == 0)
        {
            std::shared_ptr<Player> player = std::make_shared<Player>(m_gameState->nextID());
            player->state.pos = placement(m_gameState->mousePos);
            m_gameState->addObject(player);
            m_hasUnsavedChanges = true;
            m_state->selectedObject = player.get();
        }
        else
        {
            std::cout << "Player already placed" << std::endl;
        }
    }
    if(m_controls.placeEnemy)
    {
        std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>(m_gameState->nextID());
        enemy->state.pos = placement(m_gameState->mousePos);
        enemy->patrolPoints.push_back(enemy->state.pos);
        m_gameState->addObject(enemy);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = enemy.get();
    }
    if(m_controls.placeTimeBox)
    {
        std::shared_ptr<TimeBox> tb = std::make_shared<TimeBox>(m_gameState->nextID());
        tb->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(tb);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = tb.get();
    }
    if(m_controls.placeSwitch)
    {
        std::shared_ptr<Switch> sw = std::make_shared<Switch>(m_gameState->nextID());
        sw->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(sw);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = sw.get();
    }
    if(m_controls.placeDoor)
    {
        std::shared_ptr<Door> door = std::make_shared<Door>(m_gameState->nextID());
        door->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(door);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = door.get();
    }
    if(m_controls.placeCloset)
    {
        std::shared_ptr<Closet> closet = std::make_shared<Closet>(m_gameState->nextID());
        closet->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(closet);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = closet.get();
    }
    if(m_controls.placeTurnstile)
    {
        std::shared_ptr<Turnstile> turnstile = std::make_shared<Turnstile>(m_gameState->nextID());
        turnstile->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(turnstile);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = turnstile.get();
    }
    if(m_controls.placeSpikes)
    {
        std::shared_ptr<Spikes> spikes = std::make_shared<Spikes>(m_gameState->nextID(), 0, 0, 0);
        spikes->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(spikes);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = spikes.get();
    }
    if(m_controls.placeObjective)
    {
        std::shared_ptr<Objective> obj = std::make_shared<Objective>(m_gameState->nextID());
        obj->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(obj);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = obj.get();
    }
    if(m_controls.placeKnife)
    {
        std::shared_ptr<Knife> knife = std::make_shared<Knife>(m_gameState->nextID());
        knife->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(knife);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = knife.get();
    }
    if(m_controls.placeGun)
    {
        std::shared_ptr<Gun> gun = std::make_shared<Gun>(m_gameState->nextID());
        gun->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(gun);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = gun.get();
    }
    if(m_controls.placeExit)
    {
        std::shared_ptr<Exit> exit = std::make_shared<Exit>(m_gameState->nextID());
        exit->state.pos = placement(m_gameState->mousePos);
        m_gameState->addObject(exit);
        m_hasUnsavedChanges = true;
        m_state->selectedObject = exit.get();
    }

    //SAVE
    if(m_controls.save)
    {
        saveLevel(m_gameState.get(), m_levelName);
        m_hasUnsavedChanges = false;
    }

    if(m_controls.toggleSnapToGrid)
    {
        m_state->snapToGrid = !m_state->snapToGrid;
    }
}

point_t Editor::placement(point_t rawPlacement)
{
    return m_state->placement(rawPlacement, m_gameState->level->scale);
}