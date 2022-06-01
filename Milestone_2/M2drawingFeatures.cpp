/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */

#include "m1.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "ezgl/color.hpp"
#include "ezgl/camera.hpp"
#include "StreetsDatabaseAPI.h"
#include <cmath>

extern M2Global m2Global;
extern M1Global m1Global;


// Draw all the features on the map in different color; 
void drawFeature(ezgl::renderer *g){
//    auto const startTime = std::chrono::high_resolution_clock::now();
    ezgl::rectangle visibleWorld = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();
        
        // iterate through all features, sorted by size decreasing
        for(int i = 0; i < m1Global.sortFeature.size(); i++){
            std::vector<ezgl::point2d> points;
            
            // get the feature ID
            int num = m1Global.sortFeature[i].second; 
            int numPoints = getNumFeaturePoints(num);
            
            // get the feature type
            FeatureType checkFeatureType = getFeatureType(num);
            
                // get the LatLon of each point in one feature
                for (int n = 0; n < numPoints; n++){
                    LatLon pointPosition = getFeaturePoint(n, num);
                    
                    double xCoord = convertLonToX(pointPosition.longitude());
                    double yCoord = convertLatToY(pointPosition.latitude());
                    
                    points.push_back(ezgl::point2d(xCoord,yCoord));   
                }
        
        
           // check if there are more than one point in one feature 
           if (points.size()>1){
               // check the type of the feature
               if (checkFeatureType == RIVER){
                   if(m2Global.darkMode){
                       g->set_color(ezgl::RIVER_DARK);
                   }else{
                       g->set_color(78,131,162,255);
                   }                 
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == LAKE){
                   if(m2Global.darkMode){
                       g->set_color(50, 83, 107);
                   }else{
                       g->set_color(96,162,184,255);
                   }
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == BEACH){
                    if(m2Global.darkMode){
                        g->set_color(128, 116, 79);
                    }else{
                        g->set_color(224,195,106,255);
                    }
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == PARK){
                   if(m2Global.darkMode){
                       g->set_color(ezgl::GREEN_DARK);
                   }else{
                       g->set_color(131,188,112,255);
                   }
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == ISLAND){
                    if(m2Global.darkMode){
                        g->set_color(140, 135, 94);
                    }else{
                        g->set_color(199,192,138,255);
                    }
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == BUILDING && zoomLevel >400){                  
                    g->set_color(167,167,167,255);
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == GREENSPACE){
                    if(m2Global.darkMode){
                        g->set_color(110, 130, 66);
                    }else{
                        g->set_color(169,201,100,255);
                    }
                    g->fill_poly(points);
                    
                } else if (checkFeatureType == GOLFCOURSE){
                    if(m2Global.darkMode){
                        g->set_color(83, 122, 73);
                    }else{
                        g->set_color(136,200,120,255);
                    }
                    g->fill_poly(points);
                } else if (checkFeatureType == STREAM && zoomLevel >50){
                    if(m2Global.darkMode){
                        g->set_color(64, 81, 107);
                    }else{
                        g->set_color(102,136,163,255); 
                    }
                    for (int j = 0; j < numPoints-1; j++){
                        g->draw_line(points[j],points[j+1]);
                    }
                } else if (checkFeatureType == GLACIER){
                     if(m2Global.darkMode){
                         g->set_color(155, 155, 155);
                     }else{
                        g->set_color(255,255,255,255);
                     }
                    g->fill_poly(points);
                }
                
            }
   
    }
//    profile_csv(startTime,"drawFeature");
}

