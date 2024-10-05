#include "Search.hh"

namespace search{

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

point_t navigate(GameState * state, const point_t & start, const point_t & end) {
    float startX = (start.x - state->level->bottomLeft.x) / state->level->scale;
    float startY = (start.y - state->level->bottomLeft.y) / state->level->scale;
    float endX = (end.x - state->level->bottomLeft.x) / state->level->scale;
    float endY = (end.y - state->level->bottomLeft.y) / state->level->scale;

    if (startX < 0 || startY < 0 || startX >= state->level->width || startY >= state->level->height) {
        throw std::runtime_error("Start position out of bounds");
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

}