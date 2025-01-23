#ifndef __PROMISE_HH__
#define __PROMISE_HH__

class Promise
{

public:

    enum PromiseType
    {
        ABSENCE,
        DEATH
    };

    Promise(int _originTimeline, int _originTick, int _target, PromiseType _type)
        : originTimeline(_originTimeline)
        , activatedTimeline(-1)
        , originTick(_originTick)
        , target(_target)
        , type(_type)
    {
    }

    //Timeline where the promise was made
    int originTimeline;
    //Timeline where the player traveled back past the promise origin
    int activatedTimeline;
    //The promise is in force starting from this tick
    int originTick;

    int target;
    PromiseType type;
};

#endif