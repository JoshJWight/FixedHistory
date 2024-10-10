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
    Player::ObservationFrame & frame = player->observations.back();
    frame.clear();

    for(auto & objpair : state->objects)
    {
        GameObject * obj = objpair.second.get();
        if(obj->id == player->id)
        {
            continue;
        }

        if(search::checkVisibility(state, player->state.pos, obj->state.pos, obj->radius()))
        {
            frame.push_back({obj->type(), obj->state});
        }
    }
}

bool observablyEqual(const ObjectState & a, const ObjectState & b)
{
    bool result = true;
    result &= a.pos == b.pos;
    result &= a.angle_deg == b.angle_deg;
    result &= a.animIdx == b.animIdx;

    return result;
}

std::string checkObservations(GameState * state, Player * player, int tick)
{
    if(player->observations.size() <= tick)
    {
        throw std::runtime_error("Tried to check observations for a tick that hasn't been recorded!");
    }

    const Player::ObservationFrame & frame = player->observations[tick];

    std::map<int, bool> found;
    for(auto & objpair : state->objects)
    {
        GameObject * obj = objpair.second.get();
        if(obj->id == player->id)
        {
            continue;
        }

        if(search::checkVisibility(state, player->state.pos, obj->state.pos, obj->radius()))
        {
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
            if(!matched)
            {
                return "A past you saw an unexpected " + GameObject::typeToString(obj->type());
            }
        }
    }
    
    for(int i=0; i<frame.size(); i++)
    {
        if(found.find(i) == found.end())
        {
            return "A past you missed a " + GameObject::typeToString(frame[i].type);
        }
    }

    //No issues found, return empty string
    return "";
}

}//namespace observation