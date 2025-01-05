#ifndef GAMECONTROLLER_HH
#define GAMECONTROLLER_HH

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

#include <tick/tickPlayer.hh>
#include <tick/tickBullet.hh>
#include <tick/tickEnemy.hh>
#include <tick/tickContainer.hh>
#include <tick/tickSwitch.hh>
#include <tick/tickDoor.hh>
#include <tick/tickSpikes.hh>
#include <tick/tickThrowable.hh>

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

    void restoreState();
    void popTimeline();
    void pushTimeline();

    bool playerVisibleToEnemy(Player* player, Enemy* enemy);
    void navigateEnemy(Enemy* enemy, point_t target);

    void playTick();

    void tick(TickType type);

    Graphics * m_graphics;
    Controls m_controls;
    DemoReader * m_demoReader;
    DemoWriter * m_demoWriter;

    std::shared_ptr<GameState> m_gameState;
};

#endif