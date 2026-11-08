// Purpose: Implements department ownership, matching, searches, and tree displays.
// Author: Aqani
#include "DepartmentTree.h"
#include "TextUtils.h"
#include <utility>

// A root-only tree is nonempty even when it has no children.
bool DepartmentTree::isEmpty() const {
    return root_ == nullptr;
}

// Uses the accepted-node count maintained by additions.
std::size_t DepartmentTree::size() const {
    return count_;
}

// Keeps the root's first spelling and rejects another root.
AddDepartmentResult DepartmentTree::addRoot(const std::string& name) {
    const std::string cleanName = TextUtils::trim(name);
    if (cleanName.empty()) return AddDepartmentResult::BlankName;
    if (root_) return AddDepartmentResult::RootAlreadyExists;

    root_ = std::make_unique<DepartmentNode>();
    root_->name = cleanName;
    ++count_;
    return AddDepartmentResult::Added;
}

// Validates names before linking a child into its parent's insertion-ordered list.
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

// Shares the project's trimmed, case-insensitive comparison rules.
const DepartmentNode* DepartmentTree::find(const std::string& name) const {
    return findFrom(root_.get(), TextUtils::normalizeKey(name));
}

// Follows parent links to compute depth without changing node data.
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

// Handles an empty tree before the indented recursive traversal.
void DepartmentTree::printHierarchy(std::ostream& out) const {
    if (!root_) {
        out << "The department tree is empty.\n";
        return;
    }
    printFrom(root_.get(), 0, out);
}

// Supplies the distinct unindented traversal required by menu option 9.
void DepartmentTree::printPreorder(std::ostream& out) const {
    if (!root_) {
        out << "The department tree is empty.\n";
        return;
    }
    preorderFrom(root_.get(), out);
}

// Maps addition outcomes to messages shared by the loader and menu.
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

// Searches a general tree rather than assuming at most two children.
DepartmentNode* DepartmentTree::findFrom(DepartmentNode* node, const std::string& key) {
    if (node == nullptr) return nullptr;
    if (TextUtils::normalizeKey(node->name) == key) return node;
    for (const auto& child : node->children) {
        DepartmentNode* match = findFrom(child.get(), key);
        if (match != nullptr) return match;
    }
    return nullptr;
}

// Indents by edge depth while preserving sibling insertion order.
void DepartmentTree::printFrom(const DepartmentNode* node, std::size_t depth, std::ostream& out) {
    out << std::string(depth * 4, ' ') << node->name << '\n';
    for (const auto& child : node->children) {
        printFrom(child.get(), depth + 1, out);
    }
}

// Emits the parent first and then each complete child subtree.
void DepartmentTree::preorderFrom(const DepartmentNode* node, std::ostream& out) {
    out << node->name << '\n';
    for (const auto& child : node->children) preorderFrom(child.get(), out);
}
