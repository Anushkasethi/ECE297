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



IntersectionIdx findClosestLegalDepot(
        const float turn_penalty,
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots) {
    
    IntersectionIdx bestDepot = 0;
    int shortestDistance = -1;
    int currentDistance = 0;
        
    for(int i = 0; i < depots.size(); i++){
        IntersectionIdx depotStart = depots[i];
        
        // if a depot can access one pickup, it can access all pickups
        // if a depot cannot access a pickup, it is not valid for any pickup
        for(int j = 0; j < deliveries.size(); j++){
            IntersectionIdx curPickup = deliveries[i].pickUp;
            
            std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(turn_penalty, 
                              std::make_pair(depotStart, curPickup));
            
            // not accessible, so not a valid depot
            if(path.size() == 0 && depotStart != curPickup){
                shortestDistance = -1;
                break;
            
            }else{
                currentDistance = findDistanceBetweenTwoPoints(std::make_pair(
                        getIntersectionPosition(depotStart),
                        getIntersectionPosition(curPickup)));
                
                if(currentDistance < shortestDistance || shortestDistance == -1){
                    shortestDistance = currentDistance;
                    bestDepot = depotStart;
                }
            }
        }
        
        if(shortestDistance == -1) continue;
    }
    return bestDepot;
}


std::vector<CourierSubPath> travelingCourier(
        const float turn_penalty,
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots) {
    
//    std::cout << "DELIVERIES: " << deliveries.size() << " || DEPOTS: " << depots.size() << std::endl;
    
    // do the dumb way around extreme toronto
//    if(deliveries.size() >= 175 && depots.size() >= 20){
//        return travelingCourierTABad(turn_penalty, deliveries, depots);
//    }

    std::vector<CourierSubPath> solution;
    
    IntersectionIdx depotStart;

//    IntersectionIdx depotStart = findClosestLegalDepot(turn_penalty, deliveries, depots);
    
    if(deliveries.size() >= 175 && depots.size() >= 20){
        depotStart = findFirstLegalDepot(turn_penalty, deliveries, depots);
    }
//    
    else depotStart = findClosestLegalDepot(turn_penalty, deliveries, depots);

    std::vector<std::pair<int, int>> legalPickupDropoffs;

    // initialize legal locations
    for (int i = 0; i < deliveries.size(); i++) {
        legalPickupDropoffs.push_back({0, 0});
    }

    int deliveryCount = 0;

    IntersectionIdx currentLocation = depotStart;
    
    CourierSubPath subPath;
    
    int shortestDistance;
    int currentDistance;

    // continue while there are still deliveries to be made
    // n pickups + n dropoffs = 2n
    while (deliveryCount < deliveries.size() * 2) {
        LatLon currentLatLon = getIntersectionPosition(currentLocation);
        
        shortestDistance = -1;
        currentDistance = 0;
        
        IntersectionIdx nextLocation = 0;
        int nextLocationIndex = 0;
        int nextIsPickup = 1;
        
        IntersectionIdx tempLocation;
        int tempIsPickup;


        // find closest next location, either pickup or dropoff
        for (int i = 0; i < deliveries.size(); i++) {
            // ignore if the package has been picked up and delivered
            if (legalPickupDropoffs[i].first == 1 && legalPickupDropoffs[i].second == 1) {
                continue;
            }
            
            // package has not been picked up
            else if (legalPickupDropoffs[i].first == 0 && legalPickupDropoffs[i].second == 0) {
                tempLocation = deliveries[i].pickUp;
                tempIsPickup = 1;
            }
            
            // package has been picked up but not dropped off
            else if (legalPickupDropoffs[i].first == 1 && legalPickupDropoffs[i].second == 0) {
                tempLocation = deliveries[i].dropOff;
                tempIsPickup = 0;
            }
            
            // package has been dropped off but not picked up -- ERROR
            else {
                std::cout << "You dropped off an imaginary package!";
                continue;
            }
            
            if(deliveryCount == 199 && i == 99){
                std::cout << tempLocation << " || " << tempIsPickup << std::endl;
            }

            currentDistance = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon,
                    getIntersectionPosition(tempLocation)));
            
            if(deliveryCount == 199 && i == 99) std::cout << currentDistance << std::endl;
            
            if(currentDistance < shortestDistance || shortestDistance == -1){
                shortestDistance = currentDistance;
                nextLocation = tempLocation;
                nextLocationIndex = i;
                nextIsPickup = tempIsPickup;
            }
        }
        // add next location to the path
        subPath.start_intersection = currentLocation;
        subPath.end_intersection = nextLocation;
        subPath.subpath = findPathBetweenIntersections(turn_penalty,
                std::make_pair(currentLocation, nextLocation));
        
        solution.push_back(subPath);
                        
        // increment delivery count, mark location as explored, reset current location
        deliveryCount++;
        
        if(nextIsPickup == 1){
            legalPickupDropoffs[nextLocationIndex].first = 1;
            
        }else{
            legalPickupDropoffs[nextLocationIndex].second = 1;
        }
        currentLocation = nextLocation;
    }
    
    shortestDistance = -1;
    currentDistance = 0;
    IntersectionIdx depotEnd = 0;
    LatLon currentLatLon = getIntersectionPosition(currentLocation);
        
    // find closest depot to return to
    for(int i = 0; i < depots.size(); i++){
        currentDistance = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon, 
                getIntersectionPosition(depots[i])));
        
        if(currentDistance < shortestDistance || shortestDistance == -1){
            shortestDistance = currentDistance;
            depotEnd = depots[i];
        }
    }
    
    subPath.start_intersection = currentLocation;
    subPath.end_intersection = depotEnd;
    subPath.subpath = findPathBetweenIntersections(turn_penalty,
                std::make_pair(currentLocation, depotEnd));
    
    
    solution.push_back(subPath);
    
            
    return solution;
}


/*
 * Simple Toronto: Max deliveries 9, Max depots 3
 * Easy Toronto: Deliveries 25, Depots 2
 * Easy New York: Deliveries 25, Depots 4
 * Easy London: Deliveries 7, Depots 1
 * Medium Toronto: Deliveries 50, Depots 5
 * Medium New York: Deliveries 50, Depots 10
 * Medium London: Deliveries 14, Depots 20
 * Hard Toronto: Deliveries 100, Depots 10
 * Hard Multi Toronto: Deliveries 100, Depots 3
 * Hard New York: Deliveries 75, Depots 4
 * Hard Multi New York: Deliveries 75, Depots 7
 * Hard London: Deliveries 44, Depots 1
 * Hard Multi London: Deliveries 44, Depots 2
 * 
 * Extreme Toronto: Deliveries 175, Depots 20
 * 
 * Extreme Multi Toronto: Deliveries 260, Depots 5
 * Extreme New York: Deliveries 128, Depots 1
 * Extreme Multi New York: Deliveries 192, Depots 2
 * Extreme London: Deliveries 65, Depots 1
 * Extreme Multi London: Deliveries 65, Depots 1
 */