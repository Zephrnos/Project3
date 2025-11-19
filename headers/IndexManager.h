#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include <map>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>

/**
 * @class IndexManager
 * @brief Builds, stores, and retrieves a ZIP → offset index for binary ZIP data files.
 *
 * The IndexManager is responsible for:
 *  - Scanning a binary data file and extracting ZIP → byte offset mappings.
 *  - Writing this mapping to a compact binary index file.
 *  - Loading the index from disk for fast ZIP code lookups.
 */
class IndexManager {
private:
    std::map<std::string, uint64_t> indexMap;  ///< Maps ZIP code → file offset

public:
    /**
     * @brief Builds the index from a binary data file.
     * @param dataFileName Path to the binary file (e.g., "Data/zip_len.dat").
     */
    void buildIndex(const std::string& dataFileName);

    /**
     * @brief Writes the in-memory index to a binary file.
     * @param indexFileName Path to the output index file (e.g., "Data/zip.idx").
     */
    void writeIndex(const std::string& indexFileName) const;

    /**
     * @brief Loads the index from a binary file into memory.
     * @param indexFileName Path to the input index file.
     */
    void readIndex(const std::string& indexFileName);

    /**
     * @brief Finds the byte offset for a given ZIP code in the index.
     * @param zip The ZIP code to search for.
     * @return The byte offset in the data file, or UINT64_MAX if not found.
     */
    uint64_t findOffset(const std::string& zip) const;

    /**
     * @brief Returns the total number of entries in the index.
     * @return Number of indexed ZIP codes.
     */
    size_t size() const { return indexMap.size(); }
};

#endif // INDEX_MANAGER_H
