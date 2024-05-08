#include "TextureBank.hh"

std::shared_ptr<TextureBank> TextureBank::instance;

sf::Texture& TextureBank::get_texture(std::string filename)
{
    if(m_map.count(filename) == 0)
    {
        m_map[filename] = sf::Texture();
        m_map[filename].loadFromFile(filename);
    }

    return m_map[filename];
}

TextureBank& TextureBank::getInstance(){
    if(instance.get() == nullptr)
    {
        instance.reset(new TextureBank());
    }
    return *instance;
}
