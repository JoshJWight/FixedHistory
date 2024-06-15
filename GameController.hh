#include "GameObject.hh"
#include "Player.hh"
#include "Bullet.hh"
#include "Enemy.hh"
#include "TimeBox.hh"
#include "Graphics.hh"
#include "TextureBank.hh"
#include "Level.hh"
#include "Controls.hh"
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
    enum TickType
    {
        ADVANCE,
        REWIND,
        PAUSE
    };

    bool checkParadoxes();

    void addObject(std::shared_ptr<GameObject> obj);

    void checkBulletUndo();
    void restoreState(int tick);
    void popTimeline();
    void pushTimeline();

    bool playerVisibleToEnemy(Player* player, Enemy* enemy);
    void navigateEnemy(Enemy* enemy, point_t target);

    void tickPlayer(Player* player);
    void tickBullet(Bullet* bullet);
    void tickEnemy(Enemy* enemy);
    void tickTimeBox(TimeBox* timeBox);
    void playTick();

    void tick(TickType type);

    int nextID(){
        return m_lastID++;
    }

    int m_lastID;

    Graphics m_graphics;
    Controls m_controls;

    Level m_level;
    std::map<int, std::shared_ptr<GameObject>> m_objects;

    std::vector<HistoryBuffer> m_historyBuffers;

    std::vector<Player*> m_players;
    std::vector<Bullet*> m_bullets;
    std::vector<Enemy*> m_enemies;
    std::vector<TimeBox*> m_timeBoxes;

    int m_currentTick;
    int m_currentTimeline;

    bool m_backwards;

    int m_lastBreakpoint;

    //Text shown in the middle of the screen
    std::string m_statusString;


    //Flags only valid for the current tick
    bool m_shouldReverse;
    TimeBox* m_boxToEnter;
};
