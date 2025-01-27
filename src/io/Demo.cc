#include "Demo.hh"

DemoReader::DemoReader(const std::string & path)
    : m_file(path)
{
}

bool DemoReader::getNextFrame(DemoFrame & frame)
{
    if(m_file.eof())
    {
        return false;
    }

    m_file.read(reinterpret_cast<char*>(&frame.controls), sizeof(short));
    m_file.read(reinterpret_cast<char*>(&frame.mouseX), sizeof(float));
    m_file.read(reinterpret_cast<char*>(&frame.mouseY), sizeof(float));

    return true;
}

DemoWriter::DemoWriter(const std::string & path)
    : m_file(path)
{
}

void DemoWriter::writeFrame(const DemoFrame & frame)
{
    m_file.write(reinterpret_cast<const char*>(&frame.controls), sizeof(short));
    m_file.write(reinterpret_cast<const char*>(&frame.mouseX), sizeof(float));
    m_file.write(reinterpret_cast<const char*>(&frame.mouseY), sizeof(float));
}