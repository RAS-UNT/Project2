// Purpose: Defines facility file loading, display, and hash-based ID searches.
// Author: Rami Badr
#ifndef FACILITY_DATABASE_H
#define FACILITY_DATABASE_H

#include "HashTable.h"
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

struct FacilityLoadSummary {
    bool opened = false;
    std::size_t accepted = 0;
    std::size_t rejected = 0;
};

class FacilityDatabase {
public:
    // Uses the custom table with a positive bucket count.
    explicit FacilityDatabase(std::size_t capacity = HashTable::DEFAULT_CAPACITY);

    // Replaces prior facilities; building names must come from accepted graph vertices.
    FacilityLoadSummary loadFromFile(const std::string& filename,
                                    const std::vector<std::string>& acceptedBuildings,
                                    std::ostream& output);

    // Displays accepted records in file order and the computed total.
    void displayAll(std::ostream& output) const;

    // Distinguishes invalid format, normalized ID not found, and a complete match.
    void displaySearchById(const std::string& id, std::ostream& output) const;

    // Delegates ID lookup to the table; returned pointers expire on the next load.
    const Facility* findById(const std::string& id) const;

    // Returns the number of accepted facilities currently stored.
    std::size_t size() const;

    // Returns the custom table's bucket count.
    std::size_t capacity() const;

    // Returns accepted-insertion collisions from the most recent load.
    std::size_t collisions() const;

    // Provides read-only records in accepted file order.
    const std::vector<Facility>& records() const;

private:
    HashTable table;

    // Matches category spelling only against previously accepted facility records.
    std::string canonicalCategory(const std::string& category) const;

    // Includes file counts and hash diagnostics after every load attempt.
    void displayLoadSummary(const std::string& filename, const FacilityLoadSummary& summary,
                            std::ostream& output) const;
};

#endif
