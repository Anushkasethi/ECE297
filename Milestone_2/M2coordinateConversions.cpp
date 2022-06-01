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

//(x, y) = (R ·lon ·cos(latavg), R ·lat)
double convertYToLat(double y) {
    double lat = y / (kEarthRadiusInMeters * kDegreeToRadian);
    return lat;
}

double convertXToLon(double x) {
    double lon = x / (kEarthRadiusInMeters * m2Global.cosAveLat * kDegreeToRadian);
    return lon;
}

double convertLonToX(double Lon){
    double x = kEarthRadiusInMeters * Lon * kDegreeToRadian * m2Global.cosAveLat;
    return x;
}

double convertLatToY(double Lat){
    double y = kEarthRadiusInMeters * Lat * kDegreeToRadian;
    return y;
}
