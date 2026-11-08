PROJECT 2 - SMART CAMPUS NAVIGATION AND FACILITY ANALYTICS

Repository: https://github.com/RAS-UNT/Project2

CURRENT STATUS
Step 2: project foundations and the custom facility ID hash table.
This is NOT a completed Milestone 1 submission. File loaders, campus graph,
BFS, department tree, and the required menu are not yet implemented. The
current campus executable prints its selected data directory. Hash-table
behavior is exercised by the separate tests, not the campus menu yet.

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

CTest now runs seven suites: the five foundation/startup suites plus table
unit tests and the separate hash-collision fixture. Direct test invocations:
  ./build/hash_table_tests
  ./build/hash_table_tests testdata/hash-collision

BUILD WITHOUT CMAKE ON LINUX
  mkdir -p build
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp -o build/campus
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/FoundationTests.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp -o build/foundation_tests
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/HashTableTests.cpp src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp -o build/hash_table_tests
  ./build/foundation_tests
  ./build/hash_table_tests
  ./build/hash_table_tests testdata/hash-collision
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

The read-only records() sequence keeps accepted insertion order for the next
step's facility display. A pointer returned by find() may be invalidated by a
later accepted insertion; use it immediately or perform the lookup again.

COLLISION DEMONSTRATION
The team's own testdata/hash-collision directory contains all four input files.
With capacity 17, IDs F001, F014, and F027 share bucket 11. The fixture also
contains a lowercase duplicate and an invalid ID. The hash-test driver prints:
  facilities.txt: accepted=3 rejected=2 capacity=17 entries=3 collisions=2
This demonstrates table loading only. Full application loading and the required
facilities summary will be connected in later steps.

DIRECTORY LAYOUT
  include/           Facility, HashTable, and shared text utility headers
  src/               Implementations and application entry point
  data/              Four unchanged instructor-provided starter files
  testdata/          Team test cases, including hash-collision/
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
