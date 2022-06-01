/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */


#include "m1.h"
#include "globals.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <string>
#include "m4.h"
#include "m3.h"



IntersectionIdx findFirstLegalDepot(
        const float turn_penalty,
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots) {
    
    IntersectionIdx bestDepot = 0;
    
    bool foundDepot = false;
        
    for(int i = 0; i < depots.size(); i++){
        IntersectionIdx depotStart = depots[i];
        
        // if a depot can access one pickup, it can access all pickups
        // if a depot cannot access a pickup, it is not valid for any pickup
        for(int j = 0; j < deliveries.size(); j++){
            IntersectionIdx curPickup = deliveries[i].pickUp;
            
            std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(turn_penalty, 
                              std::make_pair(depotStart, curPickup));
            
            // accessible, so valid
            if(path.size() != 0){
                foundDepot = true;
                bestDepot = depotStart;
                break;
            }
        }
        if(foundDepot == true) break;
    }
    return bestDepot;
}


std::vector<CourierSubPath> travelingCourierTABad(
        const float turn_penalty,
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots) {

    std::vector<CourierSubPath> solution;

    IntersectionIdx depotStart = findFirstLegalDepot(turn_penalty, deliveries, depots);

    IntersectionIdx currentLocation = depotStart;
    
    CourierSubPath subPath;

    // continue while there are still deliveries to be made
    for(int i = 0; i < deliveries.size(); i++){
        // go to pickup
        subPath.start_intersection = currentLocation;
        subPath.end_intersection = deliveries[i].pickUp;
        subPath.subpath = findPathBetweenIntersections(turn_penalty, std::make_pair
                (currentLocation, deliveries[i].pickUp));
        
        solution.push_back(subPath);
        
        // go to dropoff
        subPath.start_intersection = deliveries[i].pickUp;
        subPath.end_intersection = deliveries[i].dropOff;
        subPath.subpath = findPathBetweenIntersections(turn_penalty, std::make_pair
                (deliveries[i].pickUp, deliveries[i].dropOff));
        
        solution.push_back(subPath);
        
        // set new current location
        currentLocation = deliveries[i].dropOff;
    }
    
    // end at the same depot we started at
    subPath.start_intersection = currentLocation;
    subPath.end_intersection = depotStart;
    subPath.subpath = findPathBetweenIntersections(turn_penalty,
                std::make_pair(currentLocation, depotStart));
    
    solution.push_back(subPath);
    
    return solution;
}
