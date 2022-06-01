#include "m1.h"
#include "globals.h"
#include "StreetsDatabaseAPI.h"
#include "m3.h"
#include <queue>

//Storing intersection id one node and street segment id used to get the id

struct WaveElem {
    IntersectionIdx nodeID;
    StreetSegmentIdx edgeID; // edge used to reach this node
    double travelTime; // Total travel time to reach node

    WaveElem(int n, int e, double time) {
        nodeID = n;
        edgeID = e;
        travelTime = time;
    }

    //sorting the priority queue based on travelTime
    bool operator<(const WaveElem & rhs) const {
        return this->travelTime > rhs.travelTime;
    }
};

class Node {
public:
    std::vector<StreetSegmentIdx> outEdge; // Outgoing edges etc.
    StreetSegmentIdx reachingEdge; // edge used to reach node
    double bestTime; // Shortest time found to this node so far
};

//A vector storing the outEdges and reachingEdge of each intersection ID
std::vector<Node> nodes;

//Tells if there exist a path between start point and end point
bool pathExist(int srcID, int destID, const double turn_penalty) {
   

    std::priority_queue<WaveElem> wavefront;
    // initialize list of intersections to visit
    // first intersection: starting point
    //                     no previous edge, travel time = 0
    wavefront.push(WaveElem(srcID, -1, 0));

    //checking for duplicates
    std::vector<bool> explored(getNumIntersections(), false);
    nodes.resize(getNumIntersections() // resizing the nodes vector
#if 0
            ,{std::vector<StreetSegmentIdx>(), 0, std::numeric_limits<double>::max()}
#endif
    );
    
    //setting the bestTime to have the maximum double value
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].bestTime = std::numeric_limits<double>::max();
    }
    
    // node for starting point does not have any reachingEdge so set it = -1
    nodes[srcID].reachingEdge = -1;

    while (!wavefront.empty()) {
        WaveElem curr = wavefront.top(); //get the first element in the priority queue
        wavefront.pop(); //remove that element which was stored in curr
       
        IntersectionIdx currID = curr.nodeID;

        //only update the reachingEdge if the travelTime to get to that node is less than the 
        //stored bestTime. Also update the bestTime if travelTime is lesser.
        if (nodes[currID].bestTime > curr.travelTime) {
            nodes[currID].reachingEdge = curr.edgeID;
            nodes[currID].bestTime = curr.travelTime;
        } 


        // return true if destination found
        if (currID == destID) {
            return true;
        }

        //if its a node which was checked earlier skip it to avoid duplicate checking
        if (explored[currID]) {
            continue;
        }
        explored[currID] = true;
        
        // add nodes to visit - OUTEDGES
        std::vector<StreetSegmentIdx> toVisit = findStreetSegmentsOfIntersection(currID);
        for (int i = 0; i < toVisit.size(); i++) {
           
            StreetSegmentInfo segInfo = getStreetSegmentInfo(toVisit[i]);
            IntersectionIdx toID = -1;

            // To check which intersection are we on and which one are we visiting:
            //Also checking for oneWay streets:
            if (segInfo.oneWay) {
                if (curr.nodeID == segInfo.from)
                    toID = segInfo.to;
                else if (curr.nodeID == segInfo.to)
                    continue;
            } else {
                if (curr.nodeID == segInfo.from)
                    toID = segInfo.to;
                else if (curr.nodeID == segInfo.to)
                    toID = segInfo.from;
            }
            
            if (explored[toID]) {
                continue;
            }
            
            // find total time to travel to that intersection from this node
            double travelTimeFromHere = nodes[currID].bestTime + findStreetSegmentTravelTime(toVisit[i]);
            
            //checking when the streetID changes and adding the turn penalties accordingly
            //make sure to not add any turn penalty is the currID is the srcID
            if (currID!=srcID && segInfo.streetID != getStreetSegmentInfo(curr.edgeID).streetID) {

                travelTimeFromHere += turn_penalty;
            }
       
            wavefront.push(WaveElem(toID, toVisit[i], travelTimeFromHere));

        }
    }
    return (false); // No path exists!
}

//Tracing backwards from destination to the starting point
//to find the shortest path obtained between 2 intersections
std::vector<StreetSegmentIdx> bfsTraceBack(int destID) {
    std::vector<StreetSegmentIdx> path;
   
    int currNodeID = destID;
    StreetSegmentIdx prevEdge = nodes[currNodeID].reachingEdge;
   
    //Keep going until you reaching the starting point which has the prevEdge = -1
    while (prevEdge != -1) {
        path.insert(path.begin(), prevEdge); //add all the edges used to travel between the intersections
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(prevEdge);
        
        //check if the nodeID is from or to:
        if (segmentInfo.to == currNodeID)
            currNodeID = segmentInfo.from; //currNodeID = node at other end of prevEdge;  
        else
            currNodeID = segmentInfo.to;
        prevEdge = nodes[currNodeID].reachingEdge;
    }
    return path; 
}

//Calculating travel time of a route

double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path) {
    double travelTime = 0;
    
    if(path.empty()) return 0;
    
    
    StreetSegmentInfo streetInfo0 = getStreetSegmentInfo(path[0]);
    StreetIdx prevStreetId = streetInfo0.streetID; //store the first streetID in the found path
    
    //traverse the rest of the path and add the travelTime for each streetID 
    for (int i = 0; i < path.size(); i++) {
        StreetSegmentInfo streetInfo = getStreetSegmentInfo(path[i]);
        StreetIdx streetId = streetInfo.streetID;
        //StreetIdx prevStreetId = streetId;
        travelTime += findStreetSegmentTravelTime(path[i]);
        
        //check if the streetID's are different then add the turn penalties:
        if (prevStreetId != streetId) {
            prevStreetId = streetId;
            travelTime += turn_penalty;
        }
    }
    return travelTime;
}

//Return a vector that stores the streetSegment id of one path
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty,
        const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids) {
    std::vector<StreetSegmentIdx> pathFound = {};
    
    //if path exists then return the path:
    if (pathExist(intersect_ids.first, intersect_ids.second, turn_penalty))
        return bfsTraceBack(intersect_ids.second);
    else
        return pathFound;
}