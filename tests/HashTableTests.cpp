// Purpose: Tests custom hashing, collision chains, and a separate collision data set.
// Author: Rami Badr
#include "HashTable.h"
#include "TextUtils.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Keeps checks active in both Debug and Release builds.
void expect(bool condition, const std::string& description, int& checks, int& failures) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

// Tests the ID table independently from the future campus file loaders.
void testTable(int& checks, int& failures) {
    HashTable table;
    expect(table.capacity() == 17, "default capacity", checks, failures);
    expect(table.size() == 0 && table.collisions() == 0 && table.records().empty(),
           "empty table state", checks, failures);
    expect(table.find("F001") == nullptr, "valid missing key in empty table", checks, failures);
    expect(table.find("invalid") == nullptr, "invalid lookup in empty table", checks, failures);

    bool zeroRejected = false;
    try {
        HashTable invalid(0);
    } catch (const std::invalid_argument&) {
        zeroRejected = true;
    }
    expect(zeroRejected, "zero capacity rejected", checks, failures);

    Facility first{" \tf001\r", "First Room", "Test Building", "Classroom"};
    expect(table.insert(first) == HashTable::InsertResult::Inserted, "insert normalized key", checks, failures);
    expect(first.id == " \tf001\r", "insertion does not modify caller record", checks, failures);
    expect(table.size() == 1 && table.collisions() == 0, "first insertion counters", checks, failures);
    const Facility* found = table.find(" f001\t");
    expect(found && found->id == "F001" && found->name == "First Room" &&
           found->building == "Test Building" && found->category == "Classroom",
           "case-insensitive lookup returns all original fields", checks, failures);

    Facility duplicate{"F001", "Replacement", "Other Building", "Other Category"};
    expect(table.insert(duplicate) == HashTable::InsertResult::DuplicateId,
           "reject uppercase duplicate", checks, failures);
    found = table.find("F001");
    expect(found && found->name == "First Room" && found->building == "Test Building" &&
           found->category == "Classroom", "duplicate retains first complete record", checks, failures);
    expect(table.size() == 1 && table.collisions() == 0, "duplicate before collisions keeps counters", checks, failures);

    expect(table.insert({"f014", "Second Room", "Test Building", "Classroom"}) == HashTable::InsertResult::Inserted,
           "insert first colliding ID", checks, failures);
    expect(table.collisions() == 1, "nonempty bucket adds one collision", checks, failures);
    expect(table.insert({"F027", "Third Room", "Test Building", "Classroom"}) == HashTable::InsertResult::Inserted,
           "insert second colliding ID", checks, failures);
    expect(table.collisions() == 2, "third chain entry adds only one collision", checks, failures);

    for (const std::string& id : std::vector<std::string>{"F001", "f014", " F027 "}) {
        expect(table.find(id) != nullptr, "find every collision-chain position: " + id, checks, failures);
    }
    expect(table.insert({" f014 ", "Replacement", "Other", "Other"}) == HashTable::InsertResult::DuplicateId,
           "reject duplicate from collision chain", checks, failures);
    found = table.find("F014");
    expect(found && found->name == "Second Room", "colliding duplicate leaves record unchanged", checks, failures);
    expect(table.size() == 3 && table.collisions() == 2, "colliding duplicate leaves counters unchanged", checks, failures);

    for (const std::string& id : std::vector<std::string>{"", " ", "A001", "F01", "F0001", "F00A", "F 01", "F001suffix"}) {
        expect(table.insert({id, "Invalid", "Test Building", "Classroom"}) == HashTable::InsertResult::InvalidId,
               "reject invalid insert: " + id, checks, failures);
        expect(table.find(id) == nullptr, "reject invalid lookup: " + id, checks, failures);
        expect(table.size() == 3 && table.collisions() == 2, "invalid ID preserves counters", checks, failures);
    }

    expect(table.find("F999") == nullptr, "missing valid ID in nonempty table", checks, failures);
    expect(table.insert({"F002", "First Room", "Another Building", "Classroom"}) == HashTable::InsertResult::Inserted,
           "same facility name with different ID is allowed", checks, failures);
    expect(table.collisions() == 2, "empty bucket insertion is not a collision", checks, failures);
    const std::vector<std::string> order{"F001", "F014", "F027", "F002"};
    expect(table.records().size() == order.size(), "all accepted records retained", checks, failures);
    for (std::size_t index = 0; index < order.size(); ++index) {
        expect(table.records()[index].id == order[index], "insertion order independent of bucket order", checks, failures);
    }

    std::ostringstream diagnostic;
    table.displayDiagnostics(diagnostic);
    expect(diagnostic.str() == "capacity=17 entries=4 collisions=2", "computed diagnostic fields", checks, failures);
    expect(table.size() == 4 && table.collisions() == 2, "queries leave counters unchanged", checks, failures);

    HashTable copied = table;
    expect(copied.find("f014") && copied.find("f014")->name == "Second Room", "copied bucket indices remain valid", checks, failures);
    expect(copied.insert({"F003", "Copy Only", "Test Building", "Lab"}) == HashTable::InsertResult::Inserted,
           "copy accepts independent insertion", checks, failures);
    expect(table.find("F003") == nullptr && table.size() == 4, "copy does not change original", checks, failures);
    HashTable assigned(1);
    assigned = table;
    expect(assigned.capacity() == 17 && assigned.size() == 4 && assigned.collisions() == 2 &&
           assigned.find("F027"), "assignment copies complete table state", checks, failures);

    HashTable oneBucket(1);
    for (int number = 0; number < 1000; ++number) {
        std::ostringstream id;
        id << 'F' << std::setw(3) << std::setfill('0') << number;
        expect(oneBucket.insert({id.str(), "Stress Room", "Test Building", "Lab"}) == HashTable::InsertResult::Inserted,
               "one-bucket insert: " + id.str(), checks, failures);
    }
    expect(oneBucket.capacity() == 1 && oneBucket.size() == 1000 && oneBucket.collisions() == 999,
           "separate chaining exceeds bucket count without arbitrary record limit", checks, failures);
    for (int number = 0; number < 1000; ++number) {
        std::ostringstream id;
        id << 'F' << std::setw(3) << std::setfill('0') << number;
        const Facility* record = oneBucket.find(id.str());
        expect(record && record->id == id.str(), "one-bucket lookup: " + id.str(), checks, failures);
    }
    expect(oneBucket.insert({"f999", "Replacement", "Other", "Other"}) == HashTable::InsertResult::DuplicateId,
           "duplicate at end of long chain rejected", checks, failures);
    expect(oneBucket.size() == 1000 && oneBucket.collisions() == 999,
           "long-chain duplicate leaves counts unchanged", checks, failures);
}

// Loads only the hash-test fixture; building/reference validation belongs to Step 3.
void testCollisionDirectory(const std::string& directory, int& checks, int& failures) {
    const std::string filename = directory + "/facilities.txt";
    std::ifstream input(filename);
    expect(input.is_open(), "open collision fixture", checks, failures);
    if (!input) return;

    HashTable table;
    std::size_t accepted = 0;
    std::size_t rejected = 0;
    std::string line;
    while (std::getline(input, line)) {
        if (TextUtils::isBlankOrComment(line)) continue;
        const std::vector<std::string> fields = TextUtils::splitFields(line);
        expect(fields.size() == 4, "fixture has four fields", checks, failures);
        if (fields.size() != 4) {
            ++rejected;
            continue;
        }

        const std::size_t previousSize = table.size();
        const std::size_t previousCollisions = table.collisions();
        HashTable::InsertResult result = table.insert({fields[0], fields[1], fields[2], fields[3]});
        if (result == HashTable::InsertResult::Inserted) {
            ++accepted;
            expect(table.collisions() == accepted - 1, "accepted fixture IDs share a bucket", checks, failures);
        } else {
            ++rejected;
            expect(table.size() == previousSize && table.collisions() == previousCollisions,
                   "rejected fixture record preserves diagnostics", checks, failures);
        }
    }

    expect(accepted == 3 && rejected == 2, "fixture acceptance and rejection totals", checks, failures);
    expect(table.capacity() == 17 && table.size() == 3 && table.collisions() == 2,
           "fixture collision diagnostic", checks, failures);
    const Facility* retained = table.find("f014");
    expect(retained && retained->id == "F014" && retained->name == "Second Collision Facility" &&
           retained->category == "Test Category", "fixture duplicate never replaces first record", checks, failures);
    expect(table.find("F001") && table.find("F027"), "collided fixture records remain searchable", checks, failures);

    std::cout << "facilities.txt: accepted=" << accepted << " rejected=" << rejected << ' ';
    table.displayDiagnostics(std::cout);
    std::cout << '\n';
}

// Runs table-only assertions or a named collision-test data directory.
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: hash_table_tests [collision-data-directory]\n";
        return 1;
    }
    int checks = 0;
    int failures = 0;
    if (argc == 2) testCollisionDirectory(argv[1], checks, failures);
    else testTable(checks, failures);

    if (failures != 0) return 1;
    std::cout << "All " << checks << " hash-table checks passed.\n";
    return 0;
}
