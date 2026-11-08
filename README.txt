PROJECT 2 - SMART CAMPUS NAVIGATION AND FACILITY ANALYTICS

Repository: https://github.com/RAS-UNT/Project2

PURPOSE AND STATUS
C++17 console application for campus facility lookup, graph exploration,
BFS reachability, and department hierarchy management. All Milestone 1 modules
and menu operations are integrated and locally verified. The source is prepared
for submission packaging from main. Actual UNT CSE CELL verification remains
pending and must be completed before submission.

MODULE OWNERS
Rami Badr: facilities, custom ID hash table, and shared application integration.
Sulav Bhattarai: CampusGraph, building/connection loading, and BFS.
Aqani: department tree, loading, menu actions, and department component tests.
See CONTRIBUTIONS.txt for completed tasks and integration fixes.

BUILD AND RUN WITH CMAKE
From the project root, with a C++17 compiler and CMake 3.10 or newer:
  cmake -S . -B build
  cmake --build build
  ./build/campus
  ./build/campus testdata/hash-collision
  ./build/campus testdata/integration-invalid

Run tests:
  cd build
  ctest --output-on-failure
  cd ..

Nine component/startup suites are always registered. A working Python 3
interpreter adds the tenth suite, containing 60 full-application menu cases.

BUILD AND TEST WITHOUT CMAKE ON LINUX
The application itself needs only a C++17 compiler:
  mkdir -p build
  g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/*.cpp -o build/campus
  ./build/campus

For all component and menu checks, using GCC and Python 3:
  bash tests/run_linux_checks.sh

The script builds under build/linux and runs the four component executables,
the collision fixture, and all 60 application menu cases. You can also run:
  python3 tests/MenuIntegrationTests.py build/linux/campus

STARTUP AND DATA
The optional argument selects a data directory; the default is data/ relative
to the working directory. Run from the project root for the default files.
Startup loads buildings.txt, connections.txt, facilities.txt, departments.txt
in that order. Facilities refer only to accepted graph building names.
Warnings show filename, physical line, and rejection reason. Each file has an
accepted/rejected summary; facilities also show capacity, entries, collisions.
Missing files are treated as empty without stopping the remaining startup/menu.
All counts and search results come from accepted input records.

The four instructor files in data/ remain unchanged. Own static fixtures live
in testdata/. Generated CRLF and menu-test inputs stay under ignored build/.
Department menu additions affect memory only and disappear when the app exits.

MILESTONE 1 MENU
  1. View all facilities
  2. Search facility by ID
  3. View campus graph
  4. Display graph counts and degrees
  5. Find reachable buildings (BFS)
  6. Add department
  7. Search department
  8. Display department hierarchy
  9. Run preorder traversal
  0. Exit

Names accept spaces and ignore capitalization and surrounding whitespace;
internal spaces remain significant. ID inputs normalize to uppercase F plus
exactly three digits. Search distinguishes invalid input, valid not found,
and a complete matching record. Duplicate facility names with different IDs
remain separate records. Facility display preserves accepted file order.

The graph preserves neighbor insertion order; BFS includes the starting
building at distance zero and prints its reachable count out of all buildings.
A department added to an empty tree becomes the root without a parent prompt.
Otherwise enter an existing parent. Hierarchy output indents four spaces per
depth; option 9 prints the same preorder sequence without indentation.

Invalid menu input reports an error and returns to the menu. Enter one whole
number from 0 to 9. Closing input also exits normally.

HASH FUNCTION AND COLLISIONS
Normalized IDs use a custom character hash:
  h = (31 * h + unsigned character value) % capacity, starting at zero
Capacity defaults to the named constant 17 and counts buckets, not a record
limit. Separate chains grow dynamically. One accepted insertion into a
nonempty bucket counts as one collision; duplicate/invalid IDs add none.
ID lookup scans only its bucket. The read-only accepted record list is used
for display and category spelling, not as a replacement for hashed ID lookup.

testdata/hash-collision uses F001, f014, and F027 in the same bucket. Run:
  ./build/campus testdata/hash-collision
The real startup summary includes:
  accepted=3 rejected=2 capacity=17 entries=3 collisions=2

GRAPH AND TREE REPRESENTATIONS
CampusGraph uses an adjacency list, updating both endpoints and counting each
edge once. BFS uses a queue and marks visited at enqueue. DepartmentTree is a
general tree with a vector of owned children per node, not a binary tree.
See design_document.txt for rationale, validation, and complexity details.
The supplied graph's Final shortest-path/report methods remain in the source
but are not exposed as extra Milestone 1 menu choices.

DIRECTORY LAYOUT
  include/           Module headers only
  src/               Implementations and main.cpp
  data/              Four unchanged instructor starter files
  testdata/          Collision, facility, department, and integration fixtures
  tests/             Component tests, menu tests, Linux test script
  CMakeLists.txt     Libraries, application, and test targets
  README.txt
  CONTRIBUTIONS.txt
  design_document.txt
  test_results.txt
  .gitignore         Excludes builds, binaries, and editor files

VERIFICATION AND SUBMISSION
Local WSL/GCC and Windows/MSVC checks do not count as actual CELL execution.
See test_results.txt for recorded results and remaining verification.
Before submitting, commit/push the final source, test on CELL, and create the
ZIP from that committed source so required files match the GitHub repository.
Do not include executables or build directories.

CELL CHECK
After connecting to an actual UNT CSE CELL machine, use the final main branch.
For a fresh checkout:
  git clone https://github.com/RAS-UNT/Project2.git Project2-check
  cd Project2-check

For an existing clean checkout instead, enter that directory and update it:
  git switch main
  git pull --ff-only origin main

Then record the machine, revision, compiler, and actual result:
  hostname
  git rev-parse --short HEAD
  g++ --version
  bash tests/run_linux_checks.sh

The script requires GCC and Python 3. If Python 3 is unavailable, use the direct
application build above, run its menu manually, and run the C++ component tests
through CMake when available. Do not label WSL output as a CELL result.
