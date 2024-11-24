#include "Search.hh"

namespace search{

VisibilityGrid createVisibilityGrid(GameState * state, point_t center)
{
    //Initialize grid to false
    VisibilityGrid grid(state->level->width, std::vector<bool>(state->level->height, false));

    VisibilityGrid levelGrid = VisibilityGrid(state->level->width, std::vector<bool>(state->level->height, false));
    for(int x = 0; x < state->level->width; x++)
    {
        for(int y = 0; y < state->level->height; y++)
        {
            levelGrid[x][y] = state->level->tiles[x][y].type == Level::WALL;
        }
    }
    for(auto pair: state->objects)
    {
        GameObject * obj = pair.second.get();
        if(obj->isObstruction())
        {
            point_t levelCoords = state->level->toLevelCoords(obj->state.pos);
            levelGrid[levelCoords.x][levelCoords.y] = true;
        }
    }

    const int N_RAYCASTS = 100;
    const float DISTANCE_LIMIT = 10000;

    for(int i = 0; i < N_RAYCASTS; i++)
    {
        float angle = i * 2 * M_PI / N_RAYCASTS;
        point_t delta = point_t(cos(angle), sin(angle));
        point_t current = center;
        int n = 0;
        while(math_util::dist(center, current) < DISTANCE_LIMIT)
        {
            n++;
            current += delta;
            point_t levelCoords = state->level->toLevelCoords(current);
            if(levelCoords.x < 0 || levelCoords.x >= state->level->width || levelCoords.y < 0 || levelCoords.y >= state->level->height)
            {
                std::cout << "Raycast out of bounds!" << std::endl;
                break;
            }
            grid[levelCoords.x][levelCoords.y] = true;
            if(levelGrid[levelCoords.x][levelCoords.y])
            {
                //Hit obstruction/wall
                break;
            }
        }
    }

    //Set any wall adjacent or diagonal to a visible floor tile to visible
    for(int x = 0; x < state->level->width; x++)
    {
        for(int y = 0; y < state->level->height; y++)
        {
            if(grid[x][y] && !levelGrid[x][y])
            {
                for(int dx = -1; dx <= 1; dx++)
                {
                    for(int dy = -1; dy <= 1; dy++)
                    {
                        if(dx == 0 && dy == 0)
                        {
                            continue;
                        }
                        int nx = x + dx;
                        int ny = y + dy;
                        if(nx < 0 || nx >= state->level->width || ny < 0 || ny >= state->level->height)
                        {
                            continue;
                        }
                        if(levelGrid[nx][ny])
                        {
                            grid[nx][ny] = true;
                        }
                    }
                }
            }
        }
    }

    return grid;
}

bool checkVisibility(GameState * state, point_t start, point_t dest)
{
    const Level::NavNode* startNode = state->level->nodeAt(start);
    const Level::NavNode* destNode = state->level->nodeAt(dest);

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
        //TODO the performance of this seems bad, improve later
        for(const auto & objpair: state->objects)
        {
            if(objpair.second->isObstruction() && objpair.second->isColliding(current))
            {
                return false;
            }
        }
        const Level::NavNode* node = state->level->nodeAt(current);
        if(node == nullptr)
        {
            return false;
        }
        if(node->id == destNode->id)
        {
            return true;
        }
        if(state->level->tileAt(current) == Level::WALL)
        {
            return false;
        }
    }
}

bool checkVisibility(GameState * state, point_t start, point_t dest_center, float dest_radius)
{
    point_t a = dest_center + point_t(dest_radius, dest_radius);
    point_t b = dest_center + point_t(dest_radius, -dest_radius);
    point_t c = dest_center + point_t(-dest_radius, -dest_radius);
    point_t d = dest_center + point_t(-dest_radius, dest_radius);

    return checkVisibility(state, start, a) || checkVisibility(state, start, b) || checkVisibility(state, start, c) || checkVisibility(state, start, d);
}

bool checkVisibility(GameState * state, point_t start, float start_radius, point_t dest_center, float dest_radius)
{
    
    float angle = math_util::angleBetween(start, dest_center);
    point_t orthogonalVector1 = math_util::normalize(point_t(cos((angle * M_PI/180.0) + M_PI_2), sin((angle * M_PI/180.0) + M_PI_2)));
    point_t orthogonalVector2 = math_util::normalize(point_t(cos((angle * M_PI/180.0) - M_PI_2), sin((angle * M_PI/180.0) - M_PI_2)));

    bool result = false;
    //Make three checks, one between the centers and two along tangent lines
    result |= checkVisibility(state, start, dest_center);
    result |= checkVisibility(state, start + (orthogonalVector1 * start_radius), dest_center + (orthogonalVector1 * dest_radius));
    result |= checkVisibility(state, start + (orthogonalVector2 * start_radius), dest_center + (orthogonalVector2 * dest_radius));

    return result;
}

point_t navigate(GameState * state, const point_t & start, const point_t & end) {
    float startX = (start.x - state->level->bottomLeft.x) / state->level->scale;
    float startY = (start.y - state->level->bottomLeft.y) / state->level->scale;
    float endX = (end.x - state->level->bottomLeft.x) / state->level->scale;
    float endY = (end.y - state->level->bottomLeft.y) / state->level->scale;

    if (startX < 0 || startY < 0 || startX >= state->level->width || startY >= state->level->height) {
        throw std::runtime_error("Start position out of bounds");
    }
    if (endX < 0 || endY < 0 || endX >= state->level->width || endY >= state->level->height) {
        std::cout << "navigate: End position out of bounds, doing nothing instead." << std::endl;
        return start;
    }

    Level::Tile& startTile = state->level->tiles[(int)startX][(int)startY];
    Level::Tile& endTile = state->level->tiles[(int)endX][(int)endY];

    if (startTile.type == Level::WALL || endTile.type == Level::WALL) {
        std::cout << "Start or end position is a wall. Cannot navigate!" << std::endl;
        return start;
    }

    if(startTile.node.id == endTile.node.id) {
        //Within the same tile, can just go directly there
        return end;
    }

    //Dijkstra's algorithm
    std::map<int, float> dist;
    std::priority_queue<Level::NavMove> queue;
    queue.push({&startTile.node, 0.0f});

    bool found = false;

    //std::cout << "Navigate: forward search" << std::endl;
    int iterations = 0;
    while(!queue.empty()) {
        Level::NavMove move = queue.top();
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

        for (Level::NavNode* neighbor : move.node->neighbors) {
            float newDist = dist[move.node->id] + math_util::dist(move.node->pos, neighbor->pos);
            if (dist.find(neighbor->id) == dist.end()) {
                queue.push({neighbor, newDist});
            }
        }
        iterations++;
        if(iterations > 1000000){
            std::cout << "Navigate: too many iterations on forward pass!" << std::endl;
            return start;
        }
    }

    if (!found) {
        std::cout << "Could not find path!" << std::endl;
        return start;
    }
    else {
        //std::cout << "Navigate: backtrace" << std::endl;
        //Note: this backtrace assumes that being neighbors is reciprocal
        Level::NavNode* current = &endTile.node;
        while (true) {
            //std::cout << current->pos.x << ", " << current->pos.y << std::endl;
            float minDist = std::numeric_limits<float>::max();
            Level::NavNode* next = nullptr;
            for (Level::NavNode* neighbor : current->neighbors) {
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

            iterations++;
            if(iterations > 1000000){
                std::cout << "Navigate: too many iterations on backtrace!" << std::endl;
                return start;
            }
        }

    }

}

float bounceOffWall(GameState * state, const point_t & startPoint, const point_t & obstructedPoint)
{
    point_t startTilePos = state->level->nodeAt(startPoint)->pos;
    point_t obstructedTilePos = state->level->nodeAt(obstructedPoint)->pos;

    if(startTilePos == obstructedTilePos)
    {
        std::cout << "Bounce off wall: start and obstructed points are in the same tile!" << std::endl;
        return math_util::angleBetween(startPoint, obstructedPoint);
    }

    float incomingAngle = math_util::angleBetween(startPoint, obstructedPoint);
    float normalAngle = math_util::angleBetween(obstructedTilePos, startTilePos);

    float outgoingAngle = 2 * normalAngle - incomingAngle + 180;
    while(outgoingAngle < -180)
    {
        outgoingAngle += 360;
    }
    while(outgoingAngle > 180)
    {
        outgoingAngle -= 360;
    }

    return outgoingAngle;
}

}