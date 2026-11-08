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
    DepartmentTree() = default;
    DepartmentTree(const DepartmentTree&) = delete;
    DepartmentTree& operator=(const DepartmentTree&) = delete;
    bool isEmpty() const;
    std::size_t size() const;
    AddDepartmentResult addRoot(const std::string& name);
    AddDepartmentResult addChild(const std::string& name, const std::string& parentName);

    const DepartmentNode* find(const std::string& name) const;
    bool search(const std::string& name, DepartmentInfo& info) const;
    void printHierarchy(std::ostream& out) const;
    static std::string describe(AddDepartmentResult result);
private:
    std::unique_ptr<DepartmentNode> root_;
    std::size_t count_ = 0;
    static DepartmentNode* findFrom(DepartmentNode* node, const std::string& key);
    static void printFrom(const DepartmentNode* node, std::size_t depth, std::ostream& out);
};

#endif
