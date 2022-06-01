#include "LatLon.h"
#include "m1.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "globals.h"
#include <cmath>
#include <string>
#include <sstream>
#include <gtk/gtk.h>
#include <chrono>
extern M2Global m2Global;


void POIsPopUpBox(GtkWidget */*widget*/, ezgl::application *application){
    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog;  
    
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    // create the dialog window
    // modal windows prevent interaction with other windows in the same application
    dialog = gtk_dialog_new_with_buttons("Toggle POIs", (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("Schools"), (GtkDialogFlags) 1,  
            ("Food Places"), (GtkDialogFlags) 2,  
            ("Hospitals"), (GtkDialogFlags) 3,
            NULL);
    
    // create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("Click on a POI button to show its locations on the map.\n"
            "You may need to zoom in more to view the POIs.");
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    // the main purpose of this is to show dialog's child widget, label
    gtk_widget_show_all(dialog);
    
    int result = gtk_dialog_run(GTK_DIALOG (dialog));
    switch(result){
        case (GtkDialogFlags) 1:
            std::cout << "Schools";
            showSchools(application);
            break;
        case (GtkDialogFlags) 2:
            std::cout << "Food Places";
            showFoodPlaces(application);
            break;
        case (GtkDialogFlags) 3:
            std::cout << "Hospitals";
            showHospitals(application);
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. 'X' button)";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << "(" << result << ")\n";
    gtk_widget_destroy(dialog);
}


/*Showing POI's for Schools, hospitals, restaurants:
 Made Buttons for each of those POI in order to display the icons at different zoom levels */

void drawPOI(ezgl::renderer *g) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    ezgl::rectangle visible_world = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visible_world.area(); //calculate the zoom level

    /*get the POI types, names and positions (convert the positions from latlon to x & y coordinates)
     check which type, to display only those POI's on the map
    */
    //if(visible_world.area()){
        //std::cout<<"Area: "<<visible_world.area()<<endl;
        for (int i = 0; i < getNumPointsOfInterest(); i += 10) {
            LatLon poiPosition = getPOIPosition(i);
            std::string poiName = getPOIName(i);
            std::string poiType = getPOIType(i);
          
            double xPos = convertLonToX(poiPosition.longitude());
            double yPos = convertLatToY(poiPosition.latitude());
           if (xPos < visible_world.left() || xPos > visible_world.right() 
                        || yPos > visible_world.top() || yPos < visible_world.bottom()){
                    continue;
                }
            
            //check if the button for schools is ON
            if(m2Global.schoolsOn){
            if((poiType == "school" || poiType == "music_school" ||poiType == "kindergarten" )&& zoomLevel>100){
                   
                 ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/school.png"); // add the same png file for schools/music_school/kindergarten
                 g->draw_surface(png_surface, ezgl::point2d(xPos,yPos), 2);
                 ezgl::renderer::free_surface(png_surface);
            }
            }
            
            //check if the button for foodPlaces is ON 
            if(m2Global.foodPlacesOn){
            if((poiType == "restaurant" )&& zoomLevel>1000){
                 
                 ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/restaurant.png"); // add the png file for restaurants
                 g->draw_surface(png_surface, ezgl::point2d(xPos,yPos),1);
                 ezgl::renderer::free_surface(png_surface);
            }
            }
               
            //check if the button for hospitals is ON
            if(m2Global.hospitalsOn){
            if( (poiType == "pharmacy" || poiType == "doctors "|| poiType == "dentist" )&& zoomLevel > 400){
               
               ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/hospital.png"); // add the same png file for hospitals/pharmacy/denstist
                 g->draw_surface(png_surface, ezgl::point2d(xPos,yPos), 2);
                 ezgl::renderer::free_surface(png_surface);
                }
            }
        }
        
//}
//     profile_csv(startTime,"drawPOI");
}
//Check if the buttons for showing foodPlaces, Schools, Hospitals are ON or OFF
//if they are OFF the don't display the icons for that particular POI, and do the opposite otherwise.

void showSchools(ezgl::application *application){
//    auto const startTime = std::chrono::high_resolution_clock::now();
    m2Global.schoolsOn = !m2Global.schoolsOn;
    application->refresh_drawing();
//     profile_csv(startTime,"showSchools");
}

void showFoodPlaces(ezgl::application *application){
//    auto const startTime = std::chrono::high_resolution_clock::now();
    m2Global.foodPlacesOn = !m2Global.foodPlacesOn;
    application->refresh_drawing();
//     profile_csv(startTime,"showFoodPlaces");
}

void showHospitals(ezgl::application *application){
//    auto const startTime = std::chrono::high_resolution_clock::now();
    m2Global.hospitalsOn = !m2Global.hospitalsOn;
    application->refresh_drawing();
//     profile_csv(startTime,"showHospitals");
}

