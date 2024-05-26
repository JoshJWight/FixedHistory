#ifndef __LEVEL_HH__
#define __LEVEL_HH__

#include <vector>
#include <string>
#include <map>
#include <queue>
#include <stdexcept>
#include <iostream>

#include "MathUtil.hh"

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

    Level(int _width, int _height, const point_t & _bottomLeft, float _scale)
        : bottomLeft(_bottomLeft)
        , scale(_scale)
        , width(_width)
        , height(_height)
    {
        tiles.resize(width);
        for (int i = 0; i < width; i++) {
            tiles[i].resize(height);
        }
    }

    void setupNavMesh() 
    {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                tiles[x][y].node.pos = point_t((x + 0.5f) * scale + bottomLeft.x, (y + 0.5f) * scale + bottomLeft.y);
                tiles[x][y].node.x = x;
                tiles[x][y].node.y = y;
                tiles[x][y].node.id = y * width + x;

                if (tiles[x][y].type == WALL) {
                    continue;
                }
                bool upEmpty = y < height - 1 && tiles[x][y + 1].type == EMPTY;
                bool downEmpty = y > 0 && tiles[x][y - 1].type == EMPTY;
                bool leftEmpty = x > 0 && tiles[x - 1][y].type == EMPTY;
                bool rightEmpty = x < width - 1 && tiles[x + 1][y].type == EMPTY;
                //Adjacent
                if(upEmpty) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x][y + 1].node);
                }
                if(downEmpty) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x][y - 1].node);
                }
                if(leftEmpty) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x - 1][y].node);
                }
                if(rightEmpty) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x + 1][y].node);
                }
                //Diagonal
                if(upEmpty && leftEmpty && tiles[x - 1][y + 1].type == EMPTY) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x - 1][y + 1].node);
                }
                if(upEmpty && rightEmpty && tiles[x + 1][y + 1].type == EMPTY) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x + 1][y + 1].node);
                }
                if(downEmpty && leftEmpty && tiles[x - 1][y - 1].type == EMPTY) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x - 1][y - 1].node);
                }
                if(downEmpty && rightEmpty && tiles[x + 1][y - 1].type == EMPTY) {
                    tiles[x][y].node.neighbors.push_back(&tiles[x + 1][y - 1].node);
                }
                
            }
        }
    }

    struct NavMove {
        //Since priority_queue is a max heap, we want to reverse the comparison
        bool operator<(const NavMove & other) const {
            return dist > other.dist;
        }

        NavNode* node;
        float dist;
    };

    point_t navigate(const point_t & start, const point_t & end) {
        float startX = (start.x - bottomLeft.x) / scale;
        float startY = (start.y - bottomLeft.y) / scale;
        float endX = (end.x - bottomLeft.x) / scale;
        float endY = (end.y - bottomLeft.y) / scale;

        if (startX < 0 || startY < 0 || startX >= width || startY >= height) {
            throw std::runtime_error("Start position out of bounds");
        }

        Tile& startTile = tiles[(int)startX][(int)startY];
        Tile& endTile = tiles[(int)endX][(int)endY];

        if (startTile.type == WALL || endTile.type == WALL) {
            std::cout << "Start or end position is a wall. Cannot navigate!" << std::endl;
            return start;
        }

        if(startTile.node.id == endTile.node.id) {
            //Within the same tile, can just go directly there
            return end;
        }

        //Dijkstra's algorithm
        std::map<int, float> dist;
        std::priority_queue<NavMove> queue;
        queue.push({&startTile.node, 0.0f});

        bool found = false;

        //std::cout << "Navigate: forward search" << std::endl;

        while(!queue.empty()) {
            NavMove move = queue.top();
            queue.pop();

            if(dist.find(move.node->id) != dist.end()) {
                //Already visited
                continue;
            }
            dist[move.node->id] = move.dist;
            //std::cout << move.node->pos.x << ", " << move.node->pos.y << ": " << move.dist << std::endl;

            if (move.node->id == endTile.node.id) {
                found = true;
                break;
            }

            for (NavNode* neighbor : move.node->neighbors) {
                float newDist = dist[move.node->id] + math_util::dist(move.node->pos, neighbor->pos);
                if (dist.find(neighbor->id) == dist.end()) {
                    queue.push({neighbor, newDist});
                }
            }
        }

        if (!found) {
            std::cout << "Could not find path!" << std::endl;
            return start;
        }
        else {
            //std::cout << "Navigate: backtrace" << std::endl;
            //Note: this backtrace assumes that being neighbors is reciprocal
            NavNode* current = &endTile.node;
            while (true) {
                //std::cout << current->pos.x << ", " << current->pos.y << std::endl;
                float minDist = std::numeric_limits<float>::max();
                NavNode* next = nullptr;
                for (NavNode* neighbor : current->neighbors) {
                    if (dist.find(neighbor->id) != dist.end() && dist[neighbor->id] < minDist) {
                        minDist = dist[neighbor->id];
                        next = neighbor;
                    }
                }
                if(next == nullptr) {
                    throw std::runtime_error("Navigation backtrace failed! This shouldn't happen!");
                }
                if(next->id == startTile.node.id) {
                    return current->pos;
                }
                current = next;
            }

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
                    tiles[x][height - y - 1].type = WALL;
                }
                else{
                    tiles[x][height - y - 1].type = EMPTY;
                    std::cout << " ";
                }
                x++;
            }
        }
        std::cout << std::endl;
 
        setupNavMesh();
    }

    bool checkVisibility(point_t start, point_t dest) const
    {
        const NavNode* startNode = nodeAt(start);
        const NavNode* destNode = nodeAt(dest);

        if(startNode == nullptr || destNode == nullptr)
        {
            return false;
        }

        if(startNode->id == destNode->id)
        {
            return true;
        }

        float DELTA_SIZE = 1;

        if(math_util::dist(start, dest) < DELTA_SIZE)
        {
            return true;
        }

        point_t delta = math_util::normalize(dest - start) * DELTA_SIZE;

        point_t current = start;
        while(true)
        {
            current += delta;
            if(math_util::dist(start, current) + DELTA_SIZE > math_util::dist(start, dest))
            {
                return true;
            }
            const NavNode* node = nodeAt(current);
            if(node == nullptr)
            {
                return false;
            }
            if(node->id == destNode->id)
            {
                return true;
            }
            if(tileAt(current) == WALL)
            {
                return false;
            }
        }
    }

    bool checkVisibility(point_t start, point_t dest_center, float dest_radius) const
    {
        point_t a = dest_center + point_t(dest_radius, dest_radius);
        point_t b = dest_center + point_t(dest_radius, -dest_radius);
        point_t c = dest_center + point_t(-dest_radius, -dest_radius);
        point_t d = dest_center + point_t(-dest_radius, dest_radius);

        return checkVisibility(start, a) || checkVisibility(start, b) || checkVisibility(start, c) || checkVisibility(start, d);
    }

    TileType tileAt(const point_t & pos) const
    {
        int x = (pos.x - bottomLeft.x) / scale;
        int y = (pos.y - bottomLeft.y) / scale;
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return WALL;
        }
        return tiles[x][y].type;
    }

    const NavNode* nodeAt(const point_t & pos) const
    {
        int x = (pos.x - bottomLeft.x) / scale;
        int y = (pos.y - bottomLeft.y) / scale;
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return nullptr;
        }
        return &tiles[x][y].node;
    }

    point_t bottomLeft;
    size_t width;
    size_t height;
    float scale;

    std::vector<std::vector<Tile>> tiles;
    
};

#endif