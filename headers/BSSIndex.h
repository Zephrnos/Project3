#ifndef BSSINDEX_H
#define BSSINDEX_H

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdint>

// Forward declarations to avoid circular dependency
class BSSFile;
class BSSFileHeader;
class BSSBlock;

/**
 * @brief Index for the Blocked Sequence Set file
 *
 * Maps the highest key in each block to its RBN for efficient searching.
 * This allows binary search through blocks instead of sequential scanning.
 */
class BSSIndex {
public:
    BSSIndex() = default;

    // Builds the index by scanning all blocks in the BSS file
    void build(BSSFile& bssFile);

    // Finds the RBN of the block that might contain the given key
    int findRBN(const std::string& key) const;

    // Writes the index to a binary file
    bool write(const std::string& filename) const;

    // Reads the index from a binary file
    bool read(const std::string& filename);

    // Dumps the index contents to an output stream
    void dump(std::ostream& os) const;

private:
    std::map<std::string, int> indexMap;  // Maps highest key -> RBN
};

#endif // BSSINDEX_H