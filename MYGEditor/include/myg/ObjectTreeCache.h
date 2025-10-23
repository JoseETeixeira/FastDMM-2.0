#pragma once

#include <string>
#include <cstdint>

namespace DMCompiler {
    class DMObjectTree;
}

namespace myg {

/**
 * ObjectTreeCache - Persistence layer for compiled object trees
 * 
 * Saves and loads DMObjectTree to/from binary cache files to avoid
 * recompilation on every editor session.
 * 
 * Cache File Format (.myg_cache.bin):
 * 
 * Header (32 bytes):
 *   - Magic: "MYGC" (4 bytes)
 *   - Version: uint32 (4 bytes)
 *   - DME Timestamp: uint64 (8 bytes)
 *   - Object Count: uint32 (4 bytes)
 *   - String Count: uint32 (4 bytes)
 *   - Reserved: (8 bytes)
 * 
 * String Table:
 *   - For each string:
 *     - Length: uint32 (4 bytes)
 *     - Data: char[length]
 * 
 * Object Tree:
 *   - For each object:
 *     - Path: string_id (uint32)
 *     - Parent ID: int32 (4 bytes)
 *     - Variable Count: uint32 (4 bytes)
 *     - For each variable:
 *       - Name: string_id (uint32)
 *       - Value: string_id (uint32)
 *       - Type: uint8 (1 byte)
 */
class ObjectTreeCache {
public:
    ObjectTreeCache();
    ~ObjectTreeCache();

    /**
     * Load object tree from cache file
     * @param cache_path Path to .myg_cache.bin file
     * @param tree Object tree to populate
     * @return true if loaded successfully, false otherwise
     */
    bool Load(const std::string& cache_path, DMCompiler::DMObjectTree* tree);

    /**
     * Save object tree to cache file
     * @param cache_path Path to .myg_cache.bin file
     * @param tree Object tree to save
     * @return true if saved successfully, false otherwise
     */
    bool Save(const std::string& cache_path, const DMCompiler::DMObjectTree* tree);

private:
    static constexpr uint32_t CACHE_MAGIC = 0x4347594D; // "MYGC"
    static constexpr uint32_t CACHE_VERSION = 1;

    struct CacheHeader {
        uint32_t magic;           // "MYGC"
        uint32_t version;         // Cache format version
        uint64_t dme_timestamp;   // DME file modification time
        uint32_t object_count;    // Number of objects in tree
        uint32_t string_count;    // Number of strings in string table
        uint64_t reserved;        // Reserved for future use
    };

    /**
     * Validate cache header
     * @param header Header to validate
     * @param current_timestamp Current DME timestamp
     * @return true if header is valid, false otherwise
     */
    bool ValidateHeader(const CacheHeader& header, uint64_t current_timestamp);

    /**
     * Serialize object tree to output stream
     * @param out Output stream
     * @param tree Object tree to serialize
     */
    void SerializeObjectTree(std::ostream& out, const DMCompiler::DMObjectTree* tree);

    /**
     * Deserialize object tree from input stream
     * @param in Input stream
     * @param tree Object tree to populate
     * @return true if deserialized successfully, false otherwise
     */
    bool DeserializeObjectTree(std::istream& in, DMCompiler::DMObjectTree* tree);

    /**
     * Write a string to output stream
     * @param out Output stream
     * @param str String to write
     */
    void WriteString(std::ostream& out, const std::string& str);

    /**
     * Read a string from input stream
     * @param in Input stream
     * @param str String to populate
     * @return true if read successfully, false otherwise
     */
    bool ReadString(std::istream& in, std::string& str);

    /**
     * Write a uint32 to output stream
     * @param out Output stream
     * @param value Value to write
     */
    void WriteUInt32(std::ostream& out, uint32_t value);

    /**
     * Read a uint32 from input stream
     * @param in Input stream
     * @param value Value to populate
     * @return true if read successfully, false otherwise
     */
    bool ReadUInt32(std::istream& in, uint32_t& value);

    /**
     * Write a uint64 to output stream
     * @param out Output stream
     * @param value Value to write
     */
    void WriteUInt64(std::ostream& out, uint64_t value);

    /**
     * Read a uint64 from input stream
     * @param in Input stream
     * @param value Value to populate
     * @return true if read successfully, false otherwise
     */
    bool ReadUInt64(std::istream& in, uint64_t& value);
};

} // namespace myg
