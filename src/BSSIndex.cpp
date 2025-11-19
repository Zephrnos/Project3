#include "../headers/BSSIndex.h"
#include "../headers/BSSFile.h"
#include "../headers/BSSBlock.h"
#include "../headers/BSSFileHeader.h"
#include <iostream>

/**
 * @brief Builds the index by scanning all blocks in the BSS file
 * @param bssFile The opened BSS file to index
 */
void BSSIndex::build(BSSFile& bssFile) {
    std::cout << "[BSSIndex::build] Starting index build..." << std::endl;
    std::cout.flush();
    indexMap.clear();
    
    std::cout << "[BSSIndex::build] Getting header..." << std::endl;
    std::cout.flush();
    const BSSFileHeader& header = bssFile.getHeader();
    
    std::cout << "[BSSIndex::build] Header info: blockSize=" << header.getBlockSize()
              << ", blockCount=" << header.getBlockCount()
              << ", listHeadRBN=" << header.getListHeadRBN() << std::endl;
    std::cout.flush();
    
    int rbn = header.getListHeadRBN();
    
    if (rbn == -1) {
        std::cout << "No active blocks in sequence set (listHeadRBN = -1).\n";
        std::cout << "Index built with 0 entries.\n";
        return;
    }
    
    std::cout << "[BSSIndex::build] Starting to process blocks from RBN " << rbn << "..." << std::endl;
    std::cout.flush();

    int blocksProcessed = 0;
    const int MAX_BLOCKS = 10000;  // Safety limit to prevent infinite loops

    while (rbn != -1 && blocksProcessed < MAX_BLOCKS) {
        std::cout << "[BSSIndex::build] Processing block " << rbn << " (block " << (blocksProcessed+1) << ")..." << std::endl;
        std::cout.flush();
        
        BSSBlock block(header.getBlockSize());
        
        std::cout << "[BSSIndex::build] Reading block " << rbn << "..." << std::endl;
        std::cout.flush();
        
        if (!bssFile.readBlock(rbn, block)) {
            std::cerr << "Error reading block " << rbn << " during index build.\n";
            break;
        }

        std::string highestKey = block.getHighestKey();
        if (!highestKey.empty()) {
            indexMap[highestKey] = rbn;
        } else {
            std::cerr << "Warning: Block " << rbn << " has no highest key (empty block?).\n";
        }

        int nextRBN = block.getHeader()->successorRBN;
        if (nextRBN == rbn) {
            std::cerr << "Error: Block " << rbn << " points to itself! Breaking loop.\n";
            break;
        }
        
        rbn = nextRBN;
        blocksProcessed++;
    }
    
    if (blocksProcessed >= MAX_BLOCKS) {
        std::cerr << "Warning: Stopped after " << MAX_BLOCKS << " blocks (possible infinite loop).\n";
    }

    std::cout << "Index built with " << indexMap.size() << " entries from "
              << blocksProcessed << " blocks.\n";
}

/**
 * @brief Finds the RBN of the block that might contain the given key
 * @param key The key to search for
 * @return RBN of the block, or -1 if not found
 */
int BSSIndex::findRBN(const std::string& key) const {
    if (indexMap.empty()) return -1;

    // Find the first block whose highest key >= search key
    auto it = indexMap.lower_bound(key);
    
    if (it == indexMap.end()) {
        // Key is larger than all highest keys, check last block
        if (!indexMap.empty()) {
            return indexMap.rbegin()->second;
        }
        return -1;
    }

    return it->second;
}

/**
 * @brief Writes the index to a binary file
 * @param filename The index file name
 * @return true on success
 */
bool BSSIndex::write(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Error: Cannot open " << filename << " for writing.\n";
        return false;
    }

    uint32_t count = static_cast<uint32_t>(indexMap.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& entry : indexMap) {
        uint16_t keyLen = static_cast<uint16_t>(entry.first.size());
        out.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
        out.write(entry.first.c_str(), keyLen);
        
        int rbn = entry.second;
        out.write(reinterpret_cast<const char*>(&rbn), sizeof(rbn));
    }

    out.close();
    return true;
}

/**
 * @brief Reads the index from a binary file
 * @param filename The index file name
 * @return true on success
 */
bool BSSIndex::read(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Cannot open " << filename << " for reading.\n";
        return false;
    }

    indexMap.clear();

    uint32_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    for (uint32_t i = 0; i < count; ++i) {
        uint16_t keyLen = 0;
        in.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));

        std::string key(keyLen, '\0');
        in.read(&key[0], keyLen);

        int rbn = 0;
        in.read(reinterpret_cast<char*>(&rbn), sizeof(rbn));

        indexMap[key] = rbn;
    }

    in.close();
    std::cout << "Loaded BSS index with " << count << " entries.\n";
    return true;
}

/**
 * @brief Dumps the index contents to an output stream
 * @param os The output stream
 */
void BSSIndex::dump(std::ostream& os) const {
    os << "\n--- BSS Index Dump ---\n";
    os << "Total entries: " << indexMap.size() << "\n\n";
    
    for (const auto& entry : indexMap) {
        os << "Key: " << entry.first << " -> RBN: " << entry.second << "\n";
    }
    
    os << "---------------------\n";
}