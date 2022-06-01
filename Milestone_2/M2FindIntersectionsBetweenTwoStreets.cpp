
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
#include <unordered_set>
#include <chrono>



extern M2Global m2Global;
extern M1Global m1Global;

double latlonY(double y);
double latlonX(double x);



// Create the find button to print the intersections of two streets that the user search

void find_button(GtkWidget */*widget*/, ezgl::application *application) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
//    auto const start = std::chrono::high_resolution_clock::now();

    // Get the GtkEntry object
    GtkEntry* text_entry = (GtkEntry *) application->get_object("streetNameEntry1");

    // Get the text written in the widget
    const char* street1 = gtk_entry_get_text(text_entry);

    // Get the GtkEntry object
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("streetNameEntry2");

    // Get the text written in the widget
    const char* street2 = gtk_entry_get_text(text_entry2);

    std::stringstream ss;

    std::string streetFirst = (std::string)street1;
    std::string streetSecond = (std::string)street2;

    // get the street id of each street input
    std::vector<StreetIdx> streetOne = findStreetIdsFromPartialStreetName(streetFirst);
    std::vector<StreetIdx> streetTwo = findStreetIdsFromPartialStreetName(streetSecond);


    // invalid input
    if (streetOne.size() == 0 || streetTwo.size() == 0) {
        ss << "Two streets were not inputted";

        application->update_message(ss.str()); // Message at screen bottom
        application->refresh_drawing();

        return;
    }
 

    // find all possible intersections for street one...
    std::unordered_set<IntersectionIdx> streetOneIntersections;
    for (int i = 0; i < streetOne.size(); i++) {
        std::unordered_set<IntersectionIdx> stOneIdxInts = m1Global.streetIntersections[streetOne[i]];

        for (auto it = stOneIdxInts.begin(); it != stOneIdxInts.end(); it++) {
            streetOneIntersections.insert(*it);
        }
    }

    // and all possible intersections for street two...
    std::unordered_set<IntersectionIdx> streetTwoIntersections;
    for (int i = 0; i < streetTwo.size(); i++) {
        std::unordered_set<IntersectionIdx> stTwoIdxInts = m1Global.streetIntersections[streetTwo[i]];

        for (auto it = stTwoIdxInts.begin(); it != stTwoIdxInts.end(); it++) {
            streetTwoIntersections.insert(*it);
        }
    }

    // and find common intersections
    
    // reset list of intersections to be highlighted
    m2Global.intersectionsHighlight.clear();
    for (auto it = streetOneIntersections.begin(); it != streetOneIntersections.end(); it++) {
        auto findMe = streetTwoIntersections.find(*it);

        if (findMe != streetTwoIntersections.end()) {
            m2Global.intersectionsHighlight.push_back(*it);
        }
    }

    // Check if the streets intersect
    if (m2Global.intersectionsHighlight.size() == 0) {
        ss << "Streets do not intersect";
        application->update_message(ss.str());
        application->refresh_drawing();
        return;
    }

    ss << "Intersection Names: ";

    for (int i = 0; i < m2Global.intersectionsHighlight.size(); i++) {
        std::string getName = getIntersectionName(m2Global.intersectionsHighlight[i]);
        ss << getName;

        if (i < m2Global.intersectionsHighlight.size() - 1) ss << ", ";

//        m2Global.intersections[m2Global.intersectionsHighlight[i]].highlight = true;


    }
    application->update_message(ss.str());
    application->refresh_drawing();
    //    auto const end = std :: chrono :: high_resolution_clock :: now();
    //    auto const delta_time = std:: chrono :: duration_cast <std :: chrono :: seconds > (end-start);
    //    std::cout << "Find_button took: "<< delta_time.count()<<"s\n";
    //    profile_csv(startTime,"find_button");

}

// list the possible street names under the text entry bar 
//according to the partial street names

void streetNameEntryChanged1(GtkEntry* self, gpointer user_data) {
    //    auto const startTime = std::chrono::high_resolution_clock::now();

    auto ezgl_app = static_cast<ezgl::application *> (user_data);
    auto streetNameEntryListStore1 = (GtkListStore *) ezgl_app->get_object("streetNameEntryListStore1");

    gtk_list_store_clear(streetNameEntryListStore1);

    GtkTreeIter iter;
    const gchar* userInput = gtk_entry_get_text(self);


    std::vector<StreetIdx> autocompleteStreet = findStreetIdsFromPartialStreetName(std::string(userInput));

    std::vector<std::string> uniqueNames;
    for (int i = 0; i < autocompleteStreet.size(); i++) {
        // only display unique street names
        if (std::find(uniqueNames.begin(), uniqueNames.end(),
                getStreetName(autocompleteStreet[i])) != uniqueNames.end()) {
            continue;
        }

        // only display the first 6 unique results
        if (uniqueNames.size() == 6) break;

        gtk_list_store_append(streetNameEntryListStore1, &iter);
        gtk_list_store_set(streetNameEntryListStore1, &iter, 0, getStreetName(autocompleteStreet[i]).c_str(), -1);

        uniqueNames.push_back(getStreetName(autocompleteStreet[i]));
    }
    //    profile_csv(startTime,"streetNameEntryChanged1");
}

// list the possible street names under the text entry bar 
//according to the partial street names

void streetNameEntryChanged2(GtkEntry* self, gpointer user_data) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    auto ezgl_app = static_cast<ezgl::application *> (user_data);
    auto streetNameEntryListStore2 = (GtkListStore *) ezgl_app->get_object("streetNameEntryListStore2");

    gtk_list_store_clear(streetNameEntryListStore2);

    GtkTreeIter iter;
    const gchar* userInput = gtk_entry_get_text(self);


    std::vector<StreetIdx> autocompleteStreet = findStreetIdsFromPartialStreetName(std::string(userInput));

    std::vector<std::string> uniqueNames;
    for (int i = 0; i < std::min((int) autocompleteStreet.size(), 6); i++) {
        // only display unique street names
        if (std::find(uniqueNames.begin(), uniqueNames.end(),
                getStreetName(autocompleteStreet[i])) != uniqueNames.end()) {
            continue;
        }

        // only display the first 6 unique results
        if (uniqueNames.size() == 6) break;

        gtk_list_store_append(streetNameEntryListStore2, &iter);
        gtk_list_store_set(streetNameEntryListStore2, &iter, 0, getStreetName(autocompleteStreet[i]).c_str(), -1);

        uniqueNames.push_back(getStreetName(autocompleteStreet[i]));
    }
//    profile_csv(startTime, "streetNameEntryChanged2");
}

//redraw the map according to the city the user chooses

void selectCountry(GtkComboBox *widget, ezgl::application* application) {
//    auto const startTime = std::chrono::high_resolution_clock::now();
    const gchar* active_id = gtk_combo_box_get_active_id(widget);

    std::string country = (std::string)active_id;
    std::string fullPath = "/cad2/ece297s/public/maps/" + country + ".streets.bin";

    m2Global.currentCountry = country;

    closeMap();

    loadMap(fullPath);

    ezgl::rectangle initial_world({m2Global.minX, m2Global.minY},
    {
        m2Global.maxX, m2Global.maxY
    });

    application->change_canvas_world_coordinates("MainCanvas", initial_world);
    application->refresh_drawing();
//    profile_csv(startTime, "selectCountry");

}

// initial set up for call back functions related to creating a button and a text entry

void initialSetup(ezgl::application* application, bool /*new_window */) {
    GObject * streetNameEntry1 = application->get_object("streetNameEntry1");
    g_signal_connect(streetNameEntry1, "changed", G_CALLBACK(streetNameEntryChanged1), application);

    GObject * streetNameEntry2 = application->get_object("streetNameEntry2");
    g_signal_connect(streetNameEntry2, "changed", G_CALLBACK(streetNameEntryChanged2), application);

    GObject * getCountry = application->get_object("getCountryEntry");
    g_signal_connect(getCountry, "changed", G_CALLBACK(selectCountry), application);


    application->create_button("Find Intersections", 8, find_button);

    application->create_button("Dark Mode", 9, toggleDarkMode);

    application->create_button("Subways", 10, showSubways);

    application->create_button("Toggle POIs", 11, POIsPopUpBox);

    application->create_button("Where's Prof?", 14, wheresProf);
    
    application->create_button("Path Finder", 15, navigationPopUpBox);
    
    application->create_button("Next Direction", 17, printDirectionsOnScreen);
}
