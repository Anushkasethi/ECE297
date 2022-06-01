/* 
 * File:   AnushkaM1Functions.cpp
 * Author: sethianu
 *
 * Created on February 1, 2022, 11:42 PM
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


// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high

//perform all of this by loading a data structure to the load map and then 
//calling the function (one line of code)to make the speed higher!

//The data structure intersectionStreetSegments is a 2D vector which holds all the intersectionID's 
//and each intersectionID further holds the streetSegment ID's.
//Hence calling that data structure intersectionStreetSegments[intersection_id] returns all the streetSegment ID's
//which are connected to that particular intersectionID.
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    return m1Global.intersectionStreetSegments[intersection_id];
}

// Returns the street names at the given intersection (includes duplicate 
// street names in the returned vector)
// Speed Requirement --> high 

//Access all the streetSegment ID's connected to a particular intersection, and traverse through all the streetSegments,
//use the struct StreetSegmentInfo to get all the street ID's for all the streetSegments and then
//using the getStreetName function we obtain all the names of the streetSegments.
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id) {
    
    std::vector<StreetSegmentIdx> ss = m1Global.intersectionStreetSegments[intersection_id]; //holds the streetSegment ID's for one intersection
    std::vector<std::string> streetNames;
    int streetSegments = getNumIntersectionStreetSegment(intersection_id);
    
    for (int i = 0; i < streetSegments; i++) {
        
        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(ss[i]);
        std::string streetN = getStreetName(streetSegInfo.streetID); 
        streetNames.push_back(streetN);
    }
    
    return streetNames;
}

// Returns true if the two intersections are directly connected, meaning you can legally 
// drive from the first intersection to the second using only one streetSegment.
// Speed Requirement --> moderate

 // Do an if statement get the StreetSegId's for the 2 intersections and then check 
//if one streetsegId is common then intersections are directly connected
bool intersectionsAreDirectlyConnected(std::pair<IntersectionIdx, IntersectionIdx> intersection_ids) {
    // if it's the same intersection, return true
    if(intersection_ids.first == intersection_ids.second) return true;
   
    std::vector<StreetSegmentIdx>streetSegmentsOfIntersection1 = m1Global.intersectionStreetSegments[intersection_ids.first];
    std::vector<StreetSegmentIdx>streetSegmentsOfIntersection2 = m1Global.intersectionStreetSegments[intersection_ids.second];
    
    for(int i = 0; i < streetSegmentsOfIntersection1.size(); i++)
    {
        for(int j = 0; j < streetSegmentsOfIntersection2.size(); j++){
            if(streetSegmentsOfIntersection1[i] == streetSegmentsOfIntersection2[j]){
                // check if it's a one way street and the intersections are oriented
                StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegmentsOfIntersection1[i]);
                
                if(segInfo.oneWay){
                    if(segInfo.from == intersection_ids.first && segInfo.to == intersection_ids.second) return true;
                    else return false;
                }
                
                // otherwise, they are directly connected regardless
                return true;
            }
        }
//        //finding if any streetSegment of intersection 1 is common in the streetSegments of intersection 2
//        if(std::find(streetSegmentsOfIntersection2.begin(), streetSegmentsOfIntersection2.end(), streetSegmentsOfIntersection1[i])!=streetSegmentsOfIntersection2.end())
//        {
//            // for oneways, check if it's a one way street and if the intersections are oriented correctly
//            StreetSegmentIdx commonSeg = *(std::find(streetSegmentsOfIntersection2.begin(), streetSegmentsOfIntersection2.end(), streetSegmentsOfIntersection1[i]));
//            StreetSegmentInfo segInfo = getStreetSegmentInfo(commonSeg);
//            
//            if(segInfo.oneWay){
//                if(segInfo.from == intersection_ids.first && segInfo.to == intersection_ids.second) return true;
//                else return false;
//            }
//            
//            return true;
//        }
    }        
    return false;
}




