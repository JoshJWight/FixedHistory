#include "LoadJsonLevel.hh"

#include <json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

using json=nlohmann::json;

namespace jsonlevel
{

const float SCALE = 20.0f;
const point_t BOTTOM_LEFT(0, 0);

void loadLevel(GameState * state, const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    
    std::ifstream file(BASE_LEVEL_DIR + levelName + ".json");
    if(!file.is_open())
    {
        throw std::runtime_error("Could not open level " + levelName);
    }

    json input = json::parse(file);


    int width, height;

    width = input["width"];
    height = input["height"];

    std::cout << "Width: " << width << " Height: " << height << std::endl;


    std::istringstream levelData(input["map"].get<std::string>());
    std::vector<std::string> tileLines;
    for(int i = 0; i < height; i++)
    {
        std::string line;
        std::getline(levelData, line);
        std::cout << line << std::endl;
        tileLines.push_back(line);
    }

    state->level = std::make_shared<Level>(width, height, BOTTOM_LEFT, SCALE);
    state->level->setFromLines(tileLines);

    for(json & object: input["objects"])
    {
        
        std::string objType = object["type"];
        int id = object["id"];
        point_t location = point_t(object["pos"]["x"], object["pos"]["y"]);
        std::cout << "Object type: " << objType << " ID: " << id << " Location: " << location.x << ", " << location.y << std::endl;

        switch(GameObject::stringToType(objType))
        {
            case GameObject::PLAYER:
            {
                std::shared_ptr<Player> player = std::make_shared<Player>(id);
                player->state.pos = location;
                state->addObject(player);
                break;
            }
            case GameObject::ENEMY:
            {
                std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>(id);
                enemy->state.pos = location;
                for(json & point : object["patrolPoints"])
                {
                    enemy->patrolPoints.push_back(point_t(point["x"], point["y"]));
                }
                state->addObject(enemy);
                break;
            }
            case GameObject::SWITCH:
            {
                std::shared_ptr<Switch> sw = std::make_shared<Switch>(id);
                sw->state.pos = location;
                sw->state.aiState = object["state"] == "on" ? Switch::ON : Switch::OFF;
                state->addObject(sw);
                break;
            }
            case GameObject::DOOR:
            {
                std::shared_ptr<Door> door = std::make_shared<Door>(id);
                door->state.pos = location;
                for(int sw : object["connectedSwitches"])
                {
                    door->connectedSwitches.push_back(sw);
                }
                state->addObject(door);
                break;
            }
            case GameObject::TIMEBOX:
            {
                std::shared_ptr<TimeBox> timeBox = std::make_shared<TimeBox>(id);
                timeBox->state.pos = location;
                state->addObject(timeBox);
                break;
            }
            case GameObject::CLOSET:
            {
                std::shared_ptr<Closet> closet = std::make_shared<Closet>(id);
                closet->state.pos = location;
                state->addObject(closet);
                break;
            }
            case GameObject::TURNSTILE:
            {
                std::shared_ptr<Turnstile> turnstile = std::make_shared<Turnstile>(id);
                turnstile->state.pos = location;
                state->addObject(turnstile);
                break;
            }
            case GameObject::SPIKES:
            {
                std::shared_ptr<Spikes> spikes = std::make_shared<Spikes>(id);
                spikes->state.pos = location;
                spikes->downDuration = object["downDuration"];
                spikes->upDuration = object["upDuration"];
                spikes->cycleOffset = object["cycleOffset"];
                state->addObject(spikes);
                break;
            }
            case GameObject::OBJECTIVE:
            {
                std::shared_ptr<Objective> objective = std::make_shared<Objective>(id);
                objective->state.pos = location;
                state->addObject(objective);
                break;
            }
            case GameObject::KNIFE:
            {
                std::shared_ptr<Knife> knife = std::make_shared<Knife>(id);
                knife->state.pos = location;
                state->addObject(knife);
                break;
            }
            case GameObject::GUN:
            {
                std::shared_ptr<Gun> gun = std::make_shared<Gun>(id);
                gun->state.pos = location;
                state->addObject(gun);
                break;
            }
            case GameObject::EXIT:
            {
                std::shared_ptr<Exit> exit = std::make_shared<Exit>(id);
                exit->state.pos = location;
                state->addObject(exit);
                break;
            }
            default:
            {
                throw std::runtime_error("Unknown object type " + objType);
            }
        }
    }
}//loadLevel

void saveLevel(GameState * state, const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    std::ofstream file(BASE_LEVEL_DIR + levelName + ".json");
    if(!file.is_open())
    {
        std::cout << "ERROR: Could not open file for level " << levelName << std::endl;
        return;
    }

    json output;
    output["width"] = state->level->width;
    output["height"] = state->level->height;

    std::stringstream levelData;
    for(int y = state->level->height - 1; y >= 0; y--)
    {
        for(int x = 0; x < state->level->width; x++)
        {
            if(state->level->tiles[x][y].type == Level::EMPTY)
            {
                levelData << ".";
            }
            else if(state->level->tiles[x][y].type == Level::WALL)
            {
                levelData << "X";
            }
            else
            {
                levelData << "?";
            }
        }
        levelData << std::endl;
    }
    output["map"] = levelData.str();

    output["objects"] = json::array();
    for(auto pair : state->objects())
    {
        GameObject * obj = pair.second.get();
        json objData;
        objData["type"] = GameObject::typeToString(obj->type());
        objData["id"] = obj->id;
        
        json pos;
        pos["x"] = obj->state.pos.x;
        pos["y"] = obj->state.pos.y;

        objData["pos"] = pos;

        switch(obj->type())
        {
            case GameObject::PLAYER:
            {
                break;
            }
            case GameObject::ENEMY:
            {
                Enemy * enemy = static_cast<Enemy*>(obj);

                json patrolPoints = json::array();

                for(point_t patrolPoint : enemy->patrolPoints)
                {
                    json point;
                    point["x"] = patrolPoint.x;
                    point["y"] = patrolPoint.y;
                    patrolPoints.push_back(point);
                }
                objData["patrolPoints"] = patrolPoints;
                break;
            }
            case GameObject::SWITCH:
            {
                Switch * sw = static_cast<Switch*>(obj);
                objData["state"] = sw->state.aiState == Switch::ON ? "on" : "off";
                break;
            }
            case GameObject::DOOR:
            {
                Door * door = static_cast<Door*>(obj);

                json connectedSwitches = json::array();
                for(int sw : door->getConnectedSwitches())
                {
                    connectedSwitches.push_back(sw);
                }
                objData["connectedSwitches"] = connectedSwitches;
                break;
            }
            case GameObject::TIMEBOX:
            {
                break;
            }
            case GameObject::CLOSET:
            {
                break;
            }
            case GameObject::TURNSTILE:
            {
                break;
            }
            case GameObject::SPIKES:
            {
                Spikes * spikes = static_cast<Spikes*>(obj);
                objData["downDuration"] = spikes->downDuration;
                objData["upDuration"] = spikes->upDuration;
                objData["cycleOffset"] = spikes->cycleOffset;
                break;
            }
            case GameObject::OBJECTIVE:
            {
                break;
            }
            case GameObject::KNIFE:
            {
                break;
            }
            case GameObject::GUN:
            {
                break;
            }
            case GameObject::EXIT:
            {
                break;
            }
            default:
            {
                throw std::runtime_error("Unknown object type " + GameObject::typeToString(obj->type()));
            }
        }
        output["objects"].push_back(objData);
    }

    file << output;

    file.close();
}

bool levelExists(const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    std::ifstream file(BASE_LEVEL_DIR + levelName + ".json");
    return file.is_open();
}

}//namespace jsonlevel