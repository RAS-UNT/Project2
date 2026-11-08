// Purpose: Defines facility records, ID rules, and shared record display.
// Author: Rami Badr
#ifndef FACILITY_H
#define FACILITY_H

#include <iosfwd>
#include <string>

struct Facility {
    std::string id;
    std::string name;
    std::string building;
    std::string category;
};

// Trims and uppercases an ID before validation, hashing, storage, or lookup.
std::string normalizeFacilityId(const std::string& id);

// Accepts exactly F followed by three ASCII digits after normalization.
bool isValidFacilityId(const std::string& id);

// Displays all four fields of an accepted, normalized facility record.
void displayFacility(const Facility& facility, std::ostream& output);

#endif
