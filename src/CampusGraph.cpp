// CampusGraph.cpp
// Purpose: Implementation of the campus graph, its loaders, BFS, shortest path, and reports.
// Author: Sulav Bhattarai (Member B)
#include "CampusGraph.h"

#include <fstream>
#include <iostream>
#include <queue>

namespace {

// Removes spaces, tabs, \r, \n, \f, \v from both ends. Inner spaces are kept.
std::string trimWhitespace(const std::string& s) {
    const std::string ws = " \t\r\n\f\v";
    std::string::size_type start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::string::size_type end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Converts ASCII letters to lowercase.
std::string toLowerAscii(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = static_cast<char>(s[i] - 'A' + 'a');
    }
    return s;
}

// Splits a line on '|' (keeps empty pieces so field counts are correct).
std::vector<std::string> splitOnPipe(const std::string& line) {
    std::vector<std::string> pieces;
    std::string current;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '|') {
            pieces.push_back(current);
            current.clear();
        } else {
            current += line[i];
        }
    }
    pieces.push_back(current);
    return pieces;
}

// Reads lines until one is a real record (not blank, not a comment). Returns false at end of file.
// 'record' gets the trimmed line; 'lineNumber' counts every physical line read.
bool nextRecord(std::ifstream& in, std::string& record, int& lineNumber) {
    std::string line;
    while (std::getline(in, line)) {
        ++lineNumber;
        record = trimWhitespace(line);
        if (record.empty() || record[0] == '#') continue;
        return true;
    }
    return false;
}

// Prints one rejection warning.
void warn(const std::string& file, int lineNumber, const std::string& reason) {
    std::cout << "WARNING: " << file << " line " << lineNumber << ": rejected - " << reason << "\n";
}

}  // namespace

// Creates an empty graph (no buildings, no connections).
CampusGraph::CampusGraph() : edgeCount_(0) {}

// Returns the number of buildings (vertices).
int CampusGraph::buildingCount() const { return static_cast<int>(names_.size()); }

// Returns the number of undirected connections, each counted once.
int CampusGraph::connectionCount() const { return edgeCount_; }

// Returns the building's name exactly as spelled in buildings.txt (index must be valid).
const std::string& CampusGraph::buildingName(int index) const { return names_[index]; }

// Finds a building by name (trimmed, case-insensitive). Returns its index or -1.
int CampusGraph::findBuilding(const std::string& name) const {
    std::string wanted = toLowerAscii(trimWhitespace(name));
    for (std::size_t i = 0; i < names_.size(); ++i) {
        if (toLowerAscii(names_[i]) == wanted) return static_cast<int>(i);
    }
    return -1;
}

// Adds one building. Rejects empty names and duplicates (ignoring case); fills 'error' on failure.
bool CampusGraph::addBuilding(const std::string& name, std::string& error) {
    error.clear();
    std::string clean = trimWhitespace(name);
    if (clean.empty()) {
        error = "empty building name";
        return false;
    }
    if (findBuilding(clean) != -1) {
        error = "duplicate building '" + clean + "'";
        return false;
    }
    names_.push_back(clean);
    adj_.push_back(std::vector<int>());
    return true;
}

// Adds one undirected connection. Rejects unknown buildings, self-connections and duplicates
// (including reversed ones); fills 'error' on failure.
bool CampusGraph::addConnection(const std::string& a, const std::string& b, std::string& error) {
    error.clear();
    std::string nameA = trimWhitespace(a);
    std::string nameB = trimWhitespace(b);
    int ia = findBuilding(nameA);
    int ib = findBuilding(nameB);
    if (ia == -1 && ib == -1) {
        error = "unknown buildings '" + nameA + "' and '" + nameB + "'";
        return false;
    }
    if (ia == -1) {
        error = "unknown building '" + nameA + "'";
        return false;
    }
    if (ib == -1) {
        error = "unknown building '" + nameB + "'";
        return false;
    }
    if (ia == ib) {
        error = "self-connection on '" + names_[ia] + "'";
        return false;
    }
    for (std::size_t i = 0; i < adj_[ia].size(); ++i) {
        if (adj_[ia][i] == ib) {
            error = "duplicate connection between '" + names_[ia] + "' and '" + names_[ib] + "'";
            return false;
        }
    }
    adj_[ia].push_back(ib);  // update BOTH endpoints, count the connection once
    adj_[ib].push_back(ia);
    ++edgeCount_;
    return true;
}

// Reads buildings.txt, rejecting bad lines with warnings, then prints the load summary.
void CampusGraph::loadBuildings(const std::string& filePath, const std::string& displayName) {
    int accepted = 0;
    int rejected = 0;
    std::ifstream in(filePath.c_str());
    if (!in.is_open()) {
        std::cout << "ERROR: Could not open file " << displayName << " (treated as empty)\n";
    } else {
        std::string record;
        int lineNumber = 0;
        while (nextRecord(in, record, lineNumber)) {
            std::string reason;
            if (record.find('|') != std::string::npos) {
                reason = "expected exactly 1 field (a building name with no '|')";
            } else {
                std::string error;
                if (!addBuilding(record, error)) reason = error;
            }
            if (reason.empty()) {
                ++accepted;
            } else {
                ++rejected;
                warn(displayName, lineNumber, reason);
            }
        }
    }
    std::cout << displayName << ": " << accepted << " accepted, " << rejected << " rejected\n";
}

// Reads connections.txt, rejecting bad lines with warnings, then prints the load summary.
void CampusGraph::loadConnections(const std::string& filePath, const std::string& displayName) {
    int accepted = 0;
    int rejected = 0;
    std::ifstream in(filePath.c_str());
    if (!in.is_open()) {
        std::cout << "ERROR: Could not open file " << displayName << " (treated as empty)\n";
    } else {
        std::string record;
        int lineNumber = 0;
        while (nextRecord(in, record, lineNumber)) {
            std::vector<std::string> fields = splitOnPipe(record);
            std::string reason;
            if (fields.size() != 2) {
                reason = "expected exactly 2 fields (BuildingA|BuildingB)";
            } else if (trimWhitespace(fields[0]).empty() || trimWhitespace(fields[1]).empty()) {
                reason = "empty required field";
            } else {
                std::string error;
                if (!addConnection(fields[0], fields[1], error)) reason = error;
            }
            if (reason.empty()) {
                ++accepted;
            } else {
                ++rejected;
                warn(displayName, lineNumber, reason);
            }
        }
    }
    std::cout << displayName << ": " << accepted << " accepted, " << rejected << " rejected\n";
}

// Prints every building (file order) with its neighbors (insertion order).
void CampusGraph::displayGraph() const {
    if (names_.empty()) {
        std::cout << "No buildings are loaded.\n";
        return;
    }
    std::cout << "Campus graph:\n";
    for (std::size_t i = 0; i < names_.size(); ++i) {
        std::cout << names_[i] << ": ";
        if (adj_[i].empty()) {
            std::cout << "(no connections)";
        } else {
            for (std::size_t j = 0; j < adj_[i].size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << names_[adj_[i][j]];
            }
        }
        std::cout << "\n";
    }
}

// Prints total buildings, total connections, and the degree of every building.
void CampusGraph::displayCountsAndDegrees() const {
    if (names_.empty()) {
        std::cout << "No buildings are loaded.\n";
        std::cout << "Total buildings: 0\nTotal connections: 0\n";
        return;
    }
    std::cout << "Total buildings: " << names_.size() << "\n";
    std::cout << "Total connections: " << edgeCount_ << "\n";
    std::cout << "Degrees:\n";
    for (std::size_t i = 0; i < names_.size(); ++i) {
        std::cout << "  " << names_[i] << ": " << adj_[i].size() << "\n";
    }
}

// BFS from 'start': prints reachable buildings in visit order with distances, and the count.
void CampusGraph::bfsReachable(const std::string& start) const {
    if (names_.empty()) {
        std::cout << "No buildings are loaded.\n";
        return;
    }
    int s = findBuilding(start);
    if (s == -1) {
        std::cout << "Error: unknown building '" << trimWhitespace(start) << "'.\n";
        return;
    }
    std::vector<bool> visited(names_.size(), false);
    std::vector<int> distance(names_.size(), 0);
    std::queue<int> waiting;
    visited[s] = true;  // mark visited when ADDED to the queue
    waiting.push(s);
    int reachable = 0;
    std::cout << "Reachable buildings from " << names_[s] << " (BFS order):\n";
    while (!waiting.empty()) {
        int current = waiting.front();
        waiting.pop();
        ++reachable;
        std::cout << "  " << names_[current] << " (distance " << distance[current] << ")\n";
        for (std::size_t i = 0; i < adj_[current].size(); ++i) {
            int next = adj_[current][i];
            if (!visited[next]) {
                visited[next] = true;
                distance[next] = distance[current] + 1;
                waiting.push(next);
            }
        }
    }
    std::cout << "Reachable: " << reachable << " of " << names_.size() << " buildings\n";
}

// BFS from 'start' to 'dest': prints the fewest-connection path, one building per line.
void CampusGraph::shortestPath(const std::string& start, const std::string& dest) const {
    if (names_.empty()) {
        std::cout << "No buildings are loaded.\n";
        return;
    }
    int s = findBuilding(start);
    int d = findBuilding(dest);
    if (s == -1 || d == -1) {
        if (s == -1) std::cout << "Error: unknown start building '" << trimWhitespace(start) << "'.\n";
        if (d == -1) std::cout << "Error: unknown destination building '" << trimWhitespace(dest) << "'.\n";
        return;
    }
    std::vector<bool> visited(names_.size(), false);
    std::vector<int> cameFrom(names_.size(), -1);
    std::queue<int> waiting;
    visited[s] = true;
    waiting.push(s);
    while (!waiting.empty() && !visited[d]) {
        int current = waiting.front();
        waiting.pop();
        for (std::size_t i = 0; i < adj_[current].size(); ++i) {
            int next = adj_[current][i];
            if (!visited[next]) {
                visited[next] = true;
                cameFrom[next] = current;
                waiting.push(next);
            }
        }
    }
    if (!visited[d]) {
        std::cout << "No path exists between " << names_[s] << " and " << names_[d] << ".\n";
        return;
    }
    std::vector<int> path;  // built backward from the destination
    for (int at = d; at != -1; at = cameFrom[at]) path.push_back(at);
    std::cout << "Shortest path:\n";
    for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
        std::cout << names_[path[i]] << "\n";
    }
    std::cout << "Number of connections: " << path.size() - 1 << "\n";
}

// Prints totals, highest-degree building(s), isolated buildings, and component count.
void CampusGraph::displayConnectivityReport() const {
    if (names_.empty()) {
        std::cout << "No buildings are loaded.\n";
        std::cout << "Total buildings: 0\nTotal connections: 0\n";
        std::cout << "Connected components: 0\n";
        return;
    }
    std::cout << "Total buildings: " << names_.size() << "\n";
    std::cout << "Total connections: " << edgeCount_ << "\n";

    std::size_t highest = 0;
    for (std::size_t i = 0; i < adj_.size(); ++i) {
        if (adj_[i].size() > highest) highest = adj_[i].size();
    }
    std::cout << "Highest degree (" << highest << "): ";
    bool first = true;
    for (std::size_t i = 0; i < adj_.size(); ++i) {
        if (adj_[i].size() == highest) {
            if (!first) std::cout << ", ";
            std::cout << names_[i];
            first = false;
        }
    }
    std::cout << "\n";

    std::cout << "Isolated buildings: ";
    first = true;
    for (std::size_t i = 0; i < adj_.size(); ++i) {
        if (adj_[i].empty()) {
            if (!first) std::cout << ", ";
            std::cout << names_[i];
            first = false;
        }
    }
    if (first) std::cout << "None";
    std::cout << "\n";

    // Count components: start a BFS from every building not yet visited.
    std::vector<bool> visited(names_.size(), false);
    int components = 0;
    for (std::size_t i = 0; i < names_.size(); ++i) {
        if (visited[i]) continue;
        ++components;
        std::queue<int> waiting;
        visited[i] = true;
        waiting.push(static_cast<int>(i));
        while (!waiting.empty()) {
            int current = waiting.front();
            waiting.pop();
            for (std::size_t j = 0; j < adj_[current].size(); ++j) {
                int next = adj_[current][j];
                if (!visited[next]) {
                    visited[next] = true;
                    waiting.push(next);
                }
            }
        }
    }
    std::cout << "Connected components: " << components << "\n";
}

// Menu helper: asks for a start building (blank rejected) and runs BFS reachability.
void bfsReachableInteractive(const CampusGraph& graph) {
    std::string start;
    std::cout << "Enter starting building: ";
    std::getline(std::cin, start);
    start = trimWhitespace(start);
    if (start.empty()) {
        std::cout << "Error: building name cannot be blank.\n";
        return;
    }
    graph.bfsReachable(start);
}

// Menu helper: asks for start and destination (blank rejected) and prints the shortest path.
void shortestPathInteractive(const CampusGraph& graph) {
    std::string start;
    std::string dest;
    std::cout << "Enter starting building: ";
    std::getline(std::cin, start);
    start = trimWhitespace(start);
    if (start.empty()) {
        std::cout << "Error: building name cannot be blank.\n";
        return;
    }
    std::cout << "Enter destination building: ";
    std::getline(std::cin, dest);
    dest = trimWhitespace(dest);
    if (dest.empty()) {
        std::cout << "Error: building name cannot be blank.\n";
        return;
    }
    graph.shortestPath(start, dest);
}
