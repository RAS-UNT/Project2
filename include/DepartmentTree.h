// Purpose: Defines the general department tree, searches, and preorder displays.
// Author: Aqani
#ifndef DEPARTMENT_TREE_H
#define DEPARTMENT_TREE_H

#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct DepartmentNode {
    std::string name;
    DepartmentNode* parent = nullptr;
    std::vector<std::unique_ptr<DepartmentNode>> children;
};

enum class AddDepartmentResult {
    Added,
    BlankName,
    BlankParent,
    Duplicate,
    ParentNotFound,
    RootAlreadyExists,
    SelfParent
};

struct DepartmentInfo {
    std::string name;
    std::string parentName;
    std::size_t childCount = 0;
    std::size_t depth = 0;
};

class DepartmentTree {
public:
    // Starts with no departments and keeps node ownership inside the tree.
    DepartmentTree() = default;
    // Prevents copying nodes whose parent pointers belong to this tree.
    DepartmentTree(const DepartmentTree&) = delete;
    DepartmentTree& operator=(const DepartmentTree&) = delete;
    // Indicates whether a root has been accepted.
    bool isEmpty() const;
    // Counts accepted departments, including menu additions.
    std::size_t size() const;
    // Creates the first department without requiring a parent name.
    AddDepartmentResult addRoot(const std::string& name);
    // Adds a unique department under a previously accepted parent.
    AddDepartmentResult addChild(const std::string& name, const std::string& parentName);

    // Finds a trimmed name without regard to ASCII capitalization.
    const DepartmentNode* find(const std::string& name) const;
    // Reports parent, direct-child count, and edge depth for a match.
    bool search(const std::string& name, DepartmentInfo& info) const;
    // Prints preorder with four spaces of indentation per depth.
    void printHierarchy(std::ostream& out) const;
    // Prints preorder without hierarchy indentation.
    void printPreorder(std::ostream& out) const;
    // Explains an addition outcome without changing the tree.
    static std::string describe(AddDepartmentResult result);
private:
    std::unique_ptr<DepartmentNode> root_;
    std::size_t count_ = 0;
    // Searches this node and each child's subtree in insertion order.
    static DepartmentNode* findFrom(DepartmentNode* node, const std::string& key);
    // Recursively formats a hierarchy using edge depth.
    static void printFrom(const DepartmentNode* node, std::size_t depth, std::ostream& out);
    // Visits a parent before its children without adding indentation.
    static void preorderFrom(const DepartmentNode* node, std::ostream& out);
};

#endif
