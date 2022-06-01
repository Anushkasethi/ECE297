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

// Change color of features, street name, streets into dark mode
void toggleDarkMode(GtkWidget */*widget*/, ezgl::application *application){
//    auto const startTime = std::chrono::high_resolution_clock::now();
    m2Global.darkMode = !m2Global.darkMode;
    application->refresh_drawing();
//     profile_csv(startTime,"toggleDarkMode");
}