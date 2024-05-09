#include "GameObject.hh"
#include "Graphics.hh"
#include "TextureBank.hh"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

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

class  GameController
{
public:
    GameController();
    void mainLoop();
private:

    void restoreState(int tick);
    void popTimeline();
    void pushTimeline();

    void playTick();

    void tick();

    int nextID(){
        return m_lastID++;
    }

    int m_lastID;

    Graphics m_graphics;
    std::map<int, std::shared_ptr<GameObject>> m_objects;

    std::vector<HistoryBuffer> m_historyBuffers;

    GameObject* m_player;

    int m_currentTick;

    bool m_backwards;

    int m_lastBreakpoint;

    //Keyboard jank
    bool m_eUnpressed;
};
