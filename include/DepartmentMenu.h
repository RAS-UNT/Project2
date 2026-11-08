// Purpose: Defines department actions with injectable input and output streams.
// Author: Aqani
#ifndef DEPARTMENT_MENU_H
#define DEPARTMENT_MENU_H

#include "DepartmentTree.h"
#include <istream>
#include <ostream>

namespace DepartmentMenu {

// Adds a root without a parent prompt or a child under an existing parent.
void addDepartment(DepartmentTree& tree, std::istream& in, std::ostream& out);

// Displays a matching department's parent, child count, and depth.
void searchDepartment(const DepartmentTree& tree, std::istream& in, std::ostream& out);

// Prints the indented hierarchy or a clear empty message.
void displayHierarchy(const DepartmentTree& tree, std::ostream& out);

}

#endif
