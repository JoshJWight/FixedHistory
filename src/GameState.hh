#ifndef __GAMESTATE_HH__
#define __GAMESTATE_HH__

#include "Level.hh"
#include <objects/GameObject.hh>

struct HistoryBuffer
{
    HistoryBuffer()
        : breakpoint(0)
    {
    }

    HistoryBuffer(const HistoryBuffer& other, int breakpoint)
        : buffer(other.buffer)
        , breakpoint(breakpoint)
    {
    }

    std::vector<ObjectState>& operator[](int i)
    {
        if(buffer.find(i) == buffer.end())
        {
            throw std::runtime_error("No buffer for object " + std::to_string(i));
        }
        return buffer.at(i);
    }

    std::map<int, std::vector<ObjectState>> buffer;
    int breakpoint;
};

struct GameState {
    std::shared_ptr<Level> level;
    std::map<int, std::shared_ptr<GameObject>> objects;
    std::vector<HistoryBuffer> historyBuffers;

    GameState()
        : level(nullptr)
    {
        historyBuffers.push_back(HistoryBuffer());
    }

    //Only for initial setup
    void addObject(std::shared_ptr<GameObject> obj)
    {
        objects[obj->id] = obj;
        historyBuffers.back().buffer[obj->id] = std::vector<ObjectState>(1);
        historyBuffers.back().buffer[obj->id][0] = obj->state;
    }

    void restoreState(int tick)
    {
        for(auto pair : objects)
        {
            std::shared_ptr<GameObject> obj = pair.second;
            obj->state = historyBuffers.back()[obj->id][tick];
        }
    }
};

#endif