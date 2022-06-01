
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


// given a path of street segments, draw the route on the map

void drawPathBetweenIntersections(ezgl::renderer *g, std::vector<StreetSegmentIdx> path) {
    ezgl::rectangle visibleWorld = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();

    // draw complete path
    g->set_color(ezgl::HIGHLIGHT);

    for (int i = 0; i < path.size(); i++) {
        drawStreetSegment(g, path[i], zoomLevel, visibleWorld);
    }

    // draw section that user is currently on
    g->set_color(ezgl::LIGHT_SKY_BLUE);
    if (m3Global.drawPathDirections.empty()){
        return;
    }

    for (int i = 0; i < m3Global.drawPathDirections[0].size(); i++) {
        drawStreetSegment(g, m3Global.drawPathDirections[0][i], zoomLevel, visibleWorld);
    }
}

// draw a street segment on the map

void drawStreetSegment(ezgl::renderer *g, StreetSegmentIdx segment, float zoomLevel,
        ezgl::rectangle visibleWorld) {
    g->set_line_width(std::min(zoomLevel / (float) 16, (float) 8));

    StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(segment);

    IntersectionIdx from = streetSegInfo.from;
    IntersectionIdx to = streetSegInfo.to;

    double x0 = convertLonToX(getIntersectionPosition(from).longitude());
    double y0 = convertLatToY(getIntersectionPosition(from).latitude());
    double x1 = convertLonToX(getIntersectionPosition(to).longitude());
    double y1 = convertLatToY(getIntersectionPosition(to).latitude());

    // only draw a street segment if it's in the visible world
    if (withinVisibleWorld(visibleWorld, x0, y0) == false &&
            withinVisibleWorld(visibleWorld, x1, y1) == false) return;

    // change colour and line width for major roads and highways
    // make it slightly thicker than other streets on the map
    // 90 km/h -> 25 m/s
    // 60 km/hh -> 16.66667 m/s
    if (streetSegInfo.speedLimit > MAJOR_SPEED_LIMIT) {
        g->set_line_width(std::min(zoomLevel / 10, (float) 10));
    }

    if (streetSegInfo.speedLimit > HIGHWAY_SPEED_LIMIT) {
        g->set_line_width(std::min(zoomLevel / 8, (float) 14));
    }

    // curvy street?
    if (streetSegInfo.numCurvePoints > 0) drawCurvyStreet(g, streetSegInfo, segment);
    else g->draw_line(ezgl::point2d(x0, y0), ezgl::point2d(x1, y1));

}
