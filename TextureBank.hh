#ifndef __TEXTURE_BANK_HH__
#define __TEXTURE_BANK_HH__

#include <map>
#include <memory>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

class TextureBank
{
public:
    static sf::Texture& get(std::string filename){
        return getInstance().get_texture(filename);
    }

    static sf::Font& getFont(){
        return getInstance().get_font();
    }

protected:
    TextureBank() {};
    sf::Texture& get_texture(std::string filename);
    sf::Font& get_font();

    static std::shared_ptr<TextureBank> instance;
    static TextureBank& getInstance();

    std::map<std::string, sf::Texture> m_map;

    std::shared_ptr<sf::Font> m_font;
};

#endif
