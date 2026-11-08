PROJECT 2 - SMART CAMPUS NAVIGATION AND FACILITY ANALYTICS

Repository: https://github.com/RAS-UNT/Project2

CURRENT STATUS
Step 3: custom hashing plus facility file loading, display, and ID search.
This is NOT a completed Milestone 1 submission. The facilities component is
implemented and tested, but campus graph, BFS, department tree, four-file
startup loading, and the required menu are not integrated on this branch yet.
The campus executable still prints its selected data directory; component
tests exercise the facility loader and searches until Step 4 menu integration.

IMPLEMENTED
- Facility record containing ID, name, building, and category.
- Facility ID normalization and F-plus-three-digits validation.
- Shared whitespace trimming, ASCII case-insensitive comparison keys,
  pipe-delimited parsing that preserves empty fields, and comment detection.
- Optional data-directory argument, with data/ as the default.
- Foundation tests and CMake build configuration.
- Custom polynomial ID hashing and separate chaining with 17 default buckets.
- Case-insensitive ID insertion/lookup and rejection of invalid or duplicate IDs.
- Accepted record order, capacity/entry/collision diagnostics, and collision tests.
- FacilityDatabase loads complete records against accepted building names.
- Filename/line/reason warnings, load summaries, and empty/missing-file recovery.
- Canonical building/category spelling and ordered facility display with totals.
- ID search distinguishes invalid format, valid not found, and complete matches.

BUILD AND RUN WITH CMAKE
Run from the project root using a C++17 compiler:
  cmake -S . -B build
  cmake --build build
  ./build/campus
  ./build/campus testdata/hash-collision

Run tests:
  cd build
  ctest --output-on-failure
  cd ..

CTest now runs eight suites: the five foundation/startup suites, table unit
tests, hash-collision fixture, and facility database tests. Direct invocations:
  ./build/hash_table_tests
  ./build/hash_table_tests testdata/hash-collision
  ./build/facility_database_tests . build

BUILD WITHOUT CMAKE ON LINUX
  mkdir -p build
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp -o build/campus
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/FoundationTests.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp -o build/foundation_tests
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/HashTableTests.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp -o build/hash_table_tests
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/FacilityDatabaseTests.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp -o build/facility_database_tests
  ./build/foundation_tests
  ./build/hash_table_tests
  ./build/hash_table_tests testdata/hash-collision
  ./build/facility_database_tests . build
  ./build/campus
  ./build/campus testdata/hash-collision

HASH TABLE
HashTable normalizes each facility ID before validation, hashing, and comparison.
Its custom formula starts at h = 0 and processes each character with:
  h = (31 * h + unsigned character value) % capacity
Each bucket holds a growing vector of indices into the accepted record list.
Insertion and lookup scan only the selected bucket. No library hash table,
library hash function, or full-record scan is used for ID lookup.

A collision means one accepted insertion into an already nonempty bucket,
regardless of the chain length. Invalid and duplicate IDs do not change entries,
collisions, or the first accepted record. The bucket count is not a record limit;
separate chaining allows records to exceed capacity without resizing.

The read-only records() sequence keeps accepted insertion order for facility
display. A pointer returned by find() may be invalidated by a
later accepted insertion; use it immediately or perform the lookup again.

COLLISION DEMONSTRATION
The team's own testdata/hash-collision directory contains all four input files.
With capacity 17, IDs F001, F014, and F027 share bucket 11. The fixture also
contains a lowercase duplicate and an invalid ID. The hash-test driver prints:
  facilities.txt: accepted=3 rejected=2 capacity=17 entries=3 collisions=2
FacilityDatabase now loads the same fixture with full facility validation and
prints its filename, warnings, acceptance counts, and these hash diagnostics.
Full application startup/menu wiring remains Step 4 work.

FACILITY DATABASE AND INTEGRATION CONTRACT
The graph module must supply a vector<string> containing ONLY accepted building
names, in canonical spelling and insertion order. No graph class is duplicated
inside the facility module. After buildings/connections are loaded, main can use:
  FacilityDatabase facilities;
  facilities.loadFromFile(dataDirectory + "/facilities.txt", acceptedBuildings, std::cout);
  facilities.displayAll(std::cout);
  facilities.displaySearchById(enteredId, std::cout);

Include FacilityDatabase.h. loadFromFile returns FacilityLoadSummary with opened,
accepted, and rejected fields. It replaces prior facilities on each load attempt;
a missing file leaves the database empty, prints a named error, and still prints
a zero-count summary. Callers should continue loading the other files.

Every accepted record has four nonempty fields, a normalized valid ID, and a
known building matched case-insensitively. The stored building uses the first
accepted spelling from the supplied building list. Categories reuse the spelling
from the first accepted facility in that category. Rejected records never
register IDs or category spellings. Duplicate names with different IDs are kept.

displayAll prints all four fields in accepted file order and a total, including
an explicit empty message and zero total. displaySearchById normalizes input,
explains invalid ID format, includes the normalized ID when missing, and prints
all fields when found. ID lookup delegates only to the custom hash table.
findById provides read-only programmatic lookup; pointers expire on the next load.

testdata/facility-validation covers rejected records, category spelling, comments,
whitespace, and ordering. testdata/facility-empty contains empty inputs. Tests
derive a CRLF copy under build/ without modifying the source fixtures or data/.

DIRECTORY LAYOUT
  include/           Facility, HashTable, FacilityDatabase, and text utility headers
  src/               Implementations and application entry point
  data/              Four unchanged instructor-provided starter files
  testdata/          hash-collision/, facility-validation/, and facility-empty/
  tests/             Automated C++ tests
  CMakeLists.txt     Build and test targets
  CONTRIBUTIONS.txt  Verified completed work and ownership status
  design_document.txt
  test_results.txt
  .gitignore

DATA
The four starter files come from the instructor's data (1).zip. Keep them
unchanged. Future loaders must read buildings, connections, facilities, and
departments in that order and compute results from accepted records.

VERIFICATION
See test_results.txt for the exact checks executed so far. Local WSL/GCC tests
are not actual UNT CSE CELL verification. CELL testing remains required.
