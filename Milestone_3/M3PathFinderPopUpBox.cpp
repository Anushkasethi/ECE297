
#include "m1.h"
#include "globals.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include "StreetsDatabaseAPI.h"
#include "m3.h"
#include <cmath>
#include <sstream>
#include <iomanip>


extern M1Global m1Global;
extern M2Global m2Global;
extern M3Global m3Global;


// sets up the pop up box with start, dest, find, and reset buttons
// Needs to:
//          Set the first intersection to the **first highlighted intersection**
//          and print intersection on bottom of screen
//          Set the second intersection to the **first highlighted intersection**
//          and print on bottom of screen
//          Send both intersections to pathfinding function when Find Path
//          is clicked
//          Clear both after path is found or when Reset/Start/Dest are clicked

void navigationPopUpBox(GtkWidget */*widget*/, ezgl::application *application) {
    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog;

    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());

    // create the dialog window
    // modal windows prevent interaction with other windows in the same application
    dialog = gtk_dialog_new_with_buttons("PathFinder", (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("Set Start"), (GtkDialogFlags) 1,
            ("Set Destination"), (GtkDialogFlags) 2,
            ("Find Path"), (GtkDialogFlags) 3,
            ("Reset"), (GtkDialogFlags) 4,
            NULL);

    // create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(""
            "1) Set your starting point by searching for it and clicking\n"
            "Find Intersection, or by clicking it on the map, and click Set\n"
            "Start.\n\n"
            "2) Do the same with your destination.\n\n"
            "3) Finally, press Find Path.\n\n"
            "4) You can view directions to your destination by clicking Next\n"
            "Direction.\n\n"
            "5) To reset the path, click any button.\n\n"
            "Thank you for using our map!");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    // the main purpose of this is to show dialog's child widget, label
    gtk_widget_show_all(dialog);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    switch (result) {
        case (GtkDialogFlags) 1: // set start
            m3Global.setFirst = true;
            setIntersection(application);
            m3Global.setFirst = false;
            break;
        case (GtkDialogFlags) 2: // set dest
            m3Global.setSecond = true;
            setIntersection(application);
            m3Global.setSecond = false;
            break;
        case(GtkDialogFlags) 3: // find path
            findPathGivenInput(application);
            break;
        case(GtkDialogFlags) 4: // reset
            resetIntersections(application);
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            break;
        default:
            break;
    }
    gtk_widget_destroy(dialog);
}


// check the highlighted intersections and return the first one

void setIntersection(ezgl::application *application) {
    std::stringstream ss;

    if (m2Global.intersectionsHighlight.size() == 0) {
        ss << "Please select an intersection";

    } else {
        if (m3Global.setFirst) {
            m3Global.start = m2Global.intersectionsHighlight[0];
            ss << "Start ";
        } else if (m3Global.setSecond) {
            m3Global.dest = m2Global.intersectionsHighlight[0];
            ss << "Destination ";
        }
        ss << "is " << getIntersectionName(m2Global.intersectionsHighlight[0]);
    }
    m3Global.path.clear();
    m3Global.pathDirections.clear();
    m3Global.drawPathDirections.clear();

    application->update_message(ss.str());
    application->refresh_drawing();
}


// pass in two intersections as input and receive path between them

void findPathGivenInput(ezgl::application *application) {
    // check if user inputted start & destination
    if(m3Global.dest == -1 || m3Global.start == -1){
        std::stringstream ss;
        ss << "Please select a start and destination!";
        application->update_message(ss.str());
        application->refresh_drawing();
        
        return;
    }
    std::pair<IntersectionIdx, IntersectionIdx> startDest(m3Global.start, m3Global.dest);
    m3Global.path = findPathBetweenIntersections(15, startDest); // assuming turn penalty is 15 seconds

    std::stringstream ss;

    // user selected start and dest as same intersection
    if (m3Global.start == m3Global.dest) {
        ss << "Route from " << getIntersectionName(m3Global.start) << " to "
                << getIntersectionName(m3Global.dest);

        m3Global.pathDirections.clear();
        m3Global.pathDirections.push_back("Looks like you're already at your destination. Congrats!");
        m3Global.pathDirections.push_back("But what is a destination?");
        m3Global.pathDirections.push_back("Where are you headed in life? Where is anyone even headed in life?");
        m3Global.pathDirections.push_back("I'm almost 20 and I still feel lost.");
        m3Global.pathDirections.push_back("Hopefully you are feeling better than me about your goals and aspirations.");
        m3Global.pathDirections.push_back("But anyways, you can set another start/destination if you'd like to go somewhere else.");
        m3Global.pathDirections.push_back("I also created a Find Professor Cohen game if you want to have some fun.");
        m3Global.pathDirections.push_back("Have a great day! :)");

        // path does not exist
    } else if (m3Global.path.empty()) {
        ss << "Path does not exist!";
        application->update_message(ss.str());
        application->refresh_drawing();

    } else {
        getPathDirections(m3Global.path);

        // travel time in seconds, with a 15s turn penalty
        double travelTime = computePathTravelTime(15, m3Global.path);

        ss << "Route from " << getIntersectionName(m3Global.start) << " to "
                << getIntersectionName(m3Global.dest) << " - total travel time is "
                << convertTravelTime(travelTime);
    }
    application->update_message(ss.str());
    application->refresh_drawing();
}


// reset start and destination

void resetIntersections(ezgl::application * application) {
    m3Global.start = -1;
    m3Global.dest = -1;
    m3Global.path.clear();
    m3Global.pathDirections.clear();
    m3Global.drawPathDirections.clear();

    std::stringstream ss;
    ss << "";

    application->update_message(ss.str());
    application->refresh_drawing();
}


// print the next direction on the screen and update the section of road the
// user is traveling on

void printDirectionsOnScreen(GtkWidget */*widget*/, ezgl::application *application) {
    std::stringstream ss;

    if (m3Global.pathDirections.size() == 0) {
        ss << "";
    } else {

        ss << m3Global.pathDirections[0];
        m3Global.pathDirections.erase(m3Global.pathDirections.begin());
    }

    // update section of road that user is on
    if (!(m3Global.drawPathDirections.empty())) {
        m3Global.drawPathDirections.erase(m3Global.drawPathDirections.begin());
    }

    application->update_message(ss.str()); // Message at screen bottom
    application->refresh_drawing();
}
