

#include "m1.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include "StreetsDatabaseAPI.h"
#include <cmath>


extern M1Global m1Global;
extern M2Global m2Global;

void drawStreetNamesNew(ezgl::renderer *g);

bool withinVisibleWorld(ezgl::rectangle visibleWorld, double x, double y) {
    if (x < visibleWorld.left() || x > visibleWorld.right()) return false;
    if (y < visibleWorld.bottom() || y > visibleWorld.top()) return false;

    return true;
}

// function to draw the line segments of a street with curvepoints

void drawCurvyStreet(ezgl::renderer *g, StreetSegmentInfo streetSegInfo, StreetSegmentIdx seg_idx) {
    //auto const startTime = std::chrono::high_resolution_clock::now();
    // get first street point
    LatLon fromCoord = getIntersectionPosition(streetSegInfo.from);
    double x0 = convertLonToX(fromCoord.longitude());
    double y0 = convertLatToY(fromCoord.latitude());

    LatLon nextCoord;
    double x1;
    double y1;

    // draw lines between all street points in order
    for (int i = 0; i < streetSegInfo.numCurvePoints; i++) {

        nextCoord = getStreetSegmentCurvePoint(i, seg_idx);
        x1 = convertLonToX(nextCoord.longitude());
        y1 = convertLatToY(nextCoord.latitude());

        g->draw_line({x0, y0},
        {
            x1, y1
        });

        x0 = x1;
        y0 = y1;
    }

    // get last street point
    nextCoord = getIntersectionPosition(streetSegInfo.to);
    x1 = convertLonToX(nextCoord.longitude());
    y1 = convertLatToY(nextCoord.latitude());

    g->draw_line({x0, y0},
    {
        x1, y1
    });
    //profile_csv(startTime,"drawCurvyStreet");
}


// function to draw all streets in the city

void drawStreets(ezgl::renderer *g) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    ezgl::rectangle visibleWorld = g->get_visible_world();

    // calculate current zoom level
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();

    // draw all street segments
    for (int i = 0; i < getNumStreetSegments(); i++) {
        g->set_line_width(std::min(zoomLevel / (float) 18, (float) 4));
        g->set_color(ezgl::GREY_75);

        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(i);

        IntersectionIdx from = streetSegInfo.from;
        IntersectionIdx to = streetSegInfo.to;

        double x0 = convertLonToX(getIntersectionPosition(from).longitude());
        double y0 = convertLatToY(getIntersectionPosition(from).latitude());
        double x1 = convertLonToX(getIntersectionPosition(to).longitude());
        double y1 = convertLatToY(getIntersectionPosition(to).latitude());

        // only draw a street segment if it's in the visible world
        if (withinVisibleWorld(visibleWorld, x0, y0) == false &&
                withinVisibleWorld(visibleWorld, x1, y1) == false) continue;

        // change colour and line width for major roads and highways
        // 90 km/h -> 25 m/s
        // 60 km/hh -> 16.66667 m/s
        if (streetSegInfo.speedLimit > MAJOR_SPEED_LIMIT) {
            if (m2Global.darkMode) {
                g->set_color(ezgl::MAJOR_MINOR_DARK);
            } else {
                g->set_color(ezgl::MAJOR_MINOR);
            }
            g->set_line_width(std::min(zoomLevel / 11, (float) 8));
        }

        if (streetSegInfo.speedLimit > HIGHWAY_SPEED_LIMIT) {
            if (m2Global.darkMode) {
                g->set_color(ezgl::HIGHWAY_DARK);
            } else {
                g->set_color(ezgl::HIGHWAY);
            }
            g->set_line_width(std::min(zoomLevel / 9, (float) 12));
        }// don't draw major roads when zoomed out
        else if (streetSegInfo.speedLimit <= 24 && zoomLevel < 4) continue;
            // don't draw minor roads when zoomed out
        else if (streetSegInfo.speedLimit <= 16 && zoomLevel < 18) continue;

        // curvy street?
        if (streetSegInfo.numCurvePoints > 0) drawCurvyStreet(g, streetSegInfo, i);
        else g->draw_line(ezgl::point2d(x0, y0), ezgl::point2d(x1, y1));
    }
//    profile_csv(startTime, "drawStreets");
}


// draw all street names in the city
// new implementation, maybe better maybe not

void drawStreetNamesNew(ezgl::renderer *g) {
    // bold font
    g->format_font("", ezgl::font_slant::normal, ezgl::font_weight::bold);

    if (m2Global.darkMode) g->set_color(ezgl::TEXT_DARK);
    else g->set_color(ezgl::TEXT);

    // determine font type by country
    if (m2Global.currentCountry == "beijing_china" || m2Global.currentCountry ==
            "hong-kong_china" || m2Global.currentCountry == "singapore" ||
            m2Global.currentCountry == "tokyo_japan") {
        g->format_font("Noto Sans CJK KR", ezgl::font_slant::normal, ezgl::font_weight::bold);
    }

    // calculate current zoom level
    ezgl::rectangle visibleWorld = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();

    int increment = 1;

    // start writing street names at this zoom level
    if (zoomLevel > 100) {
        // change the amount of street names that are displayed, to avoid overlap
        if (zoomLevel < 300) {
            increment = 9;
        } else if (zoomLevel >= 300 && zoomLevel < 500) {
            increment = 7;
        }

        // STRATEGY: (courtesy of Derek, Karen, and Iris)
        //      Loop through all street segments on a street
        //      Of the street segments in the visible world, determine the
        //      longest one
        //      Draw the street name on that segment 
        //      Since we loop by street, no need to check if a name has already
        //      been drawn
        for (int i = 0; i < getNumStreets(); i += increment) {
            // don't draw street names if their name is unknown
            if (getStreetName(i) == "<unknown>") continue;

            double longestSegmentLength = -1;
            double longestSegmentX = -1;
            double longestSegmentY = -1;
            double longestSegmentAngle = 0;

            std::vector<StreetSegmentIdx> streetSegments(m1Global.streetStreetSegments[i].begin(),
                    m1Global.streetStreetSegments[i].end());

            for (int j = 0; j < streetSegments.size(); j++) {
                StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegments[j]);

                // only draw major/highway names unless zoomed in a lot
                if (segInfo.speedLimit < MAJOR_SPEED_LIMIT && zoomLevel < 500) continue;

                double x0 = convertLonToX(getIntersectionPosition(segInfo.from).longitude());
                double y0 = convertLatToY(getIntersectionPosition(segInfo.from).latitude());
                double x1 = convertLonToX(getIntersectionPosition(segInfo.to).longitude());
                double y1 = convertLatToY(getIntersectionPosition(segInfo.to).latitude());

                double aveX = (x0 + x1) / 2;
                double aveY = (y0 + y1) / 2;

                // only check a street segment if it's in the visible world
                if (withinVisibleWorld(visibleWorld, aveX, aveY) == false) continue;

                // check if it's the longest visible street segment
                if (findStreetSegmentLength(streetSegments[j]) > longestSegmentLength) {
                    longestSegmentLength = findStreetSegmentLength(streetSegments[j]);
                    longestSegmentX = aveX;
                    longestSegmentY = aveY;

                    if (x0 == x1) {
                        longestSegmentAngle = 90;
                    } else {
                        longestSegmentAngle = atan((y0 - y1) / (x0 - x1));
                    }
                }
            }

            // if longest segment wasn't updated, street is not in visible world
            if (longestSegmentLength == -1) continue;

            g->set_text_rotation(longestSegmentAngle * 180 / M_PI);
            g->draw_text({longestSegmentX, longestSegmentY}, getStreetName(i));
        }
    }
}

// draw all street names (previous implementation, not great)

void drawStreetNames(ezgl::renderer * g) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    // bold font
    g->format_font("", ezgl::font_slant::normal, ezgl::font_weight::bold);

    // determine font type by country
    if (m2Global.currentCountry == "beijing_china" || m2Global.currentCountry ==
            "hong-kong_china" || m2Global.currentCountry == "singapore" ||
            m2Global.currentCountry == "tokyo_japan") {
        g->format_font("Noto Sans CJK KR", ezgl::font_slant::normal, ezgl::font_weight::bold);
    }

    // calculate current zoom level
    ezgl::rectangle visibleWorld = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();

    // vector that holds all streets whose names that have been drawn
    // unique street names will only be drawn once to avoid overlap
    std::vector<StreetIdx> currentlyDrawnStreets;

    int increment = 2;

    // start writing street names at this zoom level
    if (zoomLevel > 100) {
        // change the amount of street names that are displayed, to avoid overlap
        if (zoomLevel < 300) {
            increment = 9;
        } else if (zoomLevel >= 300 && zoomLevel < 500) {
            increment = 7;
        }

        for (int i = 0; i < getNumStreetSegments(); i += increment) {
            StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(i);

            // don't draw street names if their name is unknown
            if (getStreetName(streetSegInfo.streetID) == "<unknown>") continue;

            // only draw major/highway names unless zoomed in a lot
            if (streetSegInfo.speedLimit > MAJOR_SPEED_LIMIT || zoomLevel > 500) {

                IntersectionIdx from = streetSegInfo.from;
                IntersectionIdx to = streetSegInfo.to;

                double x0 = convertLonToX(getIntersectionPosition(from).longitude());
                double y0 = convertLatToY(getIntersectionPosition(from).latitude());
                double x1 = convertLonToX(getIntersectionPosition(to).longitude());
                double y1 = convertLatToY(getIntersectionPosition(to).latitude());

                double aveX = (x0 + x1) / 2;
                double aveY = (y0 + y1) / 2;

                // only draw a street name if it's in the visible world
                if (aveX < visibleWorld.left() || aveX > visibleWorld.right()
                        || aveY > visibleWorld.top() || aveY < visibleWorld.bottom()) {
                    continue;
                }

                // only write a street name once
                if (std::find(currentlyDrawnStreets.begin(), currentlyDrawnStreets.end(),
                        streetSegInfo.streetID) == currentlyDrawnStreets.end()) {

                    if (streetSegInfo.speedLimit > MAJOR_SPEED_LIMIT) {
                        g->set_font_size(std::min(zoomLevel / 11, (float) MAJOR_HIGHWAY_TEXT_SIZE));
                    } else if (streetSegInfo.speedLimit > 24) {
                        g->set_font_size(std::min(zoomLevel / 9, (float) MAJOR_HIGHWAY_TEXT_SIZE));
                    } else g->set_font_size(std::min(zoomLevel / (float) 18, (float) MINOR_TEXT_SIZE));

                    // calculate slope of street for text rotation
                    // vertical street
                    if (x0 == x1) {
                        g->set_text_rotation(90);
                    } else {
                        double angle = atan((y0 - y1) / (x0 - x1));

                        g->set_text_rotation(angle * 180 / M_PI);
                    }

                    // change text colour if dark mode
                    if (m2Global.darkMode) {
                        g->set_color(ezgl::TEXT_DARK);
                    } else {
                        g->set_color(ezgl::TEXT);
                    }

                    std::string name;

                    // add arrows if the street is one way
                    if (streetSegInfo.oneWay) {
                        std::string rightArrow = u8"\u2192";
                        std::string leftArrow = u8"\u2190";

                        // text will always be rightwards

                        // if from is more rightwards than to, draw left facing
                        // arrows
                        if (x0 > x1) {
                            name = leftArrow;
                            name.append(getStreetName(streetSegInfo.streetID));
                            name.append(leftArrow);
                            // otherwise, rightwards arrows
                        } else {
                            name = rightArrow;
                            name.append(getStreetName(streetSegInfo.streetID));
                            name.append(rightArrow);
                        }
                    } else {
                        name = getStreetName(streetSegInfo.streetID);
                    }

                    g->draw_text({aveX, aveY}, name);

                    currentlyDrawnStreets.push_back(streetSegInfo.streetID);
                }
            }
        }
    }
    //    profile_csv(startTime,"drawStreetNames");
}
