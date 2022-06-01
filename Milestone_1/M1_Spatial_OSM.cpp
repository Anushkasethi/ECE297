/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */

#include <iostream>
#include <limits>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "math.h"
#include "globals.h"
#include <string>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <algorithm>

extern M1Global m1Global;

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    double minDistance=std::numeric_limits<double>::max();
    double distance_with_myposition = 0;
    POIIdx poiID=0;
    int poiNum = getNumPointsOfInterest();
    
    // Loop around each point of interest, calculate distance and compare them
    // with the smallest distance
      for(int i=0;i< poiNum; i++){
        std::string poiType = getPOIType(i);
        if (poiType == POItype){
            LatLon poiPosition = getPOIPosition(i);
            std::pair <LatLon,LatLon> coord (poiPosition,my_position);
            distance_with_myposition = findDistanceBetweenTwoPoints(coord);  
            // Find the smallest distance, return the POIid of it 
            if(distance_with_myposition < minDistance){
                minDistance = distance_with_myposition;
                poiID = i;
            }
        }
      }
    
    return poiID;
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to the given position
// Speed Requirement --> none

// loop through all intersections
// calculate distance between LatLon intersection and LatLon my_position
// store min distance and the intersection with min distance

IntersectionIdx findClosestIntersection(LatLon my_position){
    
    double minDistance=std::numeric_limits<double>::max(); //make minDistance = the largest number in the double in the beginning 
    IntersectionIdx intersectionId = 0;
    for(int intersection=0; intersection< getNumIntersections(); ++intersection)
    {
        LatLon intersectionPosition = getIntersectionPosition(intersection);
        
        std::pair <LatLon, LatLon> coordinatesOfIntersections (intersectionPosition, my_position);
        double distance = findDistanceBetweenTwoPoints(coordinatesOfIntersections );
        if(distance<minDistance)
        {
            minDistance = distance;
            intersectionId = intersection;
        }
        
    }
    return intersectionId;
    
}


int findClosestSubwayStation(LatLon my_position){
    
    double minDistance=std::numeric_limits<double>::max(); //make minDistance = the largest number in the double in the beginning 
    int SubwayStationIdx = 0;
    
    for(int station=0; station< m1Global.subwayStationLocations.size(); ++station)
    {        
        double lon = convertXToLon(m1Global.subwayStationLocations[station].x);
        double lat = convertYToLat(m1Global.subwayStationLocations[station].y);
        
        LatLon stationPosition(lat, lon);
        
        std::pair <LatLon, LatLon> coordinatesOfStations (stationPosition, my_position);
        
        double distance = findDistanceBetweenTwoPoints(coordinatesOfStations);
        
        if(distance<minDistance)
        {
            minDistance = distance;
            //SubwayStationName = m1Global.subwayStationLocations[station].second;
            SubwayStationIdx = station;
        }
        
    }
    return SubwayStationIdx;
    
}
