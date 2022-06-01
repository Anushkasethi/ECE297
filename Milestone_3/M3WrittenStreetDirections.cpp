
#include "m1.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include "StreetsDatabaseAPI.h"
#include "m3.h"
#include <cmath>
#include <sstream>
#include <iomanip>


extern M1Global m1Global;
extern M2Global m2Global;
extern M3Global m3Global;


// given a point and a reference point, determine what quadrant the line between
// the points is in

int findQuadrantOfStreetSegment(IntersectionIdx point, IntersectionIdx ref) {
    LatLon pointLatLon = getIntersectionPosition(point);
    LatLon refLatLon = getIntersectionPosition(ref);

    double x0 = convertLonToX(refLatLon.longitude());
    double y0 = convertLatToY(refLatLon.latitude());
    double x1 = convertLonToX(pointLatLon.longitude());
    double y1 = convertLatToY(pointLatLon.latitude());

    double deltaX = x1 - x0;
    double deltaY = y1 - y0;

    if (deltaX < 0) {
        // bottom left
        if (deltaY < 0) {
            return 3;
            // top left
        } else {
            return 2;
        }
    } else {
        // bottom right
        if (deltaY < 0) {
            return 4;
            // bottom left
        } else {
            return 1;
        }
    }
}

// given a distance, convert it to km or m and round it to 1 decimal place

std::string convLengthToString(int length) {
    std::string result;

    // if length can be rounded to 1000, round to nearest 100 and convert to km
    if (length >= 950) {
        double km = (double) length / (double) 1000;

        // round up
        if (length % 100 >= 50) {
            length += 0.1;
        }

        std::stringstream setOneDecimalPlace;
        setOneDecimalPlace << std::fixed << std::setprecision(1) << km;

        result = setOneDecimalPlace.str() + " km";
        // otherwise, round to nearest 10 and keep in m
    } else {
        // round down
        if (length % 10 < 5) {
            length /= 10;
            length *= 10;

            // round up
        } else {
            length /= 10;
            length *= 10;
            length += 10;
        }

        std::stringstream setOneDecimalPlace;
        setOneDecimalPlace << length;

        result = setOneDecimalPlace.str() + " metres";
    }
    return result;
}

// find the initial direction of the route to be printed on the bottom of the
// screen

std::string findStartingDirection() {
    std::string res;

    // use first two segments to determine starting direction
    // find common intersection
    IntersectionIdx startPoint;
    IntersectionIdx nextPoint;

    // if path is one segment, use start and dest to determine direction
    if (m3Global.path.size() == 1) {
        startPoint = m3Global.start;
        nextPoint = m3Global.dest;

        // otherwise, use the first segment in the path
    } else {
        StreetSegmentInfo curSegInfo = getStreetSegmentInfo(m3Global.path[0]);
        StreetSegmentInfo nextSegInfo = getStreetSegmentInfo(m3Global.path[1]);

        if (curSegInfo.from == nextSegInfo.from || curSegInfo.from == nextSegInfo.to) {
            startPoint = curSegInfo.to;
            nextPoint = curSegInfo.from;
        } else {
            startPoint = curSegInfo.from;
            nextPoint = curSegInfo.to;
        }
    }

    LatLon startCoord = getIntersectionPosition(startPoint);
    LatLon nextCoord = getIntersectionPosition(nextPoint);

    double deltaX = convertLonToX(startCoord.longitude()) - convertLonToX(nextCoord.longitude());
    double deltaY = convertLatToY(startCoord.latitude()) - convertLatToY(nextCoord.latitude());

    // if change in x is greater, assume east/west
    if (abs(deltaX) > abs(deltaY)) {
        if (deltaX < 0) {
            res = "east";
        } else {
            res = "west";
        }
        // else assume north/south
    } else {
        if (deltaY < 0) {
            res = "north";
        } else {
            res = "south";
        }
    }
    return res;
}

// given two street segments, determine their common intersection, previous,
// and next intersection

std::vector<IntersectionIdx> determineIntersections(std::pair<StreetSegmentInfo, StreetSegmentInfo> segments) {
    StreetSegmentInfo prevStreet = segments.first;
    StreetSegmentInfo curStreet = segments.second;

    // three intersections: the common intersection
    //                      the previous intersection
    //                      the next intersection
    IntersectionIdx commonIts;
    IntersectionIdx prevIts;
    IntersectionIdx nextIts;

    // determine common intersection of the two street segments
    if (prevStreet.from == curStreet.from || prevStreet.from == curStreet.to) {
        commonIts = prevStreet.from;
        prevIts = prevStreet.to;

        if (commonIts == curStreet.from) {
            nextIts = curStreet.to;
        } else {
            nextIts = curStreet.from;
        }

    } else {
        commonIts = prevStreet.to;
        prevIts = prevStreet.from;

        if (commonIts == curStreet.from) {
            nextIts = curStreet.to;
        } else {
            nextIts = curStreet.from;
        }
    }
    return {prevIts, commonIts, nextIts};
}

// get the directions of the route to be printed on the bottom of the screen
// as well as the street segments corresponding to each direction

void getPathDirections(std::vector<StreetSegmentIdx> path) {
    m3Global.pathDirections.clear();
    m3Global.drawPathDirections.clear();

    StreetSegmentInfo prevStreet = getStreetSegmentInfo(path[0]);
    std::string prevStreetName = getStreetName(prevStreet.streetID);

    // holds the length of travel for each direction
    int straightLength = findStreetSegmentLength(path[0]);

    // street segments corresponding to the current direction
    std::vector<StreetSegmentIdx> currentDirection;

    // push an empty vector into the drawing path as user has not started
    // navigation yet
    m3Global.drawPathDirections.push_back(currentDirection);

    // find starting direction
    std::string direction = findStartingDirection();

    // don't print street name if <unknown>
    if (prevStreetName == "<unknown>") {
        m3Global.pathDirections.push_back("Head " + direction);
    } else {
        m3Global.pathDirections.push_back("Head " + direction + " on " + prevStreetName);
    }

    currentDirection.push_back(path[0]);
    m3Global.drawPathDirections.push_back(currentDirection);

    currentDirection.clear();
    currentDirection.push_back(path[0]);

    for (int i = 1; i < path.size(); i++) {
        StreetSegmentInfo curStreet = getStreetSegmentInfo(path[i]);
        std::string curStreetName = getStreetName(curStreet.streetID);

        // if street name hasn't changed, no need to update directions
        if (curStreet.streetID == prevStreet.streetID) {
            prevStreet = curStreet;
            prevStreetName = curStreetName;

            straightLength += findStreetSegmentLength(path[i]);
            currentDirection.push_back(path[i]);
            continue;

        } else {
            // get intersections relative to each other
            std::vector<IntersectionIdx> intersections = determineIntersections(
            {prevStreet, curStreet});

            IntersectionIdx prevIts = intersections[0];
            IntersectionIdx commonIts = intersections[1];
            IntersectionIdx nextIts = intersections[2];

            // determine what quadrant each street is in
            int prevQuadrant = findQuadrantOfStreetSegment(prevIts, commonIts);
            int curQuadrant = findQuadrantOfStreetSegment(nextIts, commonIts);

            // left turn: go to the previous quadrant, or from quadrant 1 to quadrant 4
            if (curQuadrant - prevQuadrant == -1 || curQuadrant - prevQuadrant == 3) {
                // don't print street name if <unknown>
                if (prevStreetName == "<unknown>") {
                    m3Global.pathDirections.push_back("Continue straight for " +
                            convLengthToString(straightLength));
                } else {
                    m3Global.pathDirections.push_back("Continue straight along " + prevStreetName
                            + " for " + convLengthToString(straightLength));
                }

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segments of turn to be highlighted
                currentDirection.push_back(path[i - 1]);
                currentDirection.push_back(path[i]);
                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segment of current position
                currentDirection.push_back(path[i]);

                // determine slight left or left turn depending on the quadrants
                // of each street segment
                //      turning from 2-1 or 4-3: check 180 - sum
                //      turning from 3-2 or 1-4: check sum
                // set 150 as the limit of making a "slight" turn
                double prevAngle = abs(findAngleBetweenIntersections(prevIts, commonIts) * 180 / M_PI);
                double curAngle = abs(findAngleBetweenIntersections(nextIts, commonIts) * 180 / M_PI);

                if ((prevQuadrant % 2 == 0 && 180 - (prevAngle + curAngle) > 150)
                        || (prevQuadrant % 2 == 1 && (prevAngle + curAngle) > 150)) {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Make a slight left turn");
                    } else {
                        m3Global.pathDirections.push_back("Make a slight left turn onto " + curStreetName);
                    }

                } else {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Turn left");
                    } else {
                        m3Global.pathDirections.push_back("Turn left onto " + curStreetName);
                    }
                }

                straightLength = findStreetSegmentLength(path[i]);

                // right turn: go to the next quadrant, or from quadrant 4 to quadrant 1
            } else if (curQuadrant - prevQuadrant == 1 || curQuadrant - prevQuadrant == -3) {
                // don't print street name if <unknown>
                if (prevStreetName == "<unknown>") {
                    m3Global.pathDirections.push_back("Continue straight for "
                            + convLengthToString(straightLength));
                } else {
                    m3Global.pathDirections.push_back("Continue straight along " + prevStreetName
                            + " for " + convLengthToString(straightLength));
                }

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segments of turn to be highlighted
                currentDirection.push_back(path[i - 1]);
                currentDirection.push_back(path[i]);

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segment of current position
                currentDirection.push_back(path[i]);

                // determine slight right or right turn
                //      turning from 2-3 or 4-1: check sum
                //      turning from 3-4 or 1-2: check 180 - sum
                double prevAngle = abs(findAngleBetweenIntersections(prevIts, commonIts) * 180 / M_PI);
                double curAngle = abs(findAngleBetweenIntersections(nextIts, commonIts) * 180 / M_PI);

                if ((prevQuadrant % 2 == 0 && (prevAngle + curAngle) > 150)
                        || (prevQuadrant % 2 == 1 && 180 - (prevAngle + curAngle) > 150)) {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Make a slight right turn");
                    } else {
                        m3Global.pathDirections.push_back("Make a slight right turn onto " + curStreetName);
                    }

                } else {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Turn right");
                    } else {
                        m3Global.pathDirections.push_back("Turn right onto " + curStreetName);
                    }
                }

                straightLength = findStreetSegmentLength(path[i]);

                // straight: go to the opposite quadrant (1-3 or 2-4)
            } else if (curQuadrant - prevQuadrant == 2 || curQuadrant - prevQuadrant == -2) {
                // don't print street name if <unknown>
                if (prevStreetName == "<unknown>") {
                    m3Global.pathDirections.push_back("Continue straight for "
                            + convLengthToString(straightLength));
                } else {
                    m3Global.pathDirections.push_back("Continue straight along " + prevStreetName
                            + " for " + convLengthToString(straightLength));
                }

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                currentDirection.push_back(path[i]);

                straightLength = findStreetSegmentLength(path[i]);

                // same quadrant, different street names: sharp left or right turn
            } else {
                // STRATEGY: find angle of previous street segment
                //           and angle of current street segment
                //           Quadrant 1: large->small=left, small->large=right (+)
                //           Quadrant 2: large->small=right, small->large=left (-)
                //                       larger in terms of magnitude, so really
                //                       more negative->less negative=right,
                //                       less negative->more negative=left
                //           Quadrant 3: large->small=left, small->large=right (+)
                //           Quadrant 4: large->small=right, small->large=left (-)
                //           SO REALLY for everything, large->small=left, small->large=right

                double prevAngle = findAngleBetweenIntersections(prevIts, commonIts);
                double curAngle = findAngleBetweenIntersections(commonIts, nextIts);

                // more negative->less negative
                if (prevAngle < curAngle) {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Make a sharp right turn");
                    } else {
                        m3Global.pathDirections.push_back("Make a sharp right turn onto"
                                + curStreetName);
                    }

                } else {
                    // don't print street name if <unknown>
                    if (prevStreetName == "<unknown>") {
                        m3Global.pathDirections.push_back("Make a sharp left turn");
                    } else {
                        m3Global.pathDirections.push_back("Make a sharp left turn onto"
                                + curStreetName);
                    }
                }

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segments of turn to be highlighted
                currentDirection.push_back(path[i - 1]);
                currentDirection.push_back(path[i]);

                m3Global.drawPathDirections.push_back(currentDirection);
                currentDirection.clear();

                // save street segment of current position
                currentDirection.push_back(path[i]);

                straightLength = findStreetSegmentLength(path[i]);
            }
        }
        prevStreet = curStreet;
        prevStreetName = curStreetName;
    }

    // save last direction
    // don't print street name if <unknown>
    if (prevStreetName == "<unknown>") {
        m3Global.pathDirections.push_back("Continue straight for "
                + convLengthToString(straightLength));
    } else {
        m3Global.pathDirections.push_back("Continue along " + prevStreetName + " for "
                + convLengthToString(straightLength));
    }

    m3Global.drawPathDirections.push_back(currentDirection);
    currentDirection.clear();

    m3Global.pathDirections.push_back("Destination");
}


// given the travel time of the entire trip, convert it to hours + minutes

std::string convertTravelTime(double travelTime) {
    std::string res;

    travelTime /= 60; // convert to minutes

    int convertedTime = (int) travelTime;

    // if longer than an hour, convert to hours + minutes
    if (travelTime > 60) {
        int hours = convertedTime / 60;
        int minutes = convertedTime % 60;

        res = std::to_string(hours) + "h " + std::to_string(minutes) +
                "min";

    } else {
        int minutes = convertedTime;
        res = std::to_string(minutes) + "min";
    }
    return res;
}

// given two intersections, find the angle of the line connecting them

double findAngleBetweenIntersections(IntersectionIdx one, IntersectionIdx two) {
    LatLon oneCoord = getIntersectionPosition(one);
    LatLon twoCoord = getIntersectionPosition(two);

    double x0 = convertLonToX(oneCoord.longitude());
    double y0 = convertLatToY(oneCoord.latitude());
    double x1 = convertLonToX(twoCoord.longitude());
    double y1 = convertLatToY(twoCoord.latitude());

    return atan((y1 - y0) / (x1 - x0));
}