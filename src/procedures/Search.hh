#ifndef __SEARCH_HH__
#define __SEARCH_HH__

#include <state/GameState.hh>
#include <state/Level.hh>
#include <utils/MathUtil.hh>

namespace search {

const float UNLIMITED_DISTANCE = 1e12;

VisibilityGrid createVisibilityGrid(GameState * state, point_t center, float startAngle_deg, float endAngle_deg, float distanceLimit);

VisibilityGrid playerVisibilityGrid(GameState * state, Player * player);

VisibilityGrid createObstructionGrid(GameState * state);

bool checkObstruction(GameState * state, point_t pos);

bool checkVisibility(GameState * state, point_t start, point_t dest);

bool checkVisibility(GameState * state, point_t start, point_t dest_center, float dest_radius);

bool checkVisibility(GameState * state, point_t start, float start_radius, point_t dest_center, float dest_radius);

point_t navigate(GameState * state, const point_t & start, const point_t & end, float maxDistance = UNLIMITED_DISTANCE);

float bounceOffWall(GameState * state, const point_t & startPoint, const point_t & obstructedPoint);
}


#endif