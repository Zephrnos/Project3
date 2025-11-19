#include "../headers/BSSIndex.h"
#include "../headers/BSSFile.h" // Full include needed for build()
#include "../headers/BSSBlock.h" // Full include needed for build()
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

BSSIndex::BSSIndex() {
    // Constructor is empty, but good to have
}

/**
 * @brief Builds the index from an open BSSFile.
 * @param bssFile An open BSSFile object.
 * @note This works by following the logical sequence set.
 */
void BSSIndex::build(BSSFile& bssFile) {
    index.clear();
    int rbn = bssFile.getHeader().getListHeadRBN();
    if (rbn == -1) return; // Empty file

    BSSBlock block(bssFile.getHeader().getBlockSize());

    while (rbn != -1) {
        if (!bssFile.readBlock(rbn, block)) break;
        
        index[block.getHighestKey()] = rbn;
        
        rbn = block.getHeader()->successorRBN;
    }
}

/**
 * @brief Writes the in-memory index to a simple text file.
 * @param filename The output file name.
 */
bool BSSIndex::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) return false;
    for (const auto& pair : index) {
        out << pair.first << "," << pair.second << "\n";
    }
    out.close();
    return true;
}

/**
 * @brief Reads the index from a text file into memory.
 * @param filename The input file name.
 */
bool BSSIndex::read(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    
    index.clear();
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string key;
        std::string rbnStr;
        if (std::getline(ss, key, ',') && std::getline(ss, rbnStr)) {
            try {
                index[key] = std::stoi(rbnStr);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Warning: Skipping invalid index line: " << line << std::endl;
            }
        }
    }
    in.close();
    return true;
}

/**
 * @brief Dumps the index to an output stream (Task 10).
 */
void BSSIndex::dump(std::ostream& os) const {
    os << "--- Simple Index Dump ---\n";
    os << "Highest Key | RBN\n";
    os << "-----------------------\n";
    for (const auto& pair : index) {
        os << std::setw(11) << pair.first << " | " << pair.second << "\n";
    }
    os << "-----------------------\n";
}

/**
 * @brief Finds the correct RBN for a given search key.
 * @param key The Zip Code to search for.
 * @return The RBN of the block that *should* contain the key.
 * @note Implements the search logic for Task 11.
 */
int BSSIndex::findRBN(const std::string& key) const {
    if (index.empty()) return -1;
    
    // std::map::lower_bound finds the first element whose key
    // is *not less than* key (i.e., >= key).
    auto it = index.lower_bound(key);
    
    if (it == index.end()) {
        // Key is greater than all keys in the index.
        // This is an error, or it belongs in the last block.
        // For a simple search, we can return the last block.
        return index.rbegin()->second;
    }
    
    // 'it' points to the first block where HighestKey >= key.
    // This is the block we need to search.
    return it->second;
}