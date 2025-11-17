#ifndef BSSINDEX_H
#define BSSINDEX_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>

// Forward declaration of BSSFile to avoid circular include
class BSSFile; 

/**
 * @brief Manages the simple primary key index {HighestKey, RBN}.
 * @note Implements Tasks 9, 10, and 11 (search logic).
 */
class BSSIndex {
public:
    BSSIndex();

    /**
     * @brief Builds the index from an open BSSFile.
     * @param bssFile An open BSSFile object.
     * @note This works by following the logical sequence set.
     */
    void build(BSSFile& bssFile);

    /**
     * @brief Writes the in-memory index to a simple text file.
     * @param filename The output file name.
     */
    bool write(const std::string& filename) const;

    /**
     * @brief Reads the index from a text file into memory.
     * @param filename The input file name.
     */
    bool read(const std::string& filename);

    /**
     * @brief Dumps the index to an output stream (Task 10).
     */
    void dump(std::ostream& os) const;

    /**
     * @brief Finds the correct RBN for a given search key.
     * @param key The Zip Code to search for.
     * @return The RBN of the block that *should* contain the key.
     * @note Implements the search logic for Task 11.
     */
    int findRBN(const std::string& key) const;

private:
    // A map automatically keeps keys sorted.
    std::map<std::string, int> index; // {HighestKey, RBN}
};

#endif // BSSINDEX_H