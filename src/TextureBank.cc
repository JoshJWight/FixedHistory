#include "TextureBank.hh"

std::shared_ptr<TextureBank> TextureBank::instance;

sf::Texture& TextureBank::get_texture(const std::string & filename)
{
    if(m_map.count(filename) == 0)
    {
        m_map[filename] = sf::Texture();
        std::string assetsPath = "assets/";
        m_map[filename].loadFromFile(assetsPath + filename);
    }

    return m_map[filename];
}

sf::Font& TextureBank::get_font()
{
    if(m_font.get() == nullptr)
    {
        m_font.reset(new sf::Font());
        m_font->loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf");
    }
    return *m_font;
}

TextureBank& TextureBank::getInstance(){
    if(instance.get() == nullptr)
    {
        instance.reset(new TextureBank());
    }
    return *instance;
}
