PROJECT 2 - SMART CAMPUS NAVIGATION AND FACILITY ANALYTICS

Repository: https://github.com/RAS-UNT/Project2

CURRENT STATUS
Step 1: project setup and facility/text validation foundations only.
This is NOT a completed Milestone 1 submission. The custom hash table, file
loaders, campus graph, BFS, department tree, and required menu are not yet
implemented. The current campus executable prints its selected data directory.
It does not claim to load or validate campus records yet.

IMPLEMENTED
- Facility record containing ID, name, building, and category.
- Facility ID normalization and F-plus-three-digits validation.
- Shared whitespace trimming, ASCII case-insensitive comparison keys,
  pipe-delimited parsing that preserves empty fields, and comment detection.
- Optional data-directory argument, with data/ as the default.
- Foundation tests and CMake build configuration.

BUILD AND RUN WITH CMAKE
Run from the project root using a C++17 compiler:
  cmake -S . -B build
  cmake --build build
  ./build/campus
  ./build/campus testdata/example

Run tests:
  cd build
  ctest --output-on-failure
  cd ..

BUILD WITHOUT CMAKE ON LINUX
  mkdir -p build
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/Facility.cpp src/TextUtils.cpp -o build/campus
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude tests/FoundationTests.cpp src/Facility.cpp src/TextUtils.cpp -o build/foundation_tests
  ./build/foundation_tests
  ./build/campus
  ./build/campus testdata/example

DIRECTORY LAYOUT
  include/           Facility and shared text utility headers
  src/               Implementations and application entry point
  data/              Four unchanged instructor-provided starter files
  testdata/          Reserved for the team's separate file-based test cases
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
