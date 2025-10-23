#include "myg/ObjectTreeCache.h"
#include <DMObjectTree.h>
#include <DMObject.h>
#include <DMVariable.h>
#include <fstream>
#include <iostream>
#include <cstring>

namespace myg {

ObjectTreeCache::ObjectTreeCache() = default;

ObjectTreeCache::~ObjectTreeCache() = default;

bool ObjectTreeCache::Load(const std::string& cache_path, DMCompiler::DMObjectTree* tree) {
    if (!tree) {
        std::cerr << "ObjectTreeCache::Load: tree is null" << std::endl;
        return false;
    }

    std::ifstream in(cache_path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "ObjectTreeCache::Load: Failed to open cache file: " << cache_path << std::endl;
        return false;
    }

    // Read header
    CacheHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(CacheHeader));
    
    if (!in.good()) {
        std::cerr << "ObjectTreeCache::Load: Failed to read header" << std::endl;
        return false;
    }

    // Validate header (skip timestamp validation for now)
    if (!ValidateHeader(header, 0)) {
        std::cerr << "ObjectTreeCache::Load: Invalid header" << std::endl;
        return false;
    }

    std::cout << "Cache header valid - Version: " << header.version 
              << ", Objects: " << header.object_count 
              << ", Strings: " << header.string_count << std::endl;

    // Deserialize object tree
    if (!DeserializeObjectTree(in, tree)) {
        std::cerr << "ObjectTreeCache::Load: Failed to deserialize object tree" << std::endl;
        return false;
    }

    std::cout << "Successfully loaded object tree from cache" << std::endl;
    return true;
}

bool ObjectTreeCache::Save(const std::string& cache_path, const DMCompiler::DMObjectTree* tree) {
    if (!tree) {
        std::cerr << "ObjectTreeCache::Save: tree is null" << std::endl;
        return false;
    }

    std::ofstream out(cache_path, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "ObjectTreeCache::Save: Failed to create cache file: " << cache_path << std::endl;
        return false;
    }

    // Prepare header
    CacheHeader header;
    header.magic = CACHE_MAGIC;
    header.version = CACHE_VERSION;
    header.dme_timestamp = 0; // TODO: Get actual DME timestamp
    header.object_count = static_cast<uint32_t>(tree->AllObjects.size());
    header.string_count = static_cast<uint32_t>(tree->StringTable.size());
    header.reserved = 0;

    // Write header
    out.write(reinterpret_cast<const char*>(&header), sizeof(CacheHeader));
    
    if (!out.good()) {
        std::cerr << "ObjectTreeCache::Save: Failed to write header" << std::endl;
        return false;
    }

    std::cout << "Writing cache - Objects: " << header.object_count 
              << ", Strings: " << header.string_count << std::endl;

    // Serialize object tree
    SerializeObjectTree(out, tree);

    if (!out.good()) {
        std::cerr << "ObjectTreeCache::Save: Failed to serialize object tree" << std::endl;
        return false;
    }

    std::cout << "Successfully saved object tree to cache" << std::endl;
    return true;
}

bool ObjectTreeCache::ValidateHeader(const CacheHeader& header, uint64_t current_timestamp) {
    if (header.magic != CACHE_MAGIC) {
        std::cerr << "Invalid cache magic: " << std::hex << header.magic << std::dec << std::endl;
        return false;
    }

    if (header.version != CACHE_VERSION) {
        std::cerr << "Unsupported cache version: " << header.version << std::endl;
        return false;
    }

    // Skip timestamp validation for now
    // if (current_timestamp > 0 && header.dme_timestamp < current_timestamp) {
    //     std::cerr << "Cache is older than DME file" << std::endl;
    //     return false;
    // }

    return true;
}

void ObjectTreeCache::SerializeObjectTree(std::ostream& out, const DMCompiler::DMObjectTree* tree) {
    // Write string table
    for (const auto& str : tree->StringTable) {
        WriteString(out, str);
    }

    // Write objects
    for (const auto& obj : tree->AllObjects) {
        if (!obj) {
            continue;
        }

        // Write path as string
        WriteString(out, obj->Path.ToString());

        // Write parent ID
        int32_t parent_id = obj->Parent ? obj->Parent->Id : -1;
        out.write(reinterpret_cast<const char*>(&parent_id), sizeof(int32_t));

        // Write variable count
        uint32_t var_count = static_cast<uint32_t>(obj->Variables.size());
        WriteUInt32(out, var_count);

        // Write variables
        for (const auto& var_pair : obj->Variables) {
            WriteString(out, var_pair.first);
            
            // Write variable flags
            uint8_t flags = 0;
            if (var_pair.second.IsGlobal) flags |= 0x01;
            if (var_pair.second.IsConst) flags |= 0x02;
            if (var_pair.second.IsFinal) flags |= 0x04;
            if (var_pair.second.IsTmp) flags |= 0x08;
            out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
            
            // Write variable type path (if present)
            bool has_type = var_pair.second.Type.has_value();
            out.write(reinterpret_cast<const char*>(&has_type), sizeof(bool));
            if (has_type) {
                WriteString(out, var_pair.second.Type->ToString());
            }
        }
    }
}

bool ObjectTreeCache::DeserializeObjectTree(std::istream& in, DMCompiler::DMObjectTree* tree) {
    // Note: Full deserialization is complex and requires deep knowledge of DMObjectTree internals
    // For now, this is a placeholder implementation
    // A complete implementation would need to:
    // 1. Read string table
    // 2. Reconstruct all DMObject instances
    // 3. Rebuild parent-child relationships
    // 4. Restore all variables
    // 5. Rebuild path lookup maps
    
    std::cerr << "ObjectTreeCache::DeserializeObjectTree: Not fully implemented yet" << std::endl;
    std::cerr << "Cache loading is disabled until full implementation is complete" << std::endl;
    
    // For now, return false to force recompilation
    return false;
}

void ObjectTreeCache::WriteString(std::ostream& out, const std::string& str) {
    uint32_t length = static_cast<uint32_t>(str.length());
    WriteUInt32(out, length);
    out.write(str.data(), length);
}

bool ObjectTreeCache::ReadString(std::istream& in, std::string& str) {
    uint32_t length;
    if (!ReadUInt32(in, length)) {
        return false;
    }

    if (length > 1024 * 1024) { // Sanity check: max 1MB string
        std::cerr << "String length too large: " << length << std::endl;
        return false;
    }

    str.resize(length);
    in.read(&str[0], length);
    
    return in.good();
}

void ObjectTreeCache::WriteUInt32(std::ostream& out, uint32_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
}

bool ObjectTreeCache::ReadUInt32(std::istream& in, uint32_t& value) {
    in.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    return in.good();
}

void ObjectTreeCache::WriteUInt64(std::ostream& out, uint64_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(uint64_t));
}

bool ObjectTreeCache::ReadUInt64(std::istream& in, uint64_t& value) {
    in.read(reinterpret_cast<char*>(&value), sizeof(uint64_t));
    return in.good();
}

} // namespace myg
