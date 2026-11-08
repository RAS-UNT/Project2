#include "DepartmentTree.h"
#include "TextUtils.h"

bool DepartmentTree::isEmpty() const {
    return root_ == nullptr;
}

std::size_t DepartmentTree::size() const {
    return count_;
}

AddDepartmentResult DepartmentTree::addRoot(const std::string& name) {
    const std::string cleanName = TextUtils::trim(name);
    if (cleanName.empty()) return AddDepartmentResult::BlankName;
    if (root_) return AddDepartmentResult::RootAlreadyExists;

    root_ = std::make_unique<DepartmentNode>();
    root_->name = cleanName;
    ++count_;
    return AddDepartmentResult::Added;
}

AddDepartmentResult DepartmentTree::addChild(const std::string& name, const std::string& parentName) {
    const std::string cleanName = TextUtils::trim(name);
    const std::string cleanParent = TextUtils::trim(parentName);
    if (cleanName.empty()) return AddDepartmentResult::BlankName;
    if (cleanParent.empty()) return AddDepartmentResult::BlankParent;

    const std::string nameKey = TextUtils::normalizeKey(cleanName);
    const std::string parentKey = TextUtils::normalizeKey(cleanParent);
    if (nameKey == parentKey) return AddDepartmentResult::SelfParent;
    if (findFrom(root_.get(), nameKey) != nullptr) return AddDepartmentResult::Duplicate;

    DepartmentNode* parent = findFrom(root_.get(), parentKey);
    if (parent == nullptr) return AddDepartmentResult::ParentNotFound;

    auto child = std::make_unique<DepartmentNode>();
    child->name = cleanName;
    child->parent = parent;
    parent->children.push_back(std::move(child));
    ++count_;
    return AddDepartmentResult::Added;
}

const DepartmentNode* DepartmentTree::find(const std::string& name) const {
    return findFrom(root_.get(), TextUtils::normalizeKey(name));
}

bool DepartmentTree::search(const std::string& name, DepartmentInfo& info) const {
    const DepartmentNode* node = find(name);
    if (node == nullptr) return false;

    info.name = node->name;
    info.parentName = node->parent ? node->parent->name : "None";
    info.childCount = node->children.size();
    info.depth = 0;
    for (const DepartmentNode* step = node->parent; step != nullptr; step = step->parent) {
        ++info.depth;
    }
    return true;
}

void DepartmentTree::printHierarchy(std::ostream& out) const {
    if (!root_) {
        out << "The department tree is empty.\n";
        return;
    }
    printFrom(root_.get(), 0, out);
}

std::string DepartmentTree::describe(AddDepartmentResult result) {
    switch (result) {
        case AddDepartmentResult::Added: return "department added";
        case AddDepartmentResult::BlankName: return "department name is blank";
        case AddDepartmentResult::BlankParent: return "parent name is blank";
        case AddDepartmentResult::Duplicate: return "duplicate department name";
        case AddDepartmentResult::ParentNotFound: return "parent department not found";
        case AddDepartmentResult::RootAlreadyExists: return "a root department already exists";
        case AddDepartmentResult::SelfParent: return "a department cannot be its own parent";
    }
    return "unknown result";
}

DepartmentNode* DepartmentTree::findFrom(DepartmentNode* node, const std::string& key) {
    if (node == nullptr) return nullptr;
    if (TextUtils::normalizeKey(node->name) == key) return node;
    for (const auto& child : node->children) {
        DepartmentNode* match = findFrom(child.get(), key);
        if (match != nullptr) return match;
    }
    return nullptr;
}

void DepartmentTree::printFrom(const DepartmentNode* node, std::size_t depth, std::ostream& out) {
    out << std::string(depth * 4, ' ') << node->name << '\n';
    for (const auto& child : node->children) {
        printFrom(child.get(), depth + 1, out);
    }
}
