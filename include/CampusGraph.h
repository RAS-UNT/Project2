// CampusGraph.h
// Purpose: Undirected, unweighted campus graph (adjacency list) with file loading,
//          display, BFS reachability, shortest path, and connectivity report.
// Author: Sulav Bhattarai (Member B)
#ifndef CAMPUS_GRAPH_H
#define CAMPUS_GRAPH_H

#include <string>
#include <vector>

class CampusGraph {
public:
    CampusGraph();

    // Load buildings.txt / connections.txt. filePath = where to open, displayName = name shown
    // in messages. Each prints its own warnings and its load summary line.
    void loadBuildings(const std::string& filePath, const std::string& displayName);
    void loadConnections(const std::string& filePath, const std::string& displayName);

    int buildingCount() const;
    int connectionCount() const;

    // Name of building number 'index' exactly as spelled in buildings.txt (index from findBuilding).
    const std::string& buildingName(int index) const;

    // Case-insensitive, trimmed lookup. Returns the building's index, or -1 if unknown.
    int findBuilding(const std::string& name) const;

    void displayGraph() const;               // each building and its neighbors
    void displayCountsAndDegrees() const;    // totals and degree of every building
    void bfsReachable(const std::string& start) const;                        // BFS order + distances
    void shortestPath(const std::string& start, const std::string& dest) const;  // fewest connections
    void displayConnectivityReport() const;  // Final report

private:
    std::vector<std::string> names_;           // building names, in buildings.txt order
    std::vector<std::vector<int> > adj_;       // adj_[i] = neighbor indexes, in insertion order
    int edgeCount_;                            // undirected connections, each counted once

    bool addBuilding(const std::string& name, std::string& error);
    bool addConnection(const std::string& a, const std::string& b, std::string& error);
};

// Menu helpers: they prompt the user, so main.cpp only has to call them.
void bfsReachableInteractive(const CampusGraph& graph);
void shortestPathInteractive(const CampusGraph& graph);

#endif
