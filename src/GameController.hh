#include <objects/GameObject.hh>
#include <objects/Player.hh>
#include <objects/Bullet.hh>
#include <objects/Enemy.hh>
#include <objects/TimeBox.hh>
#include <objects/Switch.hh>
#include <objects/Door.hh>
#include <objects/Closet.hh>
#include <objects/Turnstile.hh>
#include <objects/Container.hh>
#include <objects/Spikes.hh>
#include "Graphics.hh"
#include "TextureBank.hh"
#include "Level.hh"
#include "Controls.hh"
#include "GameState.hh"
#include "Search.hh"
#include "Observation.hh"
#include "Demo.hh"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>



class  GameController
{
public:
    GameController(const std::string & levelPath, Graphics * graphics, DemoReader * demoReader, DemoWriter * demoWriter);
    //False means restart level, true means go to next level
    bool mainLoop();
private:
    enum TickType
    {
        ADVANCE,
        REWIND,
        PAUSE
    };

    bool checkParadoxes();
    bool checkWin();

    void addObject(std::shared_ptr<GameObject> obj);

    void restoreState(int tick);
    void popTimeline();
    void pushTimeline();

    bool playerVisibleToEnemy(Player* player, Enemy* enemy);
    void navigateEnemy(Enemy* enemy, point_t target);

    void tickPlayer(Player* player);
    void tickBullet(Bullet* bullet);
    void tickEnemy(Enemy* enemy);
    void tickContainer(Container* container);
    void tickSwitch(Switch* sw);
    void tickDoor(Door* door);
    void tickSpikes(Spikes* spikes);
    void tickThrowable(Throwable* throwable);
    void playTick();

    void tick(TickType type);

    Graphics * m_graphics;
    Controls m_controls;
    DemoReader * m_demoReader;
    DemoWriter * m_demoWriter;

    std::shared_ptr<GameState> m_gameState;

    int m_currentTick;
    int m_currentTimeline;

    bool m_backwards;

    //Text shown in the middle of the screen
    std::string m_statusString;


    //Flags only valid for the current tick
    bool m_shouldReverse;
    int m_boxToEnter;
};
