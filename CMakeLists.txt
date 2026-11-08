cmake_minimum_required(VERSION 3.10)

project(SmartCampusNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(facility_core src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp)
target_include_directories(facility_core PUBLIC include)

add_executable(campus src/main.cpp)
target_link_libraries(campus PRIVATE facility_core)

add_executable(foundation_tests tests/FoundationTests.cpp)
target_link_libraries(foundation_tests PRIVATE facility_core)

add_executable(hash_table_tests tests/HashTableTests.cpp)
target_link_libraries(hash_table_tests PRIVATE facility_core)

add_executable(facility_database_tests tests/FacilityDatabaseTests.cpp)
target_link_libraries(facility_database_tests PRIVATE facility_core)

enable_testing()
add_test(NAME facility_foundation COMMAND foundation_tests)
add_test(NAME default_data_directory COMMAND campus)
set_tests_properties(default_data_directory PROPERTIES PASS_REGULAR_EXPRESSION "Data directory: data/")
add_test(NAME alternate_data_directory COMMAND campus testdata/example)
set_tests_properties(alternate_data_directory PROPERTIES PASS_REGULAR_EXPRESSION "Data directory: testdata/example")
add_test(NAME extra_arguments_rejected COMMAND campus first second)
set_tests_properties(extra_arguments_rejected PROPERTIES WILL_FAIL TRUE)
add_test(NAME blank_data_directory_rejected COMMAND campus " ")
set_tests_properties(blank_data_directory_rejected PROPERTIES WILL_FAIL TRUE)
add_test(NAME facility_hash_table COMMAND hash_table_tests)
add_test(NAME hash_collision_fixture COMMAND hash_table_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/testdata/hash-collision")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/facility-database-tests")
add_test(NAME facility_database COMMAND facility_database_tests
    "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/facility-database-tests")
