#ifndef __CRIME_HH__
#define __CRIME_HH__

#include <objects/GameObject.hh>

class Crime : public GameObject
{
public:
    //Overall 7x7 grid, largest that can fit in 64-bit int
    const static int SEARCH_RADIUS = 3;
    const static int SEARCH_DIAMETER = 2 * SEARCH_RADIUS + 1;

    const static uint64_t FULLY_SEARCHED = (1UL << (SEARCH_DIAMETER * SEARCH_DIAMETER + 1UL)) - 1UL;

    enum CrimeType
    {
        MURDER,
        TRESPASSING
    };

    Crime(int id)
        : GameObject(id)
        , subjectId(-1)
    {
        colliderType = CIRCLE;
        size = point_t(1, 1);
        setupSprites({"crime.png"});
    }

    Crime(int id, Crime* ancestor)
        : GameObject(id, ancestor)
        , crimeType(ancestor->crimeType)
        , subjectId(ancestor->subjectId)
    {
    }
    
    bool isTransient() override
    {
        return true;
    }

    void submitSearch(int x, int y)
    {
        if(x > SEARCH_RADIUS || x < -SEARCH_RADIUS || y > SEARCH_RADIUS || y < -SEARCH_RADIUS)
        {
            throw std::runtime_error("Crime::submitSearch: input is out of bounds");
        }
        int idx = x * SEARCH_DIAMETER + y;

        nextState.searchStatus |= 1 << idx;
    }
    bool isSearched(int x, int y)
    {
        if(x > SEARCH_RADIUS || x < -SEARCH_RADIUS || y > SEARCH_RADIUS || y < -SEARCH_RADIUS)
        {
            throw std::runtime_error("Crime::isSearched: input is out of bounds");
        }
        int idx = x * SEARCH_DIAMETER + y;

        return state.searchStatus & (1 << idx);
    }

    CrimeType crimeType;
    int subjectId;

};

#endif