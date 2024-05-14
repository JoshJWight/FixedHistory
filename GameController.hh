#include "GameObject.hh"
#include "Player.hh"
#include "Bullet.hh"
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

    void checkBulletUndo();
    void restoreState(int tick);
    void popTimeline();
    void pushTimeline();

    void tickPlayer(Player* player);
    void tickBullet(Bullet* bullet);
    void playTick();

    void tick();

    int nextID(){
        return m_lastID++;
    }

    int m_lastID;

    Graphics m_graphics;
    std::map<int, std::shared_ptr<GameObject>> m_objects;

    std::vector<HistoryBuffer> m_historyBuffers;

    std::vector<Player*> m_players;
    std::vector<Bullet*> m_bullets;

    int m_currentTick;
    int m_currentTimeline;

    bool m_backwards;

    int m_lastBreakpoint;

    //Keyboard jank
    bool m_eUnpressed;
};
