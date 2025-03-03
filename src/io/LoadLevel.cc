#include "LoadLevel.hh"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

const float SCALE = 20.0f;
const point_t BOTTOM_LEFT(0, 0);

//To parse things like "2.5,4.2"
point_t parsePoint(const std::string & point)
{
    std::istringstream iss(point);
    std::string x, y;
    std::getline(iss, x, ',');
    std::getline(iss, y, ',');
    return point_t(std::stof(x), std::stof(y));
}

point_t getLocation(const std::vector<std::string>& tileLines, const std::string & location, float scale, point_t bottomLeft)
{
    for(int x = 0; x < tileLines[0].size(); x++)
    {
        for(int y=0; y < tileLines.size(); y++)
        {
            if(tileLines[y][x] == location[0])
            {
                return point_t((x + 0.5) * scale + bottomLeft.x, (tileLines.size() - y - 0.5) * scale + bottomLeft.y);
            }
        }
    }
    std::cout << "Could not find location " << location << std::endl;
    return point_t(0, 0);
}

std::vector<point_t> getLocations(const std::vector<std::string>& tileLines, const std::string & location, float scale, point_t bottomLeft)
{
    std::vector<point_t> locations;
    for(int x = 0; x < tileLines[0].size(); x++)
    {
        for(int y=0; y < tileLines.size(); y++)
        {
            if(tileLines[y][x] == location[0])
            {
                locations.push_back(point_t((x + 0.5) * scale + bottomLeft.x, (tileLines.size() - y - 0.5) * scale + bottomLeft.y));
            }
        }
    }
    if(locations.size() == 0)
    {
        std::cout << "Could not find location " << location << std::endl;
    }
    return locations;
}

void constructObject(GameState * state, int id, const std::string & objType, point_t position, const std::vector<std::string>& tokens, const std::vector<std::string> & tileLines)
{
    std::shared_ptr<GameObject> obj;
    if(objType == "player")
    {
        obj = std::make_shared<Player>(id);
    }
    else if(objType == "enemy")
    {
        std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>(id);
        enemy->patrolPoints.push_back(position);
        for(int i=3; i<tokens.size(); i++)
        {
            std::string patrolPoint = tokens[i];
            //enemy->patrolPoints.push_back(getLocation(tileLines, patrolPoint, SCALE, BOTTOM_LEFT));
            enemy->patrolPoints.push_back(parsePoint(patrolPoint));
        }
        obj = enemy;
    }
    else if(objType == "switch")
    {
        std::shared_ptr<Switch> sw = std::make_shared<Switch>(id);

        if(tokens.size() < 4)
        {
            throw std::runtime_error("Not enough tokens for switch");
        }
        std::string startingState = tokens[3];
        if(startingState == "on")
        {
            sw->state.aiState = Switch::ON;
        }
        else if(startingState == "off")
        {
            sw->state.aiState = Switch::OFF;
        }
        else
        {
            throw std::runtime_error("Unknown switch state " + startingState);
        }
        obj = sw;
    }
    else if(objType == "door")
    {
        std::shared_ptr<Door> door = std::make_shared<Door>(id);
        for(int i=3; i<tokens.size(); i++)
        {
            int switchID = std::stoi(tokens[i]);
            
            std::cout << "Switch ID: " << switchID << std::endl;
            door->connectedSwitches.push_back(switchID);
        }
        obj = door;
    }
    else if(objType == "timebox")
    {
        obj = std::make_shared<TimeBox>(id);
    }
    else if (objType == "closet")
    {
        obj = std::make_shared<Closet>(id);
    }
    else if (objType == "turnstile")
    {
        obj = std::make_shared<Turnstile>(id);
    }
    else if (objType == "spikes")
    {
        int downDuration = 0;
        if(tokens.size() > 3)
        {
            downDuration = std::stoi(tokens[3]);
        }
        int upDuration = 1;
        if(tokens.size() > 4)
        {
            upDuration = std::stoi(tokens[4]);
        }
        int cycleOffset = 0;
        if(tokens.size() > 5)
        {
            cycleOffset = std::stoi(tokens[5]);
        }
        obj = std::make_shared<Spikes>(id, downDuration, upDuration, cycleOffset);
    }
    else if (objType == "objective")
    {
        obj = std::make_shared<Objective>(id);
    }
    else if (objType == "knife")
    {
        obj = std::make_shared<Knife>(id);
    }
    else if (objType == "exit")
    {
        obj = std::make_shared<Exit>(id);
    }
    else
    {
        throw std::runtime_error("Unknown object type " + objType);
    }

    obj->state.pos = position;
    state->addObject(obj);
}

void loadLevel(GameState * state, const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    
    std::ifstream file(BASE_LEVEL_DIR + levelName + ".txt");
    if(!file.is_open())
    {
        throw std::runtime_error("Could not open level " + levelName);
    }
    int width, height;
    file >> width >> height;

    std::cout << "Width: " << width << " Height: " << height << std::endl;

    //Skip the rest of the line
    std::string dummy;
    std::getline(file, dummy);

    std::vector<std::string> tileLines;
    for(int i = 0; i < height; i++)
    {
        std::string line;
        std::getline(file, line);
        std::cout << line << std::endl;
        tileLines.push_back(line);
    }

    

    state->level = std::make_shared<Level>(width, height, BOTTOM_LEFT, SCALE);
    state->level->setFromLines(tileLines);

    while(!file.eof())
    {
        std::string line;
        std::getline(file, line);
        std::cout << line << std::endl;
        
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        while(!iss.eof())
        {
            std::string token;
            iss >> token;
            tokens.push_back(token);
        }

        if(tokens.size() < 3)
        {
            std::cout << "Not enough tokens in this line, ending parsing" << std::endl;
            return;
        }

        std::string objType = tokens[0];
        int id = std::stoi(tokens[1]);
        std::string location = tokens[2];
        std::cout << "Object type: " << objType << " ID: " << id << " Location: " << location << std::endl;

        if(id == -1)
        {
            std::vector<point_t> positions = getLocations(tileLines, location, SCALE, BOTTOM_LEFT);
            for(point_t position : positions)
            {
                constructObject(state, state->nextID(), objType, position, tokens, tileLines);
            }
        }
        else
        {
            //point_t position = getLocation(tileLines, location, SCALE, BOTTOM_LEFT);
            point_t position = parsePoint(location);
            constructObject(state, id, objType, position, tokens, tileLines);
        }
    }

    return;
}

void saveLevel(GameState * state, const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    std::ofstream file(BASE_LEVEL_DIR + levelName + ".txt");
    if(!file.is_open())
    {
        std::cout << "ERROR: Could not open file for level " << levelName << std::endl;
        return;
    }

    file << state->level->width << " " << state->level->height << std::endl;
    for(int y = state->level->height - 1; y >= 0; y--)
    {
        for(int x = 0; x < state->level->width; x++)
        {
            if(state->level->tiles[x][y].type == Level::EMPTY)
            {
                file << ".";
            }
            else if(state->level->tiles[x][y].type == Level::WALL)
            {
                file << "X";
            }
            else
            {
                file << "?";
            }
        }
        file << std::endl;
    }

    for(auto pair : state->objects())
    {
        GameObject * obj = pair.second.get();
        file << GameObject::typeToString(obj->type()) << " " << obj->id << " " << obj->state.pos.x << "," << obj->state.pos.y;

        switch(obj->type())
        {
            case GameObject::PLAYER:
            {
                break;
            }
            case GameObject::ENEMY:
            {
                Enemy * enemy = static_cast<Enemy*>(obj);
                for(point_t patrolPoint : enemy->patrolPoints)
                {
                    file << " " << patrolPoint.x << "," << patrolPoint.y;
                }
                break;
            }
            case GameObject::SWITCH:
            {
                Switch * sw = static_cast<Switch*>(obj);
                file << " " << (sw->state.aiState == Switch::ON ? "on" : "off");
                break;
            }
            case GameObject::DOOR:
            {
                Door * door = static_cast<Door*>(obj);
                for(int sw : door->getConnectedSwitches())
                {
                    file << " " << sw;
                }
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
                file << " " << spikes->downDuration << " " << spikes->upDuration << " " << spikes->cycleOffset;
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
            case GameObject::EXIT:
            {
                break;
            }
            default:
            {
                throw std::runtime_error("Unknown object type " + GameObject::typeToString(obj->type()));
            }
        }

        file << std::endl;
    }

    file.close();
}

bool levelExists(const std::string & levelName)
{
    std::string BASE_LEVEL_DIR = "./levels/";
    std::ifstream file(BASE_LEVEL_DIR + levelName + ".txt");
    return file.is_open();
}