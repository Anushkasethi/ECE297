/* 
 * Copyright 2022 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <string>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "math.h"
#include "globals.h"
#include "ezgl/color.hpp"
#include <chrono>
#include <fstream>


bool mapLoaded = false;


// TO DO:
//   check corner cases (???))
//   add better comments!
//      high level comments at the top of the file
//      low level comments above functions
//   add more variables (don't do too much in a statement)
//      use auto for iterators in for loops
//   use assert
//   see project management & code style rubric


// initialize all global variables (see globals.h)
M1Global m1Global;
M3Global m3Global;
extern M2Global m2Global;
//std::ofstream profile_csv_file("response time.csv");

//profile_csv_file must already be open (opened in load map)
//void profile_csv(std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
//std::string func_name) {
//    
//#ifdef PROFILING
//    auto const endTime = std::chrono::high_resolution_clock::now();
//    auto const elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>
//    (endTime-startTime);
//    profile_csv_file << func_name << "," << elapsedTime.count() << std::endl;
//#endif
//}

bool loadMap(std::string map_streets_database_filename) {
    bool loadStreetsSuccessful = false; 
    bool loadOSMSuccessful = false;
    
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    // check for valid path
    if(map_streets_database_filename.find(".streets.bin") == std::string::npos) return false;
        
    // check for and close any map that is currently loaded
    if(mapLoaded == true) closeMap();
    
    // load StreetsDatabase
    loadStreetsSuccessful = loadStreetsDatabaseBIN(map_streets_database_filename);
    
    // check for invalid file path
    if(loadStreetsSuccessful == false) return false;
    
    // change string to .osm.bin and load OSMDatabase
    std::string mapOSMDatabaseFilename = map_streets_database_filename.replace(map_streets_database_filename.end()-11,
            map_streets_database_filename.end(), "osm.bin");
    loadOSMSuccessful = loadOSMDatabaseBIN(mapOSMDatabaseFilename);
    
    // confirm that both OSM and StreetsAPI have loaded successfully
    mapLoaded = loadStreetsSuccessful && loadOSMSuccessful;
    
    if(mapLoaded == false){
        return mapLoaded;
    }
    
    // load global variables
    getStreetSegmentsofIntersections();
    getIntersectionsofStreet();
    getStreetSegmentsofStreet();
    getStreetNamesofStreet();
    getLatLonofOSMNodes();
    calculateStreetSegmentTravelTimes();
    
    // load global variables for M2
    findAveLatWorldBounds();
    getIntersectionDataStructure();
    loadSubwayStations();
    getFeatureAreas();
    
    // set up Cohen's location
    srand (time(NULL));
    m2Global.cohenIntersection = std::rand() % getNumIntersections(); // 1712;(exam centre location)
    
    // set up start and destination as nothing
    m3Global.dest = -1;
    m3Global.start = -1;
    
    return mapLoaded;
}

void getStreetSegmentsofIntersections(){
    // a vector created for holding all the intersections & street segments
    // they are connected to
    m1Global.intersectionStreetSegments.resize(getNumIntersections());  
    
    for(int intersection = 0; intersection < getNumIntersections(); ++intersection)
    {
       int streetSegments = getNumIntersectionStreetSegment(intersection); // the no. of street segments which are intersecting at one intersection_id
    
        // accessing the streetId's of all those streets that are connected to one common intersection
        for(int i=0; i<streetSegments; i++)
        {
            StreetSegmentIdx streetSegmentIds = getIntersectionStreetSegment(i, intersection); 

            // ADDING STREET SEGMENT TO INTERSECTION VECTOR
            // a vector created which holds all the streetId's for one intersection (has O(Ns), as going through all intersections and then hooking up all the streetSegId's to each intersection)
            m1Global.intersectionStreetSegments[intersection].push_back(streetSegmentIds); 
        } 
    } 
}


void getIntersectionsofStreet(){    
    // a vector that will hold all the intersections on each street
    m1Global.streetIntersections.resize(getNumStreets());
    
    for(int intersection = 0; intersection < getNumIntersections(); ++intersection)
    {
       int streetSegments = getNumIntersectionStreetSegment(intersection); // the no. of street segments which are intersecting at one intersection_id
    
        // accessing the streetId's of all those streets that are connected to one common intersection
        for(int i=0; i<streetSegments; i++)
        {
            StreetSegmentIdx streetSegmentIds = getIntersectionStreetSegment(i, intersection); 

            // ADDING INTERSECTION TO STREET VECTOR
            // find the street that this street segment belongs to, and add this
            // intersection to the corresponding streetID in streetIntersections vector
            StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegmentIds);
            StreetIdx sIdx = segInfo.streetID;

            m1Global.streetIntersections[sIdx].insert(intersection);
        } 
    } 
}


void getStreetSegmentsofStreet(){
    // vector that will hold all the street segments on a given street
    m1Global.streetStreetSegments.resize(getNumStreets());
    
    for(int streetSegment = 0; streetSegment < getNumStreetSegments(); ++streetSegment)
    {
        StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegment);
        StreetIdx sIdx = segInfo.streetID;

        // ADDING STREET SEGMENT TO STREET VECTOR
        m1Global.streetStreetSegments[sIdx].insert(streetSegment);
    } 
}


void getStreetNamesofStreet(){
    // add all street names with no spaces and lowercase to the multimap 
    // the multimap will take care of sorting
    for(int streetID = 0; streetID < getNumStreets(); streetID++){
        std::string streetName = getStreetName(streetID);
        
        // erases all spaces in string -- O(n)
        streetName.erase(std::remove(streetName.begin(), streetName.end(), ' '), streetName.end());
        
        // converts string to lowercase -- O(n)
        boost::algorithm::to_lower(streetName);
        
        m1Global.streetNames.insert(std::pair<std::string, StreetIdx>(streetName, streetID));
    }
}

//insert all the OSMIDs'[by using osmNode->id()] and its LatLon Coordinates in an unordered_map 
void getLatLonofOSMNodes(){
    for(int i = 0; i < getNumberOfNodes(); i++){
        const OSMNode* osmNode = getNodeByIndex(i);
        std::pair<OSMID, LatLon> findingLatLonOSMID (osmNode->id(), getNodeCoords(osmNode));
        m1Global.findingLatlonOfOSMNodes.insert(findingLatLonOSMID);
        
        std::pair<OSMID, const OSMNode*> osmNodeFromID (osmNode->id(), osmNode);
        m1Global.OSMNodesFromOSMID.insert(osmNodeFromID);
    }
    
    for(int i = 0; i < getNumberOfWays(); i++){
        const OSMWay* osmWay = getWayByIndex(i);
        std::pair<OSMID, const OSMWay*> osmWayFromID (osmWay->id(), osmWay);
        m1Global.OSMWaysFromOSMID.insert(osmWayFromID);
    }
}
//find the distance using the findStreetSegmentLength function for each streetSegment ID 
//find the speedLimit for each segment 
//Then calculate the times' needed to traverse each street segment and store them in a vector
void calculateStreetSegmentTravelTimes(){
    
    m1Global.travelTime.resize(getNumStreetSegments());
    
    for(StreetSegmentIdx i=0; i<getNumStreetSegments(); i++)
    {
        double distance = findStreetSegmentLength(i); 
        StreetSegmentInfo streetSegment = getStreetSegmentInfo(i); 
        double time= (distance / streetSegment.speedLimit); 
        m1Global.travelTime[i] = time;
    }
}

// Return the LatLon of an OSM node; will only be called with OSM nodes (not ways or relations)
// Speed requirement --> high

//find the OSMid we want in the unordered_map using an iterator 
//return the LatLOn coordinates of the OSMid 
LatLon findLatLonOfOSMNode (OSMID OSMid) {
    std::unordered_map<OSMID, LatLon>::iterator it = m1Global.findingLatlonOfOSMNodes.find(OSMid);
    return ((*it).second);
}


// takes a string color/color code and sets the colour to be drawn
ezgl::color getSubwayLineColor(std::string color) {

    if (color == "green") return ezgl::GREEN;
    else if (color == "yellow") return ezgl::YELLOW;
    else if (color == "purple") return ezgl::PURPLE;
    else if (color == "red") return ezgl::RED;
    else if (color == "orange") return ezgl::ORANGE;
    else if (color == "blue") return ezgl::BLUE;
    else if (color == "darkgreen") return ezgl::DARK_GREEN;
    else if (color == "pink") return ezgl::PINK;
    else if (color == "gray") return ezgl::GREY_55;
    else if (color == "brown") return ezgl::SADDLE_BROWN;
    else {
        // remove # from beginning of colour code
        color = color.substr(1, color.size() - 1);
        
        // begin conversion from string to hex
        unsigned long color_code = strtoul(color.c_str(), NULL, 16);
        
        // last two digits = blue
        uint_fast8_t color_code_b = color_code % 256;
        
        // middle two digits = green
        uint_fast8_t color_code_g = (color_code / 256) % 256;
        
        // first two digits = red
        uint_fast8_t color_code_r = color_code / (256 * 256);
        
        return {color_code_r, color_code_g, color_code_b};
    }
}


void loadSubwayStations() {
    std::vector<const OSMRelation *> osmSubwayLines;
    
    // loop through all OSM relations
    for(int i = 0; i < getNumberOfRelations(); i++){
        const OSMRelation *currRelation = getRelationByIndex(i);
        
        // check tag of current relation
        for(int j = 0; j < getTagCount(currRelation); j++){
            std::pair<std::string, std::string> tagPair = getTagPair(currRelation, j);
            
            // push relations with the route = subway tag
            if(tagPair.first == "route" && tagPair.second == "subway"){
                osmSubwayLines.push_back(currRelation);
                // no need to check the rest of the tags
                break;
            }
        }
    }

    ezgl::color ezglColor;
    
    // for each subway line (relation), get its color and members
    for(int i = 0; i < osmSubwayLines.size(); i++){
        bool colourAdded = false;
        // get subway line color
        for(int j = 0; j < getTagCount(osmSubwayLines[i]); j++){
            std::pair<std::string, std::string> tagPair = getTagPair(osmSubwayLines[i], j);
            
            if(tagPair.first == "colour"){
                colourAdded = true;
                ezglColor = getSubwayLineColor(tagPair.second);
                m1Global.subwayLineColors.push_back(ezglColor);
                break;
            }
        }
        
        if(colourAdded == false) m1Global.subwayLineColors.push_back(ezgl::BLACK);
        
        // get relation members
        std::vector<TypedOSMID> routeMembers = getRelationMembers(osmSubwayLines[i]);
        
        // all points on a subway line
        std::vector<SubwayWayPointData> wayLocations;
        
        // loop through all members in a relation (subway line)
        for(int j = 0; j < routeMembers.size(); j++){
            // a node is a subway station
            if(routeMembers[j].type() == TypedOSMID::Node){              
                // find the node pointer
                const OSMNode *currNode = m1Global.OSMNodesFromOSMID.find(routeMembers[j])->second;
                
                // set up the subway station struct
                SubwayStationData subwayStationData;
                
                // get the name tag of that node
                for(int k = 0; k < getTagCount(currNode); k++){
                    std::pair<std::string, std::string> tagPair = getTagPair(currNode, k);
                    
                    if(tagPair.first == "name"){
                        subwayStationData.name = tagPair.second;
                        break;
                    }
                }
                
                subwayStationData.color = ezglColor;
                
                // get location of the node
                LatLon subwayCoords = getNodeCoords(currNode);
                
                subwayStationData.x = convertLonToX(subwayCoords.longitude());
                subwayStationData.y = convertLatToY(subwayCoords.latitude());
                
                m1Global.subwayStationLocations.push_back(subwayStationData);
            }

            // a way is a path between subway stations
            if(routeMembers[j].type() == TypedOSMID::Way){
                // find the OSMWay
                const OSMWay *currWay = m1Global.OSMWaysFromOSMID.find(routeMembers[j])->second;

                const std::vector<OSMID>& wayMembers = getWayMembers(currWay);

                for (int m = 0; m < wayMembers.size(); m++) {
                    LatLon coord = findLatLonOfOSMNode(wayMembers[m]);

                    double x = convertLonToX(coord.longitude());
                    double y = convertLatToY(coord.latitude());

                    SubwayWayPointData wayPointData;
                    wayPointData.x = x;
                    wayPointData.y = y;
                    if (m == 0) wayPointData.beginOfWay = true;

                    wayLocations.push_back(wayPointData);
                }
            }
        }
        m1Global.subwayLineLocations.push_back(wayLocations);
    }
}

void getFeatureAreas(){
    int numFeature = getNumFeatures();
    
    for(int i = 0; i < numFeature; i++){       
        double featureArea = findFeatureArea(i);    
        m1Global.sortFeature.push_back(std::make_pair(featureArea, i)); 
    }
    
     std::sort(m1Global.sortFeature.rbegin(), m1Global.sortFeature.rend());
}


void closeMap() {
    mapLoaded = false;    
    
    // clear global vectors
    m1Global.intersectionStreetSegments.clear();
    m1Global.streetIntersections.clear();
    m1Global.streetStreetSegments.clear();
    m1Global.streetNames.clear();
    m1Global.findingLatlonOfOSMNodes.clear();
    m1Global.travelTime.clear();
    m1Global.subwayStationLocations.clear();
    m1Global.subwayLineColors.clear();
    m1Global.subwayLineLocations.clear();
    m1Global.sortFeature.clear();
    m1Global.OSMNodesFromOSMID.clear();
    m1Global.OSMWaysFromOSMID.clear();
    m3Global.drawPathDirections.clear();
    m3Global.path.clear();
    m3Global.pathDirections.clear();
    
    closeStreetDatabase();
    closeOSMDatabase();    
}
