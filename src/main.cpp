// Purpose: Coordinates campus file loading and the Milestone 1 console menu.
// Author: Rami Badr
#include "CampusGraph.h"
#include "DepartmentLoader.h"
#include "DepartmentMenu.h"
#include "DepartmentTree.h"
#include "FacilityDatabase.h"
#include "TextUtils.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
// Builds paths from the selected directory without machine-specific locations.
std::string dataPath(const std::string& directory, const std::string& filename) {
    const char last = directory.back();
    return directory + ((last == '/' || last == '\\') ? "" : "/") + filename;
}

// Keeps exactly the Milestone 1 numbering, including option 0 for exit.
void displayMenu() {
    std::cout << "\nCampus Navigation Menu\n"
              << "1. View all facilities\n"
              << "2. Search facility by ID\n"
              << "3. View campus graph\n"
              << "4. Display graph counts and degrees\n"
              << "5. Find reachable buildings (BFS)\n"
              << "6. Add department\n"
              << "7. Search department\n"
              << "8. Display department hierarchy\n"
              << "9. Run preorder traversal\n"
              << "0. Exit\n"
              << "Choose an option: ";
}
}

// Loads modules in dependency order and delegates operations to their owners.
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: campus [data-directory]\n";
        return 1;
    }
    const std::string dataDirectory = argc == 2 ? argv[1] : "data/";
    if (TextUtils::trim(dataDirectory).empty()) {
        std::cerr << "The data directory cannot be blank.\n";
        return 1;
    }
    std::cout << "Smart Campus Navigation and Facility Analytics System\n"
              << "Data directory: " << dataDirectory << '\n';

    CampusGraph graph;
    FacilityDatabase facilities;
    DepartmentTree departments;
    graph.loadBuildings(dataPath(dataDirectory, "buildings.txt"), "buildings.txt");
    graph.loadConnections(dataPath(dataDirectory, "connections.txt"), "connections.txt");
    std::vector<std::string> acceptedBuildings;
    for (int index = 0; index < graph.buildingCount(); ++index) {
        acceptedBuildings.push_back(graph.buildingName(index));
    }
    facilities.loadFromFile(dataPath(dataDirectory, "facilities.txt"), acceptedBuildings, std::cout);
    DepartmentLoader::load(dataPath(dataDirectory, "departments.txt"), departments, std::cout);

    while (true) {
        displayMenu();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        std::istringstream input(line);
        int choice = -1;
        if (!(input >> choice) || !(input >> std::ws).eof() || choice < 0 || choice > 9) {
            std::cout << "Invalid menu choice. Enter a whole number from 0 to 9.\n";
            continue;
        }
        if (choice == 0) break;

        switch (choice) {
            case 1:
                facilities.displayAll(std::cout);
                break;
            case 2:
                std::cout << "Facility ID: ";
                if (std::getline(std::cin, line)) facilities.displaySearchById(line, std::cout);
                break;
            case 3:
                graph.displayGraph();
                break;
            case 4:
                graph.displayCountsAndDegrees();
                break;
            case 5:
                bfsReachableInteractive(graph);
                break;
            case 6:
                DepartmentMenu::addDepartment(departments, std::cin, std::cout);
                break;
            case 7:
                DepartmentMenu::searchDepartment(departments, std::cin, std::cout);
                break;
            case 8:
                DepartmentMenu::displayHierarchy(departments, std::cout);
                break;
            case 9:
                departments.printPreorder(std::cout);
                break;
        }
    }
    std::cout << "Goodbye.\n";
    return 0;
}
