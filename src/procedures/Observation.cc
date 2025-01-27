#include "Observation.hh"

namespace observation{

void recordObservations(GameState * state, Player * player, int tick)
{
    if(player->recorded)
    {
        throw std::runtime_error("Shouldn't be recording observations for a recorded player!");
    }

    if(player->observations.size() < tick)
    {
        throw std::runtime_error("Player observation buffer is smaller than expected!");
    }
    if(player->observations.size() == tick)
    {
        player->observations.push_back(Player::ObservationFrame());
    }
    Player::ObservationFrame & frame = player->observations[tick];
    frame.clear();

    //Player can't see anything if they're in a box
    if(player->state.boxOccupied)
    {
        return;
    }

    for(auto & objpair : state->objects())
    {
        GameObject * obj = objpair.second.get();
        if(!obj->activeAt(tick))
        {
            continue;
        }
        if(obj->id == player->id)
        {
            continue;
        }
        if(obj->state.visible == false)
        {
            continue;
        }

        if(obj->isAlwaysDrawn())
        {
            continue;
        }

        if(search::checkVisibility(state, player->state.pos, player->radius(), obj->state.pos, obj->radius()))
        {
            frame.push_back({obj->type(), obj->state, obj->id});
        }
    }

    //std::cout << "Recorded " << frame.size() << " observations for player " << player->id << " at tick " << tick << std::endl;
}

bool observablyEqual(const ObjectState & a, const ObjectState & b)
{
    bool result = true;
    result &= a.pos == b.pos;
    result &= a.angle_deg == b.angle_deg;
    result &= a.animIdx == b.animIdx;

    return result;
}

void printObservations(int playerID, const Player::ObservationFrame & frame, Player::ObservationFrame & objects, int tick)
{
    std::cout << "Failed observation check by player " << playerID << " on tick " << tick << std::endl;
    std::cout << "Expected:" << std::endl;
    for(auto & obj : frame)
    {
        std::cout << GameObject::typeToString(obj.type) << " at " << obj.state.pos.x << ", " << obj.state.pos.y 
            << " (originally id " << obj.id << ")" << std::endl;
    }
    std::cout << "Actual:" << std::endl;
    for(auto & obj : objects)
    {
        std::cout << GameObject::typeToString(obj.type) << " at " << obj.state.pos.x << ", " << obj.state.pos.y 
            << " (id " << obj.id << ")" << std::endl;
    }
    std::cout << std::endl;
}

std::string checkObservations(GameState * state, Player * player, int tick)
{
    if(player->state.boxOccupied)
    {
        return "";
    }

    std::string result = "";
    if(player->observations.size() <= tick)
    {
        throw std::runtime_error("Tried to check observations player " + std::to_string(player->id) + 
            " at tick " + std::to_string(tick) + " but observation buffer was only " + std::to_string(player->observations.size()) + " long");
    }

    const Player::ObservationFrame & frame = player->observations[tick];
    Player::ObservationFrame actual;

    std::map<int, bool> found;
    for(auto & objpair : state->objects())
    {
        GameObject * obj = objpair.second.get();
        if(!obj->activeAt(tick))
        {
            continue;
        }
        if(obj->id == player->id)
        {
            continue;
        }
        if(obj->state.visible == false)
        {
            continue;
        }

        if(obj->isAlwaysDrawn())
        {
            continue;
        }

        if(search::checkVisibility(state, player->state.pos, player->radius(), obj->state.pos, obj->radius()))
        {
            actual.push_back({obj->type(), obj->state, obj->id});

            bool matched = false;
            for(int i=0; i<frame.size(); i++)
            {
                if(frame[i].type == obj->type() && observablyEqual(frame[i].state, obj->state))
                {
                    found[i] = true;
                    matched = true;
                    break;
                }
            }
            //Temporary until we have better stealth options
            if(obj->type() == GameObject::PLAYER)
            {
                continue;
            }

            if(!matched)
            {
                result = "A past you saw an unexpected " + GameObject::typeToString(obj->type());
            }
        }
    }
    
    for(int i=0; i<frame.size(); i++)
    {
        if(frame[i].type == GameObject::PLAYER)
        {
            continue;
        }
        if(found.find(i) == found.end())
        {
            result = "A past you missed a " + GameObject::typeToString(frame[i].type);
        }
    }

    if(result != "")
    {
        printObservations(player->id, frame, actual, tick);
    }

    return result;
}

}//namespace observation