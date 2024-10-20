#ifndef __GAMESTATE_HH__
#define __GAMESTATE_HH__

#include "Level.hh"
#include "objects/Player.hh"
#include "objects/Bullet.hh"
#include "objects/Enemy.hh"
#include "objects/TimeBox.hh"
#include "objects/Switch.hh"
#include "objects/Door.hh"
#include "objects/Closet.hh"
#include "objects/Turnstile.hh"
#include "objects/Container.hh"
#include "objects/Spikes.hh"
#include "objects/Throwable.hh"
#include "objects/Objective.hh"
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

    std::vector<Player*> players;
    std::vector<Bullet*> bullets;
    std::vector<Enemy*> enemies;
    std::vector<Switch*> switches;
    std::vector<Door*> doors;
    std::vector<Container*> containers;
    std::vector<Spikes*> spikes;
    std::vector<Throwable*> throwables;

    GameState()
        : level(nullptr)
        , m_lastID(0)
    {
        historyBuffers.push_back(HistoryBuffer());
    }

    //Only for initial setup
    void addObject(std::shared_ptr<GameObject> obj)
    {
        objects[obj->id] = obj;
        historyBuffers.back().buffer[obj->id] = std::vector<ObjectState>(1);
        historyBuffers.back().buffer[obj->id][0] = obj->state;

        if(obj->id >= m_lastID)
        {
            m_lastID = obj->id + 1;
        }
    }

    void restoreState(int tick)
    {
        for(auto pair : objects)
        {
            std::shared_ptr<GameObject> obj = pair.second;
            obj->state = historyBuffers.back()[obj->id][tick];
        }
    }

    template <typename T>
    T* getObject(int id)
    {
        auto it = objects.find(id);
        if(it == objects.end())
        {
            throw std::runtime_error("No object with ID " + std::to_string(id));
        }

        T* ptr = dynamic_cast<T*>(it->second.get());
        if(ptr == nullptr)
        {
            throw std::runtime_error("Object with ID " + std::to_string(id) + " is not of type " + typeid(T).name());
        }
        return ptr;
    }

    int nextID(){
        return m_lastID++;
    }

    int m_lastID;
};

std::shared_ptr<GameState> loadGameState(const std::string& filename);

#endif