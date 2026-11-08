// Purpose: Loads validated facilities and exposes ordered display and custom-hash search.
// Author: Rami Badr
#include "FacilityDatabase.h"
#include "TextUtils.h"
#include <fstream>
#include <ostream>

namespace {
// Returns the first accepted building spelling without changing internal spaces.
std::string canonicalBuilding(const std::string& reference,
                              const std::vector<std::string>& acceptedBuildings) {
    const std::string key = TextUtils::normalizeKey(reference);
    for (const std::string& building : acceptedBuildings) {
        if (TextUtils::normalizeKey(building) == key) return TextUtils::trim(building);
    }
    return "";
}
}

// Lets the hash table enforce its positive-capacity requirement.
FacilityDatabase::FacilityDatabase(std::size_t capacity) : table(capacity) {}

// Loads complete lines, reports rejected records, and treats a missing file as empty.
FacilityLoadSummary FacilityDatabase::loadFromFile(
    const std::string& filename,
    const std::vector<std::string>& acceptedBuildings,
    std::ostream& output
) {
    table = HashTable(table.capacity());
    FacilityLoadSummary summary;
    std::ifstream input(filename);
    if (!input) {
        output << "Error: cannot open facility file '" << filename << "'.\n";
        displayLoadSummary(filename, summary, output);
        return summary;
    }
    summary.opened = true;

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (TextUtils::isBlankOrComment(line)) continue;
        auto reject = [&](const std::string& reason) {
            ++summary.rejected;
            output << "Warning: " << filename << ':' << lineNumber << ": " << reason << '\n';
        };

        const std::vector<std::string> fields = TextUtils::splitFields(line);
        if (fields.size() != 4) {
            reject("expected exactly four pipe-delimited fields");
            continue;
        }
        if (fields[0].empty() || fields[1].empty() || fields[2].empty() || fields[3].empty()) {
            reject("empty required field");
            continue;
        }

        const std::string id = normalizeFacilityId(fields[0]);
        if (!isValidFacilityId(id)) {
            reject("invalid facility ID; use F followed by exactly three digits");
            continue;
        }
        const std::string building = canonicalBuilding(fields[2], acceptedBuildings);
        if (building.empty()) {
            reject("unknown building '" + fields[2] + "'");
            continue;
        }

        Facility facility{id, fields[1], building, canonicalCategory(fields[3])};
        HashTable::InsertResult result = table.insert(facility);
        if (result == HashTable::InsertResult::Inserted) {
            ++summary.accepted;
        } else if (result == HashTable::InsertResult::DuplicateId) {
            reject("duplicate facility ID '" + id + "'");
        } else {
            reject("invalid facility ID; use F followed by exactly three digits");
        }
    }

    if (input.bad()) output << "Error: could not finish reading facility file '" << filename << "'.\n";
    displayLoadSummary(filename, summary, output);
    return summary;
}

// A rejected record cannot register a category because only accepted records are scanned.
std::string FacilityDatabase::canonicalCategory(const std::string& category) const {
    const std::string key = TextUtils::normalizeKey(category);
    for (const Facility& facility : table.records()) {
        if (TextUtils::normalizeKey(facility.category) == key) return facility.category;
    }
    return category;
}

// Uses the table's current values rather than expected sample totals.
void FacilityDatabase::displayLoadSummary(const std::string& filename,
                                         const FacilityLoadSummary& summary,
                                         std::ostream& output) const {
    output << filename << ": accepted=" << summary.accepted << " rejected=" << summary.rejected << ' ';
    table.displayDiagnostics(output);
    output << '\n';
}

// Traverses accepted file order, independently of collision-bucket order.
void FacilityDatabase::displayAll(std::ostream& output) const {
    if (table.size() == 0) output << "No facilities are loaded.\n";
    for (const Facility& facility : table.records()) {
        displayFacility(facility, output);
        output << '\n';
    }
    output << "Total facilities: " << table.size() << '\n';
}

// Validates search input before using the custom ID hash table.
void FacilityDatabase::displaySearchById(const std::string& id, std::ostream& output) const {
    const std::string key = normalizeFacilityId(id);
    if (!isValidFacilityId(key)) {
        output << "Invalid facility ID. Use F followed by exactly three digits.\n";
        return;
    }
    const Facility* facility = findById(key);
    if (facility == nullptr) output << "Facility " << key << " was not found.\n";
    else displayFacility(*facility, output);
}

// ID searches never scan the full record sequence.
const Facility* FacilityDatabase::findById(const std::string& id) const {
    return table.find(id);
}

// Counts only accepted, unique facility IDs.
std::size_t FacilityDatabase::size() const {
    return table.size();
}

// Keeps diagnostics available without exposing mutable table storage.
std::size_t FacilityDatabase::capacity() const {
    return table.capacity();
}

// Rejected duplicates and invalid references do not add collisions.
std::size_t FacilityDatabase::collisions() const {
    return table.collisions();
}

// Reuses the table's insertion-ordered record storage.
const std::vector<Facility>& FacilityDatabase::records() const {
    return table.records();
}
