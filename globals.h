/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   globals.h
 * Author: yemaggie
 *
 * Created on February 3, 2022, 7:32 PM
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <unordered_set>
#include <unordered_map>
#include <map>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "LatLon.h"
#include "OSMID.h"
#include "ezgl/color.hpp"
#include <chrono>
#include <fstream>


// structs for extra information
struct SubwayStationData {
    double x;
    double y;
    std::string name;
    ezgl::color color;
    bool highlight = false;
};

struct SubwayWayPointData{
    double x;
    double y;
    bool beginOfWay = false;
};

struct Intersection_data {
    double x;
    double y;
    std::string name;
    bool highlight = false;
};


//M1 globals
struct M1Global{
    // vector that will hold all the street segments at an intersection
    std::vector<std::vector<StreetSegmentIdx>> intersectionStreetSegments;

    // vector that will hold all the intersections on a given street
    std::vector<std::unordered_set<IntersectionIdx>> streetIntersections;

    // vector that will hold all the street segments on a given street
    std::vector<std::unordered_set<StreetSegmentIdx>> streetStreetSegments;
    
    // multimap that will hold all the names of each street
    std::multimap<std::string, StreetIdx> streetNames;
    
    // hash map that will hold all the LatLons of each OSM node
    std::unordered_map<OSMID, LatLon> findingLatlonOfOSMNodes;
    
    // vector that will hold all the travel times of each street segment
    std::vector<double> travelTime;
    
    // vector that will hold information about all subway stations
    std::vector<SubwayStationData> subwayStationLocations;
    
    // vector that will hold information about the colors of each subway line
    std::vector<ezgl::color> subwayLineColors;
    
    // vector that will hold information about the path of each subway line
    std::vector<std::vector<SubwayWayPointData>> subwayLineLocations;
    
    // hash map that will hold information about the OSM pointers for each OSM node ID
    std::unordered_map<OSMID, const OSMNode*> OSMNodesFromOSMID; 
    
    // hash map tha twill hold information about the OSM pointers for each OSM way ID
    std::unordered_map<OSMID, const OSMWay*> OSMWaysFromOSMID;
    
    // vector that will hold all the areas of each feature
    std::vector<std::pair<double,int>> sortFeature;
};


//M2 globals
struct M2Global{
    double initialWorldArea;
    ezgl::rectangle visibleWorld;
    
    // world bounds and conversions
    float aveLat;
    double cosAveLat;
    double maxX;
    double minX;
    double maxY;
    double minY;
    
    std::vector<Intersection_data> intersections;
    std::vector<IntersectionIdx> intersectionsHighlight;
    
    // toggle colour schemes
    bool darkMode = false;
    
    // toggle subways
    bool subwaysOn = false;
    
    // toggle POIs
    bool schoolsOn = false;
    bool hospitalsOn = false;
    bool foodPlacesOn = false;
    
    std::string currentCountry;
        
    IntersectionIdx cohenIntersection;  
};


// M3 globals
struct M3Global{
    std::vector<std::string> pathDirections;
    std::vector<StreetSegmentIdx> path;
    std::vector<std::vector<StreetSegmentIdx>> drawPathDirections;
    
    bool setFirst = false;
    bool setSecond = false;
    
    IntersectionIdx start = -1;
    IntersectionIdx dest = -1;    
};

// helper functions for initializing global variables
void getStreetSegmentsofIntersections();
void getIntersectionsofStreet();
void getStreetSegmentsofStreet();
void getStreetNamesofStreet();
void getLatLonofOSMNodes();
void calculateStreetSegmentTravelTimes();

void getFeatureAreas();
void getIntersectionDataStructure();
void findAveLatWorldBounds();
void loadSubwayStations();
ezgl::color getSubwayLineColor(std::string color);



void initialSetup(ezgl::application* application, bool /*new_window */);


// helpful conversion functions
double convertXToLon(double x);
double convertYToLat(double y);
double convertLonToX(double Lon);
double convertLatToY(double Lat);


// drawing functions
void drawMainCanvas(ezgl::renderer *g);
void drawMap();

void drawStreets(ezgl::renderer *g);
void drawStreetNames(ezgl::renderer *g);
void drawCurvyStreet(ezgl::renderer *g, StreetSegmentInfo streetSegInfo, StreetSegmentIdx seg_idx);

void drawFeature(ezgl::renderer *g);
void drawPOI(ezgl::renderer *g);
void drawIntersections(ezgl::renderer *g);

void drawSubwayLines(ezgl::renderer *g);
void drawSubwayStations(ezgl::renderer *g);

void drawCohen(ezgl::renderer *g);


// navigation UI
void getPathDirections(std::vector<StreetSegmentIdx> path);
void printDirectionsOnScreen(GtkWidget */*widget*/, ezgl::application *application);

void navigationPopUpBox(GtkWidget */*widget*/, ezgl::application *application);
void setIntersection(ezgl::application *application);
void findPathGivenInput(ezgl::application *application);
void resetIntersections(ezgl::application *application);

int findQuadrantOfStreetSegment(IntersectionIdx point, IntersectionIdx ref);
double findAngleBetweenIntersections(IntersectionIdx one, IntersectionIdx two);
std::string findStartingDirection();
std::string convertTravelTime(double travelTime);
std::string convLengthToString(int length);
std::vector<IntersectionIdx> determineIntersections(std::pair<StreetSegmentInfo, StreetSegmentInfo> segments);

// drawing navigation
void drawPathBetweenIntersections(ezgl::renderer *g, std::vector<StreetSegmentIdx>);
void drawStreetSegment(ezgl::renderer *g, StreetSegmentIdx segment, float zoomLevel,
        ezgl::rectangle visibleWorld);

// toggle points of interest
void POIsPopUpBox(GtkWidget */*widget*/, ezgl::application *application);
void showSchools(ezgl::application *application);
void showFoodPlaces(ezgl::application *application);
void showHospitals(ezgl::application *application);

// toggle subways
void showSubways(GtkWidget *widget, ezgl::application *application);

// where's Professor Cohen? game
void wheresProf(GtkWidget */*widget*/, ezgl::application *application);
void resetCohen(ezgl::application *application);
void cohenHint(ezgl::application *application);


// toggle colour scheme
void toggleDarkMode(GtkWidget *widget, ezgl::application *application);


int findClosestSubwayStation(LatLon my_position);

bool withinVisibleWorld(ezgl::rectangle visibleWorld, double x, double y);


// glade button/text box functions
void actingOnMouseClicks(ezgl::application *app, GdkEventButton *event, double x, double y);
void streetNameEntryChanged1(GtkEntry* self, gpointer user_data);
void streetNameEntryChanged2(GtkEntry* self, gpointer user_data);
void selectCountry( GtkComboBox *widget, ezgl::application *application);
void find_button(GtkWidget *widget, ezgl::application *application);


// declare "magic constants"
static constexpr int const& MAJOR_SPEED_LIMIT = 16;
static constexpr int const& HIGHWAY_SPEED_LIMIT = 24;
static constexpr int const& MAJOR_HIGHWAY_TEXT_SIZE = 11;
static constexpr int const& MINOR_TEXT_SIZE = 8;

void profile_csv(std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
    std::string func_name);

#endif /* GLOBALS_H */

