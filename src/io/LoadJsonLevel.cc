#include "LoadJsonLevel.hh"

#include <json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

using json=nlohmann::json;

namespace jsonlevel
{

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