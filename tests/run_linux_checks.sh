#!/usr/bin/env bash
# Purpose: Builds and tests the integrated Milestone 1 application with GCC.
# Author: Rami Badr
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."
mkdir -p build/linux/facility-database-tests
flags=(-std=c++17 -Wall -Wextra -pedantic -Iinclude)
sources=(src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp
         src/CampusGraph.cpp src/DepartmentTree.cpp src/DepartmentLoader.cpp src/DepartmentMenu.cpp)
objects=()
for source in "${sources[@]}"; do
    object="build/linux/$(basename "${source%.cpp}").o"
    g++ "${flags[@]}" -c "$source" -o "$object"
    objects+=("$object")
done

g++ "${flags[@]}" src/main.cpp "${objects[@]}" -o build/linux/campus
g++ "${flags[@]}" tests/FoundationTests.cpp "${objects[@]}" -o build/linux/foundation_tests
g++ "${flags[@]}" tests/HashTableTests.cpp "${objects[@]}" -o build/linux/hash_table_tests
g++ "${flags[@]}" tests/FacilityDatabaseTests.cpp "${objects[@]}" -o build/linux/facility_database_tests
g++ "${flags[@]}" tests/DepartmentTreeTests.cpp "${objects[@]}" -o build/linux/department_tests

./build/linux/foundation_tests
./build/linux/hash_table_tests
./build/linux/hash_table_tests testdata/hash-collision
./build/linux/facility_database_tests . build/linux/facility-database-tests
./build/linux/department_tests data testdata
python3 tests/MenuIntegrationTests.py build/linux/campus
printf 'All Linux integration checks passed.\n'
