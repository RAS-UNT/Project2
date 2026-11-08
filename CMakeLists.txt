cmake_minimum_required(VERSION 3.10)

project(SmartCampusNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(facility_core src/Facility.cpp src/TextUtils.cpp src/HashTable.cpp src/FacilityDatabase.cpp)
target_include_directories(facility_core PUBLIC include)
add_library(graph_core src/CampusGraph.cpp)
target_include_directories(graph_core PUBLIC include)

add_library(department_core src/DepartmentTree.cpp src/DepartmentLoader.cpp src/DepartmentMenu.cpp)
target_include_directories(department_core PUBLIC include)
target_link_libraries(department_core PUBLIC facility_core)

add_executable(campus src/main.cpp)
target_link_libraries(campus PRIVATE facility_core graph_core department_core)

add_executable(foundation_tests tests/FoundationTests.cpp)
target_link_libraries(foundation_tests PRIVATE facility_core)

add_executable(hash_table_tests tests/HashTableTests.cpp)
target_link_libraries(hash_table_tests PRIVATE facility_core)

add_executable(facility_database_tests tests/FacilityDatabaseTests.cpp)
target_link_libraries(facility_database_tests PRIVATE facility_core)

add_executable(department_tests tests/DepartmentTreeTests.cpp)
target_link_libraries(department_tests PRIVATE department_core)

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
add_test(NAME department_components COMMAND department_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/data" "${CMAKE_CURRENT_SOURCE_DIR}/testdata")

if(CMAKE_VERSION VERSION_LESS 3.12)
    find_package(PythonInterp 3 QUIET)
    set(MENU_TEST_PYTHON "${PYTHON_EXECUTABLE}")
else()
    find_package(Python3 COMPONENTS Interpreter QUIET)
    set(MENU_TEST_PYTHON "${Python3_EXECUTABLE}")
endif()
if(MENU_TEST_PYTHON)
    execute_process(COMMAND "${MENU_TEST_PYTHON}" -c "import sys; sys.exit(0)"
        RESULT_VARIABLE PYTHON_RUN_STATUS OUTPUT_QUIET ERROR_QUIET TIMEOUT 5)
endif()
if(MENU_TEST_PYTHON AND "${PYTHON_RUN_STATUS}" STREQUAL "0")
    add_test(NAME integrated_menu COMMAND "${MENU_TEST_PYTHON}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/MenuIntegrationTests.py" "$<TARGET_FILE:campus>")
    set_tests_properties(integrated_menu PROPERTIES TIMEOUT 60)
else()
    message(STATUS "Python 3 unavailable: integrated menu test skipped; component tests remain enabled")
endif()
set_tests_properties(default_data_directory alternate_data_directory PROPERTIES TIMEOUT 10)
set_tests_properties(default_data_directory alternate_data_directory PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
