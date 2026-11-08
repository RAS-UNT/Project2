#ifndef DEPARTMENT_LOADER_H
#define DEPARTMENT_LOADER_H
 
#include "DepartmentTree.h"
#include <ostream>
#include <string>
 
struct DepartmentLoadSummary {
    bool fileOpened = false;
    std::size_t loaded = 0;
    std::size_t rejected = 0;
};
 
namespace DepartmentLoader {
 
DepartmentLoadSummary load(const std::string& path, DepartmentTree& tree, std::ostream& out);
 
}
 
#endif
 
