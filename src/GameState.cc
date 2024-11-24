#include "GameState.hh"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

std::shared_ptr<GameState> loadGameState(const std::string& filename)
{
    std::shared_ptr<GameState> state = std::make_shared<GameState>();
    
    std::ifstream file(filename);
    if(!file.is_open())
    {
        throw std::runtime_error("Could not open file " + filename);
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

    point_t bottomLeft(0, 0);
    float scale = 20.0f;

    state->level = std::make_shared<Level>(width, height, bottomLeft, scale);
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
            std::cout << "Not enough tokens in this line, skipping" << std::endl;
            continue;
        }

        std::string objType = tokens[0];
        int id = std::stoi(tokens[1]);
        std::string location = tokens[2];
        std::cout << "Object type: " << objType << " ID: " << id << " Location: " << location << std::endl;

        point_t position = getLocation(tileLines, location, scale, bottomLeft);

        std::shared_ptr<GameObject> obj;
        if(objType == "player")
        {
            obj = std::make_shared<Player>(id);
            state->players.push_back(static_cast<Player*>(obj.get()));
        }
        else if(objType == "enemy")
        {
            std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>(id);
            enemy->patrolPoints.push_back(position);
            for(int i=3; i<tokens.size(); i++)
            {
                std::string patrolPoint = tokens[i];
                enemy->patrolPoints.push_back(getLocation(tileLines, patrolPoint, scale, bottomLeft));
            }

            state->enemies.push_back(enemy.get());
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

            state->switches.push_back(sw.get());
            obj = sw;
        }
        else if(objType == "door")
        {
            std::shared_ptr<Door> door = std::make_shared<Door>(id);
            for(int i=3; i<tokens.size(); i++)
            {
                int switchID = std::stoi(tokens[i]);
                
                std::cout << "Switch ID: " << switchID << std::endl;
                door->addSwitch(state->getObject<Switch>(switchID));
            }
            state->doors.push_back(door.get());
            obj = door;
        }
        else if(objType == "timebox")
        {
            obj = std::make_shared<TimeBox>(id);
            state->containers.push_back(static_cast<TimeBox*>(obj.get()));
        }
        else if (objType == "closet")
        {
            obj = std::make_shared<Closet>(id);
            state->containers.push_back(static_cast<Closet*>(obj.get()));
        }
        else if (objType == "turnstile")
        {
            obj = std::make_shared<Turnstile>(id);
            state->containers.push_back(static_cast<Turnstile*>(obj.get()));
        }
        else if (objType == "spikes")
        {
            if(tokens.size() < 5)
            {
                throw std::runtime_error("Not enough tokens for spikes");
            }
            int downDuration = std::stoi(tokens[3]);
            int upDuration = std::stoi(tokens[4]);
            int cycleOffset = 0;
            if(tokens.size() > 5)
            {
                cycleOffset = std::stoi(tokens[5]);
            }
            obj = std::make_shared<Spikes>(id, downDuration, upDuration, cycleOffset);
            state->spikes.push_back(static_cast<Spikes*>(obj.get()));
        }
        else if (objType == "objective")
        {
            obj = std::make_shared<Objective>(id);
            state->throwables.push_back(static_cast<Objective*>(obj.get()));
        }
        else if (objType == "knife")
        {
            obj = std::make_shared<Knife>(id);
            state->throwables.push_back(static_cast<Knife*>(obj.get()));
        }
        else if (objType == "exit")
        {
            obj = std::make_shared<Exit>(id);
            state->exits.push_back(static_cast<Exit*>(obj.get()));
        }
        else
        {
            throw std::runtime_error("Unknown object type " + objType);
        }

        obj->state.pos = position;
        state->addObject(obj);
    }

    return state;
}