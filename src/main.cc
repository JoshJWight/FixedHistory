#include "GameController.hh"

int main(int argc, char** argv)
{
    std::string level = "levels/knifetest.txt";

    if(argc > 1)
    {
        level = "levels/" + std::string(argv[1]) + ".txt";
    }

    GameController gc(level);
    gc.mainLoop();
    return 0;
}
