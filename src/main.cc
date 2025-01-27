#include <argparse/argparse.hpp>
#include "GameController.hh"
#include "Editor.hh"
#include <io/Graphics.hh>
#include <io/Demo.hh>

int main(int argc, char** argv)
{
    argparse::ArgumentParser program("FixedHistory");
    
    program.add_argument("-d", "--demo")
        .help("Demo file to play")
        .default_value(std::string(""));
    
    program.add_argument("levels")
        .help("Level to play")
        .remaining();

    program.add_argument("-e", "--edit")
        .help("Launch in editor mode")
        .default_value(false)
        .implicit_value(true);

    try
    {
        program.parse_args(argc, argv);
    }
    catch(const std::runtime_error & err)
    {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }

    auto levels = program.get<std::vector<std::string>>("levels");
    if(levels.size() == 0)
    {
        std::cout << "Specify at least one level via positional args" << std::endl;
        exit(0);
    }

    Graphics graphics(1920, 1080);

    if(program["--edit"] == true)
    {
        std::cout << "Launching editor" << std::endl;
        Editor editor(&graphics, levels[0]);
        editor.mainLoop();
        return 0;
    }
    else
    {
        std::string demo = program.get<std::string>("--demo");
        std::shared_ptr<DemoReader> demoReader;
        if(demo != "")
        {
            std::cout << "Playing demo " << demo << std::endl;
            demoReader.reset(new DemoReader(demo));
        }
        else
        {
            std::cout << "Not playing demo" << std::endl;
        }
        std::shared_ptr<DemoWriter> demoWriter(new DemoWriter("most_recent_demo"));

        for(int i = 0; i < levels.size(); i++)
        {
            std::string level = std::string(levels[i]);
            GameController gc(level, &graphics, demoReader.get(), demoWriter.get());
            bool rc = gc.mainLoop();
            if(!rc)
            {
                //Restart level
                i--;
            }
        }
    }

    return 0;
}
