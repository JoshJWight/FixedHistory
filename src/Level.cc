#include "Level.hh"

Level::Level(int _width, int _height, const point_t & _bottomLeft, float _scale)
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

void Level::setupNavMesh() 
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

void Level::setFromLines(const std::vector<std::string> & lines) {
    if (lines.size() != height) {
        throw std::runtime_error("Level does not fit this string");
    }
    for (int y = 0; y < height; y++) {
        if (lines[y].size() != width) {
            throw std::runtime_error("Level does not fit this string");
        }
        for (int x = 0; x < width; x++) {
            if (lines[y][x] == 'X') {
                tiles[x][height - y - 1].type = WALL;
            }
            else{
                tiles[x][height - y - 1].type = EMPTY;
            }
        }
    }
    setupNavMesh();
}

void Level::setFromString(const std::string & str) {
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



Level::TileType Level::tileAt(const point_t & pos) const
{
    int x = (pos.x - bottomLeft.x) / scale;
    int y = (pos.y - bottomLeft.y) / scale;
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return WALL;
    }
    return tiles[x][y].type;
}

const Level::NavNode* Level::nodeAt(const point_t & pos) const
{
    int x = (pos.x - bottomLeft.x) / scale;
    int y = (pos.y - bottomLeft.y) / scale;
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return nullptr;
    }
    return &tiles[x][y].node;
}

point_t Level::toLevelCoords(const point_t & worldPos) const
{
    return point_t(floor((worldPos.x - bottomLeft.x) / scale), floor((worldPos.y - bottomLeft.y) / scale));
}