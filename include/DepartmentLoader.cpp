#include "DepartmentLoader.h"
#include "TextUtils.h"
#include <fstream>
#include <vector>

namespace {

void warn(std::ostream& out, const std::string& path, std::size_t lineNumber,
          const std::string& reason) {
    out << "Warning: " << path << " line " << lineNumber << ": " << reason << '\n';
}

}

DepartmentLoadSummary DepartmentLoader::load(const std::string& path, DepartmentTree& tree,
                                             std::ostream& out) {
    DepartmentLoadSummary summary;
    std::ifstream input(path);
    if (!input) {
        out << "Error: could not open " << path << ". No departments were loaded.\n";
        return summary;
    }
    summary.fileOpened = true;

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (TextUtils::isBlankOrComment(line)) continue;

        const std::vector<std::string> fields = TextUtils::splitFields(line);
        if (fields.size() != 2) {
            warn(out, path, lineNumber, "expected 2 fields (DepartmentName|ParentName), found "
                 + std::to_string(fields.size()));
            ++summary.rejected;
            continue;
        }

        const std::string& name = fields[0];
        const std::string& parent = fields[1];
        AddDepartmentResult result;
        if (name.empty()) {
            result = AddDepartmentResult::BlankName;
        } else if (parent.empty()) {
            result = AddDepartmentResult::BlankParent;
        } else if (TextUtils::normalizeKey(parent) == "NONE") {
            result = tree.addRoot(name);
        } else {
            result = tree.addChild(name, parent);
        }

        if (result == AddDepartmentResult::Added) {
            ++summary.loaded;
        } else {
            std::string reason = DepartmentTree::describe(result);
            if (result == AddDepartmentResult::Duplicate) reason += " \"" + name + "\"";
            if (result == AddDepartmentResult::ParentNotFound) reason += " \"" + parent + "\"";
            warn(out, path, lineNumber, reason);
            ++summary.rejected;
        }
    }

    out << "Departments: " << summary.loaded << " loaded, " << summary.rejected
        << " rejected.\n";
    return summary;
}
