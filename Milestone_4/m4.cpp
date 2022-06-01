///*
// * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
// * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
// */
//#include "m1.h"
//#include "globals.h"
//#include "StreetsDatabaseAPI.h"
//#include <vector>
//#include <string>
//#include "m4.h"
//#include "m3.h"
//
//std::vector<CourierSubPath> travelingCourier(
//        const float turn_penalty,
//        const std::vector<DeliveryInf>& deliveries,
//        const std::vector<IntersectionIdx>& depots) {
//
//    std::vector<CourierSubPath> solution;
//    // every depot -> has courier subpaths -> between depots/pickups/dropoffs
//    std::vector<std::vector < CourierSubPath>> solutionPath;
//    std::vector<StreetSegmentIdx> path;
//
//    int depotId = 0;
//    int deliveryId = 0;
//    int toId = 0;
//    int currentId = 0;
//    int current = 0;
//    int count = 0;
//    int start = 0;
//    int end = 0;
//
//    LatLon deliveryLatLon;
//    LatLon deliveryLatLon1;
//    LatLon depotLatLon;
//    LatLon currentLatLon;
//
//    double shortestDistance = -1;
//    double shortestTotalDistance = -1;
//    double totalDistance = 0;
//    double currentDistance = 0;
//    double currentDistance1 = 0;
//
//    std::vector < std::pair<bool, bool>> explored(deliveries.size(), std::make_pair(false, false));
//
//    //Go over all the depots
//    for (int i = 0; i < depots.size(); i++) {
//        depotId = depots[i];
//        currentId = depotId;
//        currentLatLon = getIntersectionPosition(currentId);
//
//        //Go over all the pickUp to find the closest one
//        for (int j = 0; j < deliveries.size(); j++) {
//            deliveryId = deliveries[j].pickUp;
//            //deliveryId1 = deliveries[j].dropOff;
//
//            LatLon deliveryLatLon = getIntersectionPosition(deliveryId);
//            //LatLon deliveryLatLon1 = getIntersectionPosition(deliveryId1);
//            currentDistance = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon, deliveryLatLon));
//
//            if (currentDistance < shortestDistance || shortestDistance < 0) {
//                shortestDistance = currentDistance;
//                //currentId = deliveryId;
//                current = j;
//                toId = deliveryId;
//            }
//        }
//        totalDistance += shortestDistance;
//        shortestDistance = -1;
//        explored[current].first = true;
//
//        // add subpath to the depot's total path
//        CourierSubPath subPath;
//        subPath.start_intersection = currentId;
//        subPath.end_intersection = toId;
//        subPath.subpath = findPathBetweenIntersections(turn_penalty,
//                std::make_pair(currentId, toId)); //the path btw the first depot and the closest deliverIntersection   
//        solutionPath[i].push_back(subPath);
//
//
//        currentId = toId;
//        currentLatLon = getIntersectionPosition(currentId);
//
//
//        while (count != deliveries.size()) {
//            for (int j = 0; j < deliveries.size(); j++) {
//                if (explored[j].first && explored[j].second) {
//                    break;
//                }
//
//                deliveryId = deliveries[j].pickUp;
//                deliveryLatLon = getIntersectionPosition(deliveries[j].pickUp);
//                deliveryLatLon1 = getIntersectionPosition(deliveries[j].dropOff);
//                currentDistance = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon, deliveryLatLon));
//                currentDistance1 = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon, deliveryLatLon1));
//                //Check if the dropOff is valid and compare with pickUp
//                if (explored[j].first) {
//                    currentDistance = currentDistance1;
//                    deliveryId = deliveries[j].dropOff;
//                }
//                //Compare the distance with shortest distance and update the to id and shortest distance
//                if (currentDistance < shortestDistance || shortestDistance < 0) {
//                    toId = deliveryId;
//                    current = j;
//                    shortestDistance = currentDistance;
//                }
//            }
//
//            totalDistance += shortestDistance;
//            shortestDistance = -1;
//
//            // add subpath to the depot's total path
//            CourierSubPath subPath;
//            subPath.start_intersection = currentId;
//            subPath.end_intersection = toId;
//            subPath.subpath = findPathBetweenIntersections(turn_penalty,
//                    std::make_pair(currentId, toId));
//            solutionPath[i].push_back(subPath);
//
//            currentId = toId;
//            currentLatLon = getIntersectionPosition(currentId);
//
//            //Define the intersection as explored
//            if (currentId == deliveries[current].pickUp) {
//                explored[current].first = true;
//            } else if (currentId == deliveries[current].dropOff) {
//                explored[current].second = true;
//            }
//
//            //if both of the pickUp and dropOff are explored, then add one to count
//            if (explored[current].first && explored[current].second) {
//                count++;
//            }
//
//        }
//
//        //Find the closest depot
//        for (int n = 0; n < depots.size(); n++) {
//
//            depotLatLon = getIntersectionPosition(depots[n]);
//            currentDistance = findDistanceBetweenTwoPoints(std::make_pair(currentLatLon, depotLatLon));
//            if (currentDistance < shortestDistance || shortestDistance < 0) {
//                toId = depots[n];
//                current = n;
//                shortestDistance = currentDistance;
//            }
//
//        }
//        end = current;
//        totalDistance += shortestDistance;
//
//        // add subpath to the depot's total path
//        subPath.start_intersection = currentId;
//        subPath.end_intersection = toId;
//        subPath.subpath = findPathBetweenIntersections(turn_penalty,
//                std::make_pair(currentId, toId));
//        solutionPath[i].push_back(subPath);
//
//
//        if (totalDistance < shortestTotalDistance || shortestTotalDistance < 0) {
//            start = i;
//            shortestTotalDistance = totalDistance;
//        }
//
//    }
//    return solutionPath[start];
//}
///*
//    std::vector<StreetSegmentIdx> solutionPath;
//    std::vector<IntersectionIdx> pickUpLocations;
//    int depotIntersectionIdx = 0;
//    int deliverIntersectionIdx = 0;
//    int startdepotIntersectionIdx = 0;
//    int startdeliverIntersectionIdx = 0;
//    //    int startdeliverNumber = 0;
//    LatLon depotLatLon;
//    LatLon deliverLatLon;
//
//    double shortestDistance = -1;
//    double distance = 0;
//
//    int count = 0;
//
//    //find the start location by choosing a depot and the nearest pickUp station
//    for (int i = 0; i < depots.size(); i++) {
//
//        depotIntersectionIdx = depots[i];
//        depotLatLon = getIntersectionPosition(depotIntersectionIdx);
//
//
//        for (int j = 0; j < deliveries.size(); j++) {
//            deliverIntersectionIdx = deliveries[j].pickUp;
//            deliverLatLon = getIntersectionPosition(deliverIntersectionIdx);
//            distance = findDistanceBetweenTwoPoints(std::make_pair(depotLatLon, deliverLatLon));
//
//            //the nearest pickUp location:
//            if (distance < shortestDistance || shortestDistance == -1) {
//                shortestDistance = distance;
//                startdepotIntersectionIdx = depotIntersectionIdx;
//                startdeliverIntersectionIdx = deliverIntersectionIdx;
//                //startdeliverNumber = j;
//            }
//        }
//
//        //find the path between the depot to the first pickUp Station:
//        solutionPath = findPathBetweenIntersections(turn_penalty,
//                std::make_pair(startdeliverIntersectionIdx, startdepotIntersectionIdx)); //the path btw the first depot and the closest deliverIntersection   
//
//        //store all the pickUp stations visited, which can be used to check legal 
//        //dropOff stations later
//
//        pickUpLocations.push_back(startdeliverIntersectionIdx); //the first pickUp station
//
//        LatLon startPosition;
//        int shortest = -1;
//        int nearestPickOrDropIDX = 0;
//        //nearest pick up or drop off station; 
//        //check if any packages are still left starting from the first delivery:
//        while (count != deliveries.size()) {
//
//            //find the nearest pickUp station to the startdeliverInteresctionIdx i.e the
//            //first pickUp location found previously:
//            
//            for (int i = 0; i < deliveries.size(); i++) {
//                deliverIntersectionIdx = deliveries[i].pickUp;
//                deliverLatLon = getIntersectionPosition(deliverIntersectionIdx);
//                startPosition = getIntersectionPosition(startdeliverIntersectionIdx);
//                distance = findDistanceBetweenTwoPoints(std::make_pair(startPosition, deliverLatLon));
//
//                //store the shortest distance and nearestPickUp station ID:
//                if (distance < shortest || shortest == -1) {
//                    shortest = distance;
//                    nearestPickOrDropIDX = deliverIntersectionIdx;
//                }
//            }
//            //??????????????????????????
//            //if this pickUp intersection is the one and not the dropOff then we also need to do:
//            //pickUpLocations.push_back(deliverIntersectionIdx);
//            //??????????????????????????
//            
//            //Now check for the dropOff stations to see if any if closer than the shortest distance 
//            //found for the nearestPickUp station found:
//            //this is done by traversing the vector which stores the pickUp points which are already
//            //counted in the solutionPath so far:
//            for (int j = 0; j < pickUpLocations.size(); j++) {
//                //getting the dropOff intersetionIds only for those whose pickUp's are known:
//                deliverIntersectionIdx = deliveries[pickUpLocations[i]].dropOff;
//                deliverLatLon = getIntersectionPosition(deliverIntersectionIdx);
//                startPosition = getIntersectionPosition(startdeliverIntersectionIdx);
//                distance = findDistanceBetweenTwoPoints(std::make_pair(startPosition, deliverLatLon));
//
//                //check if any distance to the dropOff is shorter than the nearestPickUp station found:
//                //if it is then find distance from startdeliverInteresctionIdx to dropOff instead of pickUp
//                //and add it to the solutionPath vector
//                if (distance < shortest) {
//                    shortest = distance;
//                    nearestPickOrDropIDX = deliverIntersectionIdx;
//
//                }
//
//            }
//
//            //need to add this path to the previous path so as to get the total path!
//            //but how to add vectors??
//            //????????????????????
//            solutionPath = findPathBetweenIntersections(turn_penalty,
//                    std::make_pair(startdeliverIntersectionIdx, nearestPickOrDropIDX));
//            //????????????????????
//
//            //update the startdeliverIntersectionIdx to the nearestPickOrDrop found n then go over the process 
//            //again till all the packages have been accounted for:
//            startdeliverIntersectionIdx = nearestPickOrDropIDX;
//            count++;
//        }
//
//
//
//
//
//
//
//
//    }
//
//
//
//}*/
//
//
//
