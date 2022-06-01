
#include <iostream>
#include <vector>
#include "OSMDatabaseAPI.h"
#include "LatLon.h"
#include "m1.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "globals.h"
#include <cmath>
#include <string>
#include <sstream>
#include <gtk/gtk.h>


extern M2Global m2Global;
extern M1Global m1Global;


void actingOnMouseClicks(ezgl::application *app, GdkEventButton * /*event*/, double x, double y) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    LatLon coordinates = LatLon(convertYToLat(y), convertXToLon(x));
    std::stringstream ss;
    IntersectionIdx id;

    // find closest intersection (when subways are not showing)
    if (m2Global.subwaysOn == false) {

        id = findClosestIntersection(coordinates);

        // reset list of intersections to be highlighted
        m2Global.intersectionsHighlight.clear();
        m2Global.intersectionsHighlight.push_back(id);


        // if Cohen is at that intersection, print a special message
        if (id == m2Global.cohenIntersection) {
            ss << "Congrats, you found Professor Cohen at " << m2Global.intersections[id].name
                    << "! " << u8"\u263A" << " Click on Reset Prof to hide him again.";
        } else {
            ss << "Intersection Selected: " << m2Global.intersections[id].name;
        }


        // otherwise find closest subway station
    } else {
        int StationIdx = findClosestSubwayStation(coordinates);
        std::string name = m1Global.subwayStationLocations[StationIdx].name;
        m1Global.subwayStationLocations[StationIdx].highlight = true;

        ss << "Closest Subway Station: " << name;
    }

    app->update_message(ss.str()); // Message at screen bottom
    app->refresh_drawing();
}



// draws all subway lines on map

void drawSubwayLines(ezgl::renderer *g) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    g->set_line_width(5);
    for (int i = 0; i < m1Global.subwayLineLocations.size(); i++) {
        // set the color of the line
        //        if(i >= m1Global.subwayLineColors.size()){
        //            g->set_color(m1Global.subwayLineColors[0]);
        //        }else{
        g->set_color(m1Global.subwayLineColors[i]);

        if (m1Global.subwayLineLocations[i].size() == 0) continue;

        // get first point
        double x0 = m1Global.subwayLineLocations[i][0].x;
        double y0 = m1Global.subwayLineLocations[i][0].y;

        double x1;
        double y1;

        for (int j = 1; j < m1Global.subwayLineLocations[i].size(); j++) {
            // don't draw a line between points if the second point is the 
            // beginning of a new way              

            x1 = m1Global.subwayLineLocations[i][j].x;
            y1 = m1Global.subwayLineLocations[i][j].y;

            if (m1Global.subwayLineLocations[i][j].beginOfWay == false) {
                g->draw_line({x0, y0},
                {
                    x1, y1
                });
            }

            x0 = x1;
            y0 = y1;
        }
    }
//    profile_csv(startTime, "drawSubwayLines");
}


// draws all subway stations on the map

void drawSubwayStations(ezgl::renderer *g) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    // draw the highlighted station last so it shows up over everything else
    SubwayStationData highlightedStation;
    for (int i = 0; i < m1Global.subwayStationLocations.size(); i++) {
        if (m1Global.subwayStationLocations[i].highlight) {
            highlightedStation = m1Global.subwayStationLocations[i];
            m1Global.subwayStationLocations[i].highlight = false;
        }
        g->set_color(m1Global.subwayStationLocations[i].color);

        // draw station as a circle on the map
        g->fill_arc({m1Global.subwayStationLocations[i].x, m1Global.subwayStationLocations[i].y},
        20, 0, 360);
    }
    g->set_color(ezgl::RED);
    g->fill_arc({highlightedStation.x, highlightedStation.y}, 20, 0, 360);
//    profile_csv(startTime, "drawSubwayStations");
}


// toggles drawing subways when button is pressed

void showSubways(GtkWidget */*widget*/, ezgl::application *application) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    m2Global.subwaysOn = !m2Global.subwaysOn;
    application->refresh_drawing();
//    profile_csv(startTime, "showSubways");
}
