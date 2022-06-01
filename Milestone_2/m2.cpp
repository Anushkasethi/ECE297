
#include "m1.h"
#include "m3.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/callback.hpp"
#include "StreetsDatabaseAPI.h"
#include <limits>
#include <cmath>

M2Global m2Global;
extern M3Global m3Global;

// Highlights the intersections whenever the user clicks on any point taking into consideration that the zoomLevel is >=400

void drawIntersections(ezgl::renderer *g) {
    ezgl::rectangle visible_world = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visible_world.area();

    float width;

    // set size of intersection depending on zoom level
    if (zoomLevel < 15) width = 300;
    else if (zoomLevel >= 15 && zoomLevel < 300) width = 100;
    else if (zoomLevel >= 300 && zoomLevel < 500) width = 50;
    else if (zoomLevel >= 500 && zoomLevel < 3000) width = 20;
    else width = 10;

    for (size_t i = 0; i < m2Global.intersectionsHighlight.size(); i++) {
        // don't highlight the intersection if it's the start or destination
        if (m2Global.intersectionsHighlight[i] == m3Global.start ||
                m2Global.intersectionsHighlight[i] == m3Global.dest) {
            continue;
        }
        g->set_color(ezgl::HIGHLIGHT);

        //to find the coordinates for the intersection we want to highlight
        float x = m2Global.intersections[m2Global.intersectionsHighlight[i]].x;
        float y = m2Global.intersections[m2Global.intersectionsHighlight[i]].y;

        g->fill_arc({x, y}, width, 0, 360);
    }

    // draw start if it has been set
    if (m3Global.start != -1) {
        // colour starting point as purple
        g->set_color(ezgl::PURPLE);
        float x = m2Global.intersections[m3Global.start].x;
        float y = m2Global.intersections[m3Global.start].y;
        g->fill_arc({x, y}, width, 0, 360);
    }

    // draw dest if it has been set
    if (m3Global.dest != -1) {
        // set marker png for destination
        ezgl::surface *png_surface = ezgl::renderer::load_png(
                "libstreetmap/resources/marker.png");
        float x = m2Global.intersections[m3Global.dest].x;
        float y = m2Global.intersections[m3Global.dest].y;
        g->draw_surface(png_surface,{x, y}, 0.1);
    }
}

void drawMainCanvas(ezgl::renderer *g) {
    //implement the dark mode feature of the map if the dark mode button is clicked
    if (m2Global.darkMode) {
        g->set_color(ezgl::BACKGROUND_DARK);
    } else {
        g->set_color(ezgl::BACKGROUND);
    }

    //    ezgl::rectangle visibleWorld = g->get_visible_world();
    //    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();
    //        std::cout << zoomLevel << std::endl;

    g->fill_rectangle({m2Global.minX, m2Global.minY},
    {
        m2Global.maxX, m2Global.maxY
    });

    drawFeature(g);
    drawStreets(g);
    drawPOI(g);

    // check if the subway button is ON, and display the subway lines on the map if ON
    if (m2Global.subwaysOn) {
        drawSubwayLines(g);
        drawSubwayStations(g);
    }

    // if there is a path, draw it
    if (m3Global.path.size() != 0) {
        drawPathBetweenIntersections(g, m3Global.path);
    }

    drawIntersections(g);

    drawCohen(g);

    drawStreetNames(g);
}

//Draw the Main Window

void drawMap() {
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings);
    ezgl::rectangle initial_world({m2Global.minX, m2Global.minY},
    {
        m2Global.maxX, m2Global.maxY
    });

    application.add_canvas("MainCanvas", drawMainCanvas, initial_world);
    application.run(initialSetup, actingOnMouseClicks, nullptr, nullptr); // callback functions: initial_setup & actingOnMouseClicks 

}


//Initial world set-Up: Finds the world Bounds and the average latitude

void findAveLatWorldBounds() {
    double maxLon = std::numeric_limits<double>::lowest();
    double minLon = std::numeric_limits<double>::max();
    double maxLat = std::numeric_limits<double>::lowest();
    double minLat = std::numeric_limits<double>::max();

    for (int i = 0; i < getNumIntersections(); i++) {
        double lat = getIntersectionPosition(i).latitude();
        double lon = getIntersectionPosition(i).longitude();

        if (lon > maxLon) maxLon = lon;
        if (lon < minLon) minLon = lon;
        if (lat > maxLat) maxLat = lat;
        if (lat < minLat) minLat = lat;
    }

    m2Global.aveLat = (maxLat + minLat) / 2 * kDegreeToRadian;
    m2Global.cosAveLat = cos(m2Global.aveLat);

    // x = R * lon * cos(ave_lat)
    // y = R * lat
    m2Global.maxX = kEarthRadiusInMeters * maxLon * kDegreeToRadian * m2Global.cosAveLat;
    m2Global.minX = kEarthRadiusInMeters * minLon * kDegreeToRadian * m2Global.cosAveLat;
    m2Global.maxY = kEarthRadiusInMeters * maxLat * kDegreeToRadian;
    m2Global.minY = kEarthRadiusInMeters * minLat * kDegreeToRadian;

    m2Global.initialWorldArea = (m2Global.maxX - m2Global.minX) * (m2Global.maxY - m2Global.minY);
}


//sets up the intersection data structure

void getIntersectionDataStructure() {
    m2Global.intersections.resize(getNumIntersections());

    // x = R * lon * cos(ave_lat)
    // y = R * lat
    for (int i = 0; i < getNumIntersections(); i++) {
        double lat = getIntersectionPosition(i).latitude() * kDegreeToRadian;
        double lon = getIntersectionPosition(i).longitude() * kDegreeToRadian;
        double xCoord = kEarthRadiusInMeters * lon * m2Global.cosAveLat;
        double yCoord = kEarthRadiusInMeters * lat;


        m2Global.intersections[i].x = xCoord;
        m2Global.intersections[i].y = yCoord;
        m2Global.intersections[i].name = getIntersectionName(i);
    }
}