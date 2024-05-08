#ifndef __TEXTURE_BANK_HH__
#define __TEXTURE_BANK_HH__

#include <map>
#include <memory>
#include <SFML/Graphics/Texture.hpp>

class TextureBank
{
public:
    static sf::Texture& get(std::string filename){
        return getInstance().get_texture(filename);
    }

protected:
    TextureBank() {};
    sf::Texture& get_texture(std::string filename);

    static std::shared_ptr<TextureBank> instance;
    static TextureBank& getInstance();

    std::map<std::string, sf::Texture> m_map;
};

#endif
