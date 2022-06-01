

#ifndef M3_H
#define M3_H

#pragma once
#include "m1.h"
#include "globals.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <string>


// find the travel time for the shortest path between two points on the map
double computePathTravelTime(const double turn_penalty,
const std::vector<StreetSegmentIdx>& path);

// given two points and a turn penalty in seconds, return the shortest path
// between them
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, 
                              const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);

// check if a path between two intersections on the map exists
bool pathExist (int srcID, int destID,const double turn_penalty);

// store the path taken from a start point to the given destination ID in a vector
std::vector<StreetSegmentIdx> bfsTraceBack(int destID);
#endif /* M3_H */

