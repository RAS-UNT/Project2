#ifndef DEPARTMENT_MENU_H
#define DEPARTMENT_MENU_H
 
#include "DepartmentTree.h"
#include <istream>
#include <ostream>
 
namespace DepartmentMenu {
 
void addDepartment(DepartmentTree& tree, std::istream& in, std::ostream& out);
 
void searchDepartment(const DepartmentTree& tree, std::istream& in, std::ostream& out);
 
void displayHierarchy(const DepartmentTree& tree, std::ostream& out);
 
}
 
#endif
