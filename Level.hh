#ifndef __LEVEL_HH__
#define __LEVEL_HH__

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include "MathUtil.hh"

class Level {
public:
    enum TileType {
        EMPTY,
        WALL
    };

    Level(int _width, int _height, const point_t & _bottomLeft, float _scale)
        : bottomLeft(_bottomLeft)
        , scale(_scale)
        , width(_width)
        , height(_height)
    {
        tiles.resize(width);
        for (int i = 0; i < width; i++) {
            tiles[i].resize(height, EMPTY);
        }
    }

    void setFromString(const std::string & str) {
        int x = 0;
        int y = 0;
        for (char c : str) {
            if (c == '\n') {
                std::cout << std::endl;
                x = 0;
                y++;
            } else {
                if(x >= width || y >= height) {
                    throw std::runtime_error("Level does not fit this string");
                }
                if (c == 'X') {
                    std::cout << "X";
                    tiles[x][height - y - 1] = WALL;
                }
                else{
                    std::cout << " ";
                }
                x++;
            }
        }
        std::cout << std::endl;
    }

    TileType tileAt(const point_t & pos) {
        int x = pos.x - bottomLeft.x / scale;
        int y = pos.y - bottomLeft.y / scale;
        if (x < 0 || x >= tiles.size() || y < 0 || y >= tiles[0].size()) {
            return WALL;
        }
        return tiles[x][y];
    }

    point_t bottomLeft;
    size_t width;
    size_t height;
    float scale;

    std::vector<std::vector<TileType>> tiles;
    
};

#endif