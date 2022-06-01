/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZoeM1Functions.cpp
 * Author: xuyuche6
 *
 * Created on February 2, 2022, 12:05 AM
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

// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    std::unordered_set<IntersectionIdx> intersectionsOfStreet = m1Global.streetIntersections[street_id];
    
    // copy into vector
    std::vector<IntersectionIdx> result (intersectionsOfStreet.begin(), intersectionsOfStreet.end());
    
    return result;
}



// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){
    StreetIdx streetOne = street_ids.first;
    StreetIdx streetTwo = street_ids.second;  
  
    // creates by reference to avoid making deep copies and messing up my speed tests
    std::unordered_set<IntersectionIdx> &streetOneIntersections = m1Global.streetIntersections[streetOne];
    std::unordered_set<IntersectionIdx> &streetTwoIntersections = m1Global.streetIntersections[streetTwo];
    
    std::vector<IntersectionIdx> commonIntersections;
    
    // find common elements and store in commonInteresctions
    for(auto streetOneIntersection = streetOneIntersections.begin(); 
            streetOneIntersection != streetOneIntersections.end(); streetOneIntersection++){
        
        if(streetTwoIntersections.find(*streetOneIntersection) != streetTwoIntersections.end()){
            commonIntersections.push_back(*streetOneIntersection);
        }
    }    
 
    return commonIntersections;
}


// Returns all street ids corresponding to street names that start with the 
// given prefix 
// The function should be case-insensitive to the street prefix. 
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to 
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty 
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty 
// (length 0) string, but your program must not crash if street_prefix is a 
// length 0 string.
// Speed Requirement --> high 
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> streetIDs;
    
    // return empty vector for an empty string
    if(street_prefix.empty()){
        return streetIDs;
    }

    // MULTIMAP
    //   Lower bound(partial street name) for starting point
    //   String manipulate last char + 1 to get ending point
    //   Loop through one by one and add to vector

    // remove spaces from street_prefix
    street_prefix.erase(std::remove(street_prefix.begin(), street_prefix.end(), ' '), street_prefix.end());
    
    // convert street_prefix to lowercase
    boost::algorithm::to_lower(street_prefix);
    
    // to set upper bound, change last character to next letter in ASCII
    // take advantage of ASCII character numbering
    std::string upperBoundName = street_prefix;
    upperBoundName.push_back(127);

    std::multimap<std::string, StreetIdx>::iterator itLower = m1Global.streetNames.lower_bound(street_prefix);
    std::multimap<std::string, StreetIdx>::iterator itUpper = m1Global.streetNames.upper_bound(upperBoundName);
    
    // add all streetIDs in the range between names
    for(std::multimap<std::string, StreetIdx>::iterator it = itLower; it != itUpper; it++){
        streetIDs.push_back((*it).second);
    }

    return streetIDs;
}


