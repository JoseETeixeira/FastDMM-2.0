#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Forward declaration - DMCompiler is in global namespace
namespace DMCompiler {
    class DMObjectTree;
}

namespace myg {

class ObjectInstance {
public:
    std::string type_path;
    std::unordered_map<std::string, std::string> vars;

    std::string GetVar(const std::string& name, ::DMCompiler::DMObjectTree* tree) const;
    void SetVar(const std::string& name, const std::string& value);
    bool IsType(const std::string& path) const;

    std::string ToString() const;
    std::string ToTGMString() const;
};

class TileInstance {
public:
    std::vector<ObjectInstance> objects;

    std::string ToString() const;
    std::string ToTGMString() const;

    static std::unique_ptr<TileInstance> FromString(
        const std::string& str,
        ::DMCompiler::DMObjectTree* tree
    );

    std::vector<ObjectInstance*> GetLayerSorted();
    ObjectInstance* GetArea();

    // Make cache members public so Map can invalidate them
    mutable std::vector<ObjectInstance*> cached_sorted_;
    mutable ObjectInstance* cached_area_;
    mutable bool cache_valid_;
};

} // namespace myg
