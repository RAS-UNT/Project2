// Purpose: Tests facility loading, canonical spellings, ordered display, and ID searches.
// Author: Rami Badr
#include "FacilityDatabase.h"
#include "TextUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Keeps test conditions active in Release builds and reports individual failures.
void expect(bool condition, const std::string& description, int& checks, int& failures) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

// Reads only known-valid fixture buildings; the production graph loader is separate.
std::vector<std::string> fixtureBuildings(const std::string& filename, int& checks, int& failures) {
    std::ifstream input(filename);
    expect(input.is_open(), "open fixture buildings: " + filename, checks, failures);
    std::vector<std::string> names;
    std::string line;
    while (std::getline(input, line)) {
        if (TextUtils::isBlankOrComment(line)) continue;
        const std::vector<std::string> fields = TextUtils::splitFields(line);
        expect(fields.size() == 1 && !fields[0].empty(), "fixture building is valid", checks, failures);
        if (fields.size() == 1 && !fields[0].empty()) names.push_back(fields[0]);
    }
    return names;
}

// Counts diagnostic markers without imposing assumptions about column alignment.
std::size_t occurrences(const std::string& text, const std::string& needle) {
    if (needle.empty()) return 0;
    std::size_t count = 0;
    std::size_t position = 0;
    while ((position = text.find(needle, position)) != std::string::npos) {
        ++count;
        position += needle.size();
    }
    return count;
}

// Creates a derived Windows-line-ending input in the ignored test work directory.
bool makeWindowsCopy(const std::string& source, const std::string& destination) {
    std::ifstream input(source, std::ios::binary);
    std::ofstream output(destination, std::ios::binary);
    if (!input || !output) return false;
    char character;
    while (input.get(character)) {
        if (character == '\r') continue;
        if (character == '\n') output << "\r\n";
        else output.put(character);
    }
    output.flush();
    return !input.bad() && output.good();
}

// Exercises component behavior without implementing another member's graph or menu.
void runTests(const std::string& root, const std::string& work, int& checks, int& failures) {
    FacilityDatabase database;
    expect(database.size() == 0 && database.capacity() == 17 && database.collisions() == 0,
           "empty database counters", checks, failures);
    std::ostringstream emptyDisplay;
    database.displayAll(emptyDisplay);
    expect(emptyDisplay.str().find("No facilities are loaded.") != std::string::npos &&
           emptyDisplay.str().find("Total facilities: 0") != std::string::npos,
           "empty display includes message and zero total", checks, failures);
    for (const std::string& id : std::vector<std::string>{"", "  ", "F01", "F0001", "A001", "F00A", "F 01", "F001suffix"}) {
        std::ostringstream output;
        database.displaySearchById(id, output);
        expect(output.str().find("Invalid facility ID.") != std::string::npos &&
               output.str().find("F followed by exactly three digits") != std::string::npos,
               "invalid search format: " + id, checks, failures);
    }
    std::ostringstream missingSearch;
    database.displaySearchById(" f999\r", missingSearch);
    expect(missingSearch.str() == "Facility F999 was not found.\n",
           "valid missing search shows normalized ID", checks, failures);

    const std::string starterFile = root + "/data/facilities.txt";
    const std::vector<std::string> starterBuildings = fixtureBuildings(root + "/data/buildings.txt", checks, failures);
    std::ostringstream starterOutput;
    FacilityLoadSummary summary = database.loadFromFile(starterFile, starterBuildings, starterOutput);
    expect(summary.opened && summary.accepted == 10 && summary.rejected == 0,
           "instructor facilities load correctly", checks, failures);
    expect(database.size() == 10 && database.collisions() == 0, "instructor data hash totals", checks, failures);
    expect(starterOutput.str() == starterFile + ": accepted=10 rejected=0 capacity=17 entries=10 collisions=0\n",
           "load summary contains filename and computed diagnostics", checks, failures);
    const Facility* firstRoom = database.findById("F005");
    const Facility* secondRoom = database.findById("F007");
    expect(firstRoom && secondRoom && firstRoom->name == "Room 101" && secondRoom->name == "Room 101" &&
           firstRoom->building != secondRoom->building, "same-name instructor facilities both retained", checks, failures);
    std::ostringstream foundSearch;
    database.displaySearchById(" \tf004\r", foundSearch);
    expect(foundSearch.str() == "ID: F004\nName: Computer Lab A\nBuilding: Science Building\nCategory: Computer Lab\n",
           "successful search displays complete canonical record", checks, failures);
    std::ostringstream allFacilities;
    database.displayAll(allFacilities);
    expect(occurrences(allFacilities.str(), "ID: ") == 10 &&
           allFacilities.str().find("Total facilities: 10") != std::string::npos,
           "display prints every accepted facility and total", checks, failures);
    expect(database.size() == 10 && database.collisions() == 0,
           "search and display do not change counts", checks, failures);

    const std::string validationFile = root + "/testdata/facility-validation/facilities.txt";
    const std::vector<std::string> buildings = fixtureBuildings(
        root + "/testdata/facility-validation/buildings.txt", checks, failures);
    std::ostringstream validationOutput;
    summary = database.loadFromFile(validationFile, buildings, validationOutput);
    expect(summary.opened && summary.accepted == 7 && summary.rejected == 12,
           "mixed validation fixture counts", checks, failures);
    expect(database.size() == 7 && database.collisions() == 3 && database.findById("F010") == nullptr,
           "new load replaces prior records and resets diagnostics", checks, failures);
    expect(occurrences(validationOutput.str(), "Warning: ") == 12,
           "one warning for each rejected record", checks, failures);
    for (int line : std::vector<int>{6, 7, 8, 9, 10, 11, 12, 15, 17, 18, 19, 20}) {
        expect(validationOutput.str().find("Warning: " + validationFile + ':' + std::to_string(line) + ": ") != std::string::npos,
               "warning identifies physical source line " + std::to_string(line), checks, failures);
    }
    for (const std::string& reason : std::vector<std::string>{"duplicate facility ID 'F014'", "unknown building 'Missing Building'",
             "empty required field", "expected exactly four pipe-delimited fields", "invalid facility ID"}) {
        expect(validationOutput.str().find(reason) != std::string::npos,
               "warning includes rejection reason: " + reason, checks, failures);
    }
    expect(validationOutput.str().find(validationFile + ": accepted=7 rejected=12 capacity=17 entries=7 collisions=3\n") != std::string::npos,
           "mixed load summary and collision counts", checks, failures);

    const Facility* record = database.findById("f001");
    expect(record && record->name == "Room #5" && record->building == "Test Building" && record->category == "Computer Lab",
           "trim fields, normalize ID, preserve embedded hash and canonical building", checks, failures);
    record = database.findById("F014");
    expect(record && record->name == "Same Name" && record->building == "West Wing" && record->category == "Computer Lab",
           "duplicate rejected and first category spelling reused", checks, failures);
    record = database.findById("F027");
    expect(record && record->name == "Same Name", "duplicate names do not overwrite records", checks, failures);
    record = database.findById("F002");
    expect(record && record->name == "Valid After Rejection" && record->category == "poisoned category",
           "unknown-building record registers neither ID nor category", checks, failures);
    record = database.findById("F028");
    expect(record && record->category == "rejected category", "duplicate record cannot introduce category spelling", checks, failures);
    record = database.findById("F030");
    expect(record && record->building == "North  Annex" && !database.findById("F031"),
           "internal building spaces remain significant", checks, failures);
    record = database.findById("F034");
    expect(record && record->category == "Computer Lab", "uppercase category reuses first accepted spelling", checks, failures);

    const std::vector<std::string> expectedOrder{"F001", "F014", "F027", "F002", "F028", "F030", "F034"};
    expect(database.records().size() == expectedOrder.size(), "accepted record count", checks, failures);
    std::ostringstream orderedDisplay;
    database.displayAll(orderedDisplay);
    std::size_t nextPosition = 0;
    for (std::size_t index = 0; index < expectedOrder.size(); ++index) {
        expect(index < database.records().size() && database.records()[index].id == expectedOrder[index],
               "record order: " + expectedOrder[index], checks, failures);
        std::size_t position = orderedDisplay.str().find("ID: " + expectedOrder[index], nextPosition);
        expect(position != std::string::npos, "display order: " + expectedOrder[index], checks, failures);
        if (position != std::string::npos) nextPosition = position + 1;
    }
    expect(occurrences(orderedDisplay.str(), "ID: ") == 7 &&
           orderedDisplay.str().find("Total facilities: 7") != std::string::npos,
           "ordered display includes exact total", checks, failures);

    const std::string windowsFile = work + "/facility-validation-crlf.txt";
    expect(makeWindowsCopy(validationFile, windowsFile), "create derived CRLF fixture", checks, failures);
    std::ostringstream windowsOutput;
    summary = database.loadFromFile(windowsFile, buildings, windowsOutput);
    expect(summary.opened && summary.accepted == 7 && summary.rejected == 12 && database.collisions() == 3,
           "Windows line endings give identical load counts", checks, failures);
    record = database.findById("f034");
    expect(record && record->category == "Computer Lab" && record->building == "West Wing",
           "CRLF leaves no carriage returns in stored fields", checks, failures);

    const std::string collisionFile = root + "/testdata/hash-collision/facilities.txt";
    const std::vector<std::string> collisionBuildings = fixtureBuildings(
        root + "/testdata/hash-collision/buildings.txt", checks, failures);
    std::ostringstream collisionOutput;
    summary = database.loadFromFile(collisionFile, collisionBuildings, collisionOutput);
    expect(summary.opened && summary.accepted == 3 && summary.rejected == 2 && database.collisions() == 2,
           "production facility loader reproduces collision diagnostic", checks, failures);
    expect(collisionOutput.str().find("accepted=3 rejected=2 capacity=17 entries=3 collisions=2") != std::string::npos,
           "production facilities summary contains collision evidence", checks, failures);
    record = database.findById("F014");
    expect(record && record->name == "Second Collision Facility" && record->category == "Test Category",
           "collision duplicate leaves original complete record intact", checks, failures);
    std::cout << collisionOutput.str();

    std::ostringstream noBuildingsOutput;
    summary = database.loadFromFile(collisionFile, {}, noBuildingsOutput);
    expect(summary.opened && summary.accepted == 0 && summary.rejected == 5 && database.size() == 0 && database.collisions() == 0,
           "no accepted buildings means no accepted facility references", checks, failures);
    expect(occurrences(noBuildingsOutput.str(), "unknown building") == 4,
           "unknown building warnings identify valid-format records", checks, failures);

    std::ostringstream recoveryOutput;
    summary = database.loadFromFile(collisionFile, collisionBuildings, recoveryOutput);
    expect(summary.accepted == 3 && summary.rejected == 2 && database.size() == 3 && database.collisions() == 2,
           "valid load recovers after rejection-only load", checks, failures);
    std::ostringstream repeatOutput;
    summary = database.loadFromFile(collisionFile, collisionBuildings, repeatOutput);
    expect(summary.accepted == 3 && summary.rejected == 2 && database.collisions() == 2,
           "repeated file load does not accumulate duplicates or collisions", checks, failures);

    const std::string missingFile = work + "/missing-directory/facilities.txt";
    std::ostringstream missingOutput;
    summary = database.loadFromFile(missingFile, buildings, missingOutput);
    expect(!summary.opened && summary.accepted == 0 && summary.rejected == 0 && database.size() == 0 && database.collisions() == 0,
           "missing file is treated as empty, replacing prior data", checks, failures);
    expect(missingOutput.str().find("Error: cannot open facility file '" + missingFile + "'.") != std::string::npos &&
           missingOutput.str().find("accepted=0 rejected=0 capacity=17 entries=0 collisions=0") != std::string::npos,
           "missing file prints filename, error, and zero summary", checks, failures);

    std::ostringstream emptyOutput;
    summary = database.loadFromFile(root + "/testdata/facility-empty/facilities.txt", {}, emptyOutput);
    expect(summary.opened && summary.accepted == 0 && summary.rejected == 0 && database.size() == 0,
           "empty file is successfully opened with zero records", checks, failures);
    std::ostringstream afterError;
    summary = database.loadFromFile(starterFile, starterBuildings, afterError);
    expect(summary.opened && summary.accepted == 10 && summary.rejected == 0 && database.findById("f004"),
           "valid loading and lookup recover after missing/empty inputs", checks, failures);

    FacilityDatabase oneBucket(1);
    std::ostringstream singleOutput;
    summary = oneBucket.loadFromFile(starterFile, starterBuildings, singleOutput);
    expect(summary.accepted == 10 && oneBucket.capacity() == 1 && oneBucket.collisions() == 9,
           "custom bucket capacity survives loader reset without limiting facilities", checks, failures);
}

// Accepts source and ignored work directories so tests do not modify starter files.
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: facility_database_tests <project-root> <work-directory>\n";
        return 1;
    }
    int checks = 0;
    int failures = 0;
    runTests(argv[1], argv[2], checks, failures);
    if (failures != 0) return 1;
    std::cout << "All " << checks << " facility-database checks passed.\n";
    return 0;
}
