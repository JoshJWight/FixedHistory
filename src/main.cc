#include <argparse/argparse.hpp>
#include "GameController.hh"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <level1> <level2> <level3> etc" << std::endl;
        return 1;
    }

    Graphics graphics(1920, 1080);

    for(int i = 1; i < argc; i++)
    {
        std::string level = "levels/" + std::string(argv[i]) + ".txt";
        GameController gc(level, &graphics);
        bool rc = gc.mainLoop();
        if(!rc)
        {
            //Restart level
            i--;
        }
    }

    return 0;
}
