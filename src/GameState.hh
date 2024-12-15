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
#include "objects/Knife.hh"
#include "objects/Exit.hh"
#include <objects/GameObject.hh>

#include <vector>

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

typedef std::vector<std::vector<bool>> VisibilityGrid;

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
    std::vector<Exit*> exits;
    int m_lastID;

    VisibilityGrid obstructionGrid;

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

    void deleteObject(int id)
    {
        std::cout << "Deleting " << GameObject::typeToString(objects[id]->type()) << " with ID " << id << std::endl;
        switch(objects[id]->type())
        {
            case GameObject::PLAYER:
                std::erase_if(players, [id](Player* p){return p->id == id;});
                break;
            case GameObject::BULLET:
                std::erase_if(bullets, [id](Bullet* b){return b->id == id;});
                break;
            case GameObject::ENEMY:
                std::erase_if(enemies, [id](Enemy* e){return e->id == id;});
                break;
            case GameObject::SWITCH:
                std::erase_if(switches, [id](Switch* s){return s->id == id;});
                break;
            case GameObject::DOOR:
                std::erase_if(doors, [id](Door* d){return d->id == id;});
                break;
            case GameObject::TIMEBOX:
            case GameObject::CLOSET:
            case GameObject::TURNSTILE:
                std::erase_if(containers, [id](Container* c){return c->id == id;});
                break;
            case GameObject::SPIKES:
                std::erase_if(spikes, [id](Spikes* s){return s->id == id;});
                break;
            case GameObject::OBJECTIVE:
            case GameObject::KNIFE:
                std::erase_if(throwables, [id](Throwable* t){return t->id == id;});
                break;
            case GameObject::EXIT:
                std::erase_if(exits, [id](Exit* e){return e->id == id;});
                break;
            default:
                throw std::runtime_error("Object type " + GameObject::typeToString(objects[id]->type()) + " not handled in deleteObject");
                break;
        }
        objects.erase(id);
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

    void doRewindCleanup(int tick, int timeline)
    {
        std::vector<int> toDelete;
        for(auto pair : objects)
        {
            GameObject* obj = pair.second.get();

            //Delete objects whose origin we've rewound past
            if(obj->initialTimeline > timeline)
            {
                toDelete.push_back(obj->id);
            }
            else if(pair.second->initialTimeline == timeline)
            {
                if(obj->backwards)
                {
                    if(obj->ending < tick)
                    {
                        toDelete.push_back(obj->id);
                    }
                }
                else
                {
                    if(obj->beginning > tick)
                    {
                        toDelete.push_back(obj->id);
                    }
                }
            }

            //Remove an object's ending marker if we've rewound past that ending
            if(obj->hasFinalTimeline && obj->finalTimeline > timeline)
            {
                obj->hasFinalTimeline = false;
                if(obj->backwards)
                {
                    obj->beginning = 0;
                }
                else
                {
                    obj->hasEnding = false;
                }
            }
            else if(obj->hasFinalTimeline && obj->finalTimeline == timeline)
            {
                if(obj->backwards)
                {
                    if(obj->beginning < tick)
                    {
                        obj->beginning = 0;
                        obj->hasFinalTimeline = false;
                    }
                }
                else
                {
                    if(obj->ending > tick)
                    {
                        obj->hasEnding = false;
                        obj->hasFinalTimeline = false;
                    }
                }
            }
        }
        for(int id : toDelete)
        {
            deleteObject(id);
        }


        //Special case stuff
        //If the active player is in a box, set the active occupant to the player
        if(players.back()->state.boxOccupied)
        {
            Container* box = dynamic_cast<Container*>(objects[players.back()->state.attachedObjectId].get());
            box->activeOccupant = players.back();
        }
    }
};

std::shared_ptr<GameState> loadGameState(const std::string& filename);

#endif