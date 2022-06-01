/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MaggieM1Functions.cpp
 * Author: yemaggie
 *
 * Created on February 1, 2022, 11:10 PM
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


// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    // (x,y) = (R*lon*cos(lat_avg), R*lat)
    // R is radius of Earth, latitude & longitude are in radians
    
    LatLon coord1 = points.first;
    LatLon coord2 = points.second;
    
    // convert latitudes, longitudes to radians
    double lat1 = coord1.latitude() * kDegreeToRadian;
    double lon1 = coord1.longitude() * kDegreeToRadian;
    
    double lat2 = coord2.latitude() * kDegreeToRadian;
    double lon2 = coord2.longitude() * kDegreeToRadian;
    
    double latAvg = (lat1 + lat2) / 2;
    
    // convert lat/lon coords to x/y coords using above equations
    double x1 = kEarthRadiusInMeters * lon1 * cos(latAvg);
    double y1 = kEarthRadiusInMeters * lat1;
    
    double x2 = kEarthRadiusInMeters * lon2 * cos(latAvg);
    double y2 = kEarthRadiusInMeters * lat2;
    
    // use Pythagorean theorem to find distance between x/y coords
    double length = x2 - x1;
    double width = y2 - y1;
    
    double distanceBetweenTwoPoints = sqrt(length * length + width * width);
    
    return distanceBetweenTwoPoints;   
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    StreetSegmentInfo streetSegment = getStreetSegmentInfo(street_segment_id);
    
    IntersectionIdx from = streetSegment.from;
    IntersectionIdx to = streetSegment.to;
    
    LatLon fromCoord = getIntersectionPosition(from);
    LatLon toCoord = getIntersectionPosition(to);
    
    //Calculate the distance for street segment with and without curve points
    if(streetSegment.numCurvePoints == 0){ // this segment is a straight line      
        std::pair <LatLon,LatLon> points (fromCoord, toCoord);
        
        return findDistanceBetweenTwoPoints(points);
    
    }else{
        int numCurvePoints = streetSegment.numCurvePoints;
        double totalLength = 0; 
        
        std::pair <LatLon, LatLon> curvyPoints;
        curvyPoints.first = fromCoord;
        
        //Returns the distance between curve points by adding the 
        //distance between each curve points
        for(int i = 0; i < numCurvePoints; i++){
            // returns coordinates of i'th curve point on given street segment
            // i starts at 0
            LatLon curvePointCoord = getStreetSegmentCurvePoint(i, street_segment_id);
            
            curvyPoints.second = curvePointCoord;
            
            totalLength += findDistanceBetweenTwoPoints(curvyPoints);
            
            // move forward down the street
            curvyPoints.first = curvyPoints.second;
        }
        
        // calculate length from last curve point to end of street
        curvyPoints.second = toCoord;
        totalLength += findDistanceBetweenTwoPoints(curvyPoints);
        
        return totalLength;  
    }
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id){
    double streetLength = 0;
    std::unordered_set<StreetSegmentIdx> streetsegment = m1Global.streetStreetSegments[street_id];
    
    //Returns the street distance by adding the distance between each segments
    
    for (auto it = streetsegment.begin(); it != streetsegment.end(); it++){
        streetLength += findStreetSegmentLength(*it);
        
    }
    
    return streetLength;
}

// Returns the travel time to drive from one end of a street segment 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 

//since the helper function calculateStreetSegmentTravelTimes() already stores the time it takes to traverse each streetSegments ID
//return the element stored in the vector travelTime by accessing the particular street_segment_id given to us
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return m1Global.travelTime[street_segment_id];
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    //int getNumFeaturePoints(FeatureIdx featureIdx);
    //LatLon getFeaturePoint(int pointNum, FeatureIdx featureIdx);
    
    std::vector<double> allx;
    std::vector<double> ally;
    double area = 0;
    
    double allLatitude =0;
    
    int numPoints = getNumFeaturePoints(feature_id);
    
    // check if feature is a closed area
    if (numPoints >= 2 && getFeaturePoint(0, feature_id) == getFeaturePoint(numPoints-1, feature_id)){
        for(int i =0; i< numPoints;i++){
            LatLon pointPosition = getFeaturePoint(i, feature_id);
            double lat = pointPosition.latitude()* kDegreeToRadian;
           
            allLatitude += lat;
        }
 
        double latAvg = (allLatitude) / (numPoints);
            
        //Transfer the latlon of each POI into meters and return all the x and y
        // coordinates
        for(int i =0; i<numPoints;i++){                
            LatLon pointPosition = getFeaturePoint(i, feature_id);
            double lat = pointPosition.latitude()* kDegreeToRadian;
            double lon = pointPosition.longitude()* kDegreeToRadian;
            double x = kEarthRadiusInMeters * lon * cos(latAvg);
            double y = kEarthRadiusInMeters * lat;
            allx.push_back (x);
            ally.push_back (y);
        } 
        
        //Return the area by using shoelace formula
        double twoTimesArea =0;
        for(int i = 0; i<numPoints-1; i++){
            twoTimesArea += (allx[i]*ally[i+1]-ally[i]*allx[i+1]);
        }
        
        twoTimesArea += (allx[numPoints-1]*ally[0]-ally[numPoints-1]*allx[0]);
        area = twoTimesArea / 2;
        return abs(area);
        
    }else{
        return 0;
    }   
}
