#ifndef __LEVEL_HH__
#define __LEVEL_HH__

#include <vector>
#include <string>
#include <map>
#include <queue>
#include <stdexcept>
#include <iostream>

#include <utils/MathUtil.hh>

class Level {
public:
    enum TileType {
        EMPTY,
        WALL
    };

    struct NavNode {
        NavNode()
            : pos(0, 0)
            , x(0)
            , y(0)
            , id(0)
        {
        }

        //Position in world space
        point_t pos;
        //Position in level grid
        int x;
        int y;
        int id;
        std::vector<NavNode*> neighbors;
    };

    struct Tile {
        TileType type;
        NavNode node;
    };

    Level(int _width, int _height, const point_t & _bottomLeft, float _scale);

    void setupNavMesh();

    struct NavMove {
        //Since priority_queue is a max heap, we want to reverse the comparison
        bool operator<(const NavMove & other) const {
            return dist > other.dist;
        }

        NavNode* node;
        float dist;
    };

    void setFromLines(const std::vector<std::string> & lines);

    void setFromString(const std::string & str);

    TileType tileAt(const point_t & pos) const;

    const NavNode* nodeAt(const point_t & pos) const;

    point_t toLevelCoords(const point_t & worldPos) const;
    point_t fromLevelCoords(const point_t & levelPos) const;

    bool worldCoordsInBounds(const point_t & worldPos) const;
    bool levelCoordsInBounds(const point_t & levelPos) const;

    point_t bottomLeft;
    size_t width;
    size_t height;
    float scale;

    std::vector<std::vector<Tile>> tiles;
    
};

#endif