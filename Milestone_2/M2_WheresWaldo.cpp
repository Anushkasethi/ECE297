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


extern M2Global m2Global;


void drawCohen(ezgl::renderer *g) {
    ezgl::rectangle visibleWorld = g->get_visible_world();
    float zoomLevel = m2Global.initialWorldArea / visibleWorld.area();

    // don't draw cohen when zoomed out
    if (zoomLevel < 400) return;

    // load cohen
    ezgl::surface *png_surface = ezgl::renderer::load_png(
            "libstreetmap/resources/cohen.png");
    
    double cohenX = m2Global.intersections[m2Global.cohenIntersection].x;
    double cohenY = m2Global.intersections[m2Global.cohenIntersection].y;
    g->draw_surface(png_surface, {cohenX, cohenY}, 0.3);
}


void wheresProf(GtkWidget */*widget*/, ezgl::application *application){
    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog;  
    
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    // create the dialog window
    // modal windows prevent interaction with other windows in the same application
    dialog = gtk_dialog_new_with_buttons("Where's Professor Cohen?", (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("Reset Prof"), (GtkDialogFlags) 1,  
            ("Hint Please!"), (GtkDialogFlags) 2,  
            NULL);
    
    // create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("We've hidden Professor Cohen somewhere on this map!\n"
            "For a hint, press Hint Please!\nTo rehide Professor Cohen, press"
            " Reset Prof.\nGood luck!");
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    // the main purpose of this is to show dialog's child widget, label
    gtk_widget_show_all(dialog);
    
    int result = gtk_dialog_run(GTK_DIALOG (dialog));
    switch(result){
        case (GtkDialogFlags) 1:
            resetCohen(application);
            break;
        case (GtkDialogFlags) 2:
            cohenHint(application);
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            break;
        default:
            break;
    }
    gtk_widget_destroy(dialog);
}


void resetCohen(ezgl::application *application){
    // generate a random intersection to hide Cohen
    m2Global.cohenIntersection = std::rand() % getNumIntersections();
    // generate a known intersection for testing
//    m2Global.waldoIntersection = std::rand() % 30 + 9660;
    
    std::stringstream ss;
    ss << " ";
    application->update_message(ss.str()); // Message at screen bottom
    application->refresh_drawing();
}


void cohenHint(ezgl::application *application){
    ezgl::renderer *g = application->get_renderer();
    ezgl::rectangle visibleWorld = g->get_visible_world();
    
    // zooming in decreases the area of visibleWorld by ~2.77 each time
    //  in other words, decreases length and width by 1.66 -> 2/3
    // increase by 2 zoom levels while keeping Cohen somewhere in the map
    double setWidth = visibleWorld.width() * 4 / 9;
    double setHeight = visibleWorld.height() * 4 / 9;
    
    double cohenX = m2Global.intersections[m2Global.cohenIntersection].x;
    double cohenY = m2Global.intersections[m2Global.cohenIntersection].y;
    
    double left;
    double right;
    double top;
    double bottom;
    
    // if Cohen is not in the visible world, zoom fit
    if((cohenX < visibleWorld.left() || cohenX > visibleWorld.right()) &&
            (cohenY < visibleWorld.bottom() || cohenY > visibleWorld.top())){
        left = m2Global.minX;
        right = m2Global.maxX;
        bottom = m2Global.minY;
        top = m2Global.maxY;

    }else{  
        // semi-randomize by changing the rectangle in which cohen can appear
        //    else, centre rectangle on cohen
        // change width
        if(cohenX > visibleWorld.left() && cohenX < visibleWorld.left() + setWidth){
            left = visibleWorld.left();
            right = visibleWorld.left() + setWidth;

        }else if(cohenX < visibleWorld.right() && cohenX > visibleWorld.right() - setWidth){
            left = visibleWorld.right() - setWidth;
            right = visibleWorld.right();

        }else{
            left = cohenX - (setWidth / 2);
            right = cohenX + (setWidth / 2);
        }  

        // change height
        if(cohenY > visibleWorld.bottom() && cohenY < visibleWorld.bottom() + setHeight){
            bottom = visibleWorld.bottom();
            top = visibleWorld.bottom() + setHeight;

        }else if(cohenY < visibleWorld.top() && cohenY > visibleWorld.top() - setHeight){
            bottom = visibleWorld.top() - setHeight;
            top = visibleWorld.top();

        }else{
            bottom = cohenY - (setHeight / 2);
            top = cohenY + (setHeight / 2);
        }  
    }

    ezgl::rectangle newWorld({left, bottom}, {right, top});

    g->set_visible_world(newWorld);

    std::stringstream ss;
    ss << "Need a hint? Professor Cohen is hidden somewhere in this part of the map.";
    application->update_message(ss.str());
    application->refresh_drawing();
}