// Purpose: Tests department loading, validation, menu actions, and traversals.
// Author: Aqani
#include "DepartmentLoader.h"
#include "DepartmentMenu.h"
#include "DepartmentTree.h"
#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

// Keeps conditions active in Release builds and reports individual failures.
void check(bool condition, const std::string& label) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << label << '\n';
    if (!condition) ++failures;
}

// Allows surrounding output while checking a required message.
bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

// Verifies the unchanged instructor hierarchy and search details.
void testStarterFile(const std::string& dataDir) {
    DepartmentTree tree;
    std::ostringstream out;
    DepartmentLoadSummary summary = DepartmentLoader::load(dataDir + "/departments.txt", tree, out);
    check(summary.fileOpened, "starter departments.txt opens");
    check(summary.loaded == 7 && summary.rejected == 0, "starter file loads 7, rejects 0");

    std::ostringstream hierarchy;
    tree.printHierarchy(hierarchy);
    const std::string expected =
        "University\n"
        "    College of Engineering\n"
        "        Computer Science\n"
        "        Electrical Engineering\n"
        "    College of Science\n"
        "        Mathematics\n"
        "        Physics\n";
    check(hierarchy.str() == expected, "preorder hierarchy uses four-space indentation");
    std::ostringstream preorder;
    tree.printPreorder(preorder);
    check(preorder.str() == "University\nCollege of Engineering\nComputer Science\nElectrical Engineering\nCollege of Science\nMathematics\nPhysics\n",
          "separate preorder traversal has no indentation");

    DepartmentInfo info;
    check(tree.search("university", info) && info.parentName == "None" && info.depth == 0
          && info.childCount == 2, "root search: parent None, depth 0, 2 children");
    check(tree.search("  COMPUTER science ", info) && info.name == "Computer Science"
          && info.parentName == "College of Engineering" && info.depth == 2
          && info.childCount == 0, "case-insensitive leaf search: depth 2, 0 children");
    check(!tree.search("History", info), "unknown department is not found");
}

// Checks physical line numbers and recovery with the malformed fixture.
void testBadFile(const std::string& testDir) {
    DepartmentTree tree;
    std::ostringstream out;
    const std::string path = testDir + "/departments_bad/departments.txt";
    DepartmentLoadSummary summary = DepartmentLoader::load(path, tree, out);
    const std::string log = out.str();

    check(summary.loaded == 4, "bad file: 4 valid records loaded");
    check(summary.rejected == 9, "bad file: 9 records rejected");
    check(contains(log, path + " line 5: parent department not found"), "unknown parent rejected");
    check(contains(log, "line 6: duplicate department name"), "case-insensitive duplicate rejected");
    check(contains(log, "line 7: a root department already exists"), "second root rejected");
    check(contains(log, "line 8: department name is blank"), "blank name rejected");
    check(contains(log, "line 9: parent name is blank"), "blank parent rejected");
    check(contains(log, "line 10: expected 2 fields"), "too many fields rejected");
    check(contains(log, "line 11: expected 2 fields"), "missing separator rejected");
    check(contains(log, "line 12: a department cannot be its own parent"), "self parent rejected");
    check(contains(log, "line 14: parent department not found"), "parent defined later is rejected");
    check(contains(log, path + ": 4 accepted, 9 rejected"), "filename and load counts printed");

    DepartmentInfo info;
    check(tree.search("University", info) && info.name == "University",
          "outer whitespace trimmed from names");
    check(tree.search("Robotics Lab", info) && info.parentName == "College of Engineering",
          "parent matched case-insensitively, original spelling kept");
}

// Ensures missing and comment-only inputs produce usable empty-tree behavior.
void testMissingAndEmpty(const std::string& testDir) {
    DepartmentTree tree;
    std::ostringstream out;
    DepartmentLoadSummary summary = DepartmentLoader::load("no_such_dir/departments.txt", tree, out);
    check(!summary.fileOpened && contains(out.str(), "could not open"), "missing file reported");
    check(contains(out.str(), "no_such_dir/departments.txt: 0 accepted, 0 rejected"), "missing file has zero-count summary");

    summary = DepartmentLoader::load(testDir + "/departments_empty/departments.txt", tree, out);
    check(summary.fileOpened && summary.loaded == 0 && summary.rejected == 0 && tree.isEmpty(),
          "comment-only file opens and leaves tree empty");

    std::ostringstream display;
    DepartmentMenu::displayHierarchy(tree, display);
    check(display.str() == "The department tree is empty.\n", "empty display message");

    std::istringstream searchInput("University\n");
    std::ostringstream searchOut;
    DepartmentMenu::searchDepartment(tree, searchInput, searchOut);
    check(contains(searchOut.str(), "empty"), "empty search message");
}

// Tests root creation without a parent prompt and arbitrary child counts.
void testMenuAdd() {
    DepartmentTree tree;
    std::istringstream rootInput("School of Music\n");
    std::ostringstream rootOut;
    DepartmentMenu::addDepartment(tree, rootInput, rootOut);
    check(!contains(rootOut.str(), "parent"), "first add does not ask for a parent");
    check(contains(rootOut.str(), "as the root"), "first department becomes root");

    std::istringstream childInput("Jazz Studies\nschool of music\n");
    std::ostringstream childOut;
    DepartmentMenu::addDepartment(tree, childInput, childOut);
    check(contains(childOut.str(), "Added \"Jazz Studies\" under \"School of Music\""),
          "child added under existing parent (names with spaces)");

    std::istringstream dupInput("JAZZ STUDIES\nSchool of Music\n");
    std::ostringstream dupOut;
    DepartmentMenu::addDepartment(tree, dupInput, dupOut);
    check(contains(dupOut.str(), "duplicate"), "duplicate add rejected");

    std::istringstream badParent("Opera\nTheatre\n");
    std::ostringstream badOut;
    DepartmentMenu::addDepartment(tree, badParent, badOut);
    check(contains(badOut.str(), "parent department not found"), "unknown parent rejected on add");

    std::istringstream blankInput("   \n");
    std::ostringstream blankOut;
    DepartmentMenu::addDepartment(tree, blankInput, blankOut);
    check(contains(blankOut.str(), "cannot be blank"), "blank name rejected on add");

    std::istringstream eofInput("");
    std::ostringstream eofOut;
    DepartmentMenu::addDepartment(tree, eofInput, eofOut);
    check(tree.size() == 2, "end of input returns without changes");

    for (int i = 1; i <= 6; ++i) {
        tree.addChild("Ensemble " + std::to_string(i), "Jazz Studies");
    }
    DepartmentInfo info;
    check(tree.search("Jazz Studies", info) && info.childCount == 6,
          "a node can hold more than two children");
}

}

// Uses caller-selected fixture directories without modifying their files.
int main(int argc, char* argv[]) {
    const std::string dataDir = argc > 1 ? argv[1] : "data";
    const std::string testDir = argc > 2 ? argv[2] : "testdata";
    testStarterFile(dataDir);
    testBadFile(testDir);
    testMissingAndEmpty(testDir);
    testMenuAdd();
    std::cout << (failures == 0 ? "All department tests passed.\n" : "Some department tests failed.\n");
    return failures == 0 ? 0 : 1;
}
