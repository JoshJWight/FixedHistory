#ifndef DEMO_HH
#define DEMO_HH

#include <fstream>

struct DemoFrame
{
    short controls;
    float mouseX;
    float mouseY;
};

class DemoReader
{

public:
    DemoReader(const std::string & path);
    bool getNextFrame(DemoFrame & frame);


private:
    std::ifstream m_file;

};

class DemoWriter
{

public:
    DemoWriter(const std::string & path);
    void writeFrame(const DemoFrame & frame);

private:
    std::ofstream m_file;

};

#endif