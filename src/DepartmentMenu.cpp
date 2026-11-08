// Purpose: Implements department additions, searches, and hierarchy menu actions.
// Author: Aqani
#include "DepartmentMenu.h"
#include "TextUtils.h"
#include <string>

namespace {

// Reads names containing spaces and returns cleanly when input ends.
bool readName(std::istream& in, std::ostream& out, const std::string& prompt, std::string& value) {
    out << prompt;
    std::string line;
    if (!std::getline(in, line)) {
        out << '\n';
        return false;
    }
    value = TextUtils::trim(line);
    return true;
}

}

// Changes only the in-memory tree, never the source data file.
void DepartmentMenu::addDepartment(DepartmentTree& tree, std::istream& in, std::ostream& out) {
    std::string name;
    if (!readName(in, out, "Enter new department name: ", name)) return;
    if (name.empty()) {
        out << "Error: department name cannot be blank.\n";
        return;
    }

    AddDepartmentResult result;
    std::string parent;
    if (tree.isEmpty()) {
        result = tree.addRoot(name);
        if (result == AddDepartmentResult::Added) {
            out << "Added \"" << name << "\" as the root department.\n";
            return;
        }
    } else {
        if (!readName(in, out, "Enter parent department name: ", parent)) return;
        if (parent.empty()) {
            out << "Error: parent name cannot be blank.\n";
            return;
        }
        result = tree.addChild(name, parent);
        if (result == AddDepartmentResult::Added) {
            const DepartmentNode* added = tree.find(name);
            out << "Added \"" << name << "\" under \"" << added->parent->name << "\".\n";
            return;
        }
    }
    out << "Error: " << DepartmentTree::describe(result);
    if (result == AddDepartmentResult::Duplicate) out << " \"" << name << "\"";
    if (result == AddDepartmentResult::ParentNotFound) out << " \"" << parent << "\"";
    out << ". Department not added.\n";
}

// Reports required search details without changing the tree.
void DepartmentMenu::searchDepartment(const DepartmentTree& tree, std::istream& in,
                                      std::ostream& out) {
    if (tree.isEmpty()) {
        out << "The department tree is empty. There is nothing to search.\n";
        return;
    }
    std::string name;
    if (!readName(in, out, "Enter department name to search: ", name)) return;
    if (name.empty()) {
        out << "Error: department name cannot be blank.\n";
        return;
    }

    DepartmentInfo info;
    if (!tree.search(name, info)) {
        out << "Department \"" << name << "\" not found.\n";
        return;
    }
    out << "Department: " << info.name << '\n'
        << "Parent: " << info.parentName << '\n'
        << "Children: " << info.childCount << '\n'
        << "Depth: " << info.depth << '\n';
}

// Keeps hierarchy indentation inside the tree implementation.
void DepartmentMenu::displayHierarchy(const DepartmentTree& tree, std::ostream& out) {
    if (!tree.isEmpty()) {
        out << "Department hierarchy (" << tree.size() << " departments):\n";
    }
    tree.printHierarchy(out);
}
