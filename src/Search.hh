#ifndef __SEARCH_HH__
#define __SEARCH_HH__

#include "GameState.hh"
#include "Level.hh"
#include "MathUtil.hh"

namespace search {

typedef std::vector<std::vector<bool>> VisibilityGrid;

VisibilityGrid createVisibilityGrid(GameState * state, point_t center);

bool checkVisibility(GameState * state, point_t start, point_t dest);

bool checkVisibility(GameState * state, point_t start, point_t dest_center, float dest_radius);

bool checkVisibility(GameState * state, point_t start, float start_radius, point_t dest_center, float dest_radius);

point_t navigate(GameState * state, const point_t & start, const point_t & end);

float bounceOffWall(GameState * state, const point_t & startPoint, const point_t & obstructedPoint);
}


#endif