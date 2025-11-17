#ifndef BSSFILEHEADER_H
#define BSSFILEHEADER_H

#include <cstdint>
#include <fstream>
#include <string>
#include <cstring>

/**
 * @brief Manages the master header for the Blocked Sequence Set file.
 * @note This class implements the "Header Record Architecture" from Task 6.
 *
 * It stores metadata about the entire file, such as block size, counts,
 * and RBN links to the first active block and the first available block.
 */
class BSSFileHeader {
public:
    BSSFileHeader(uint32_t bSize = 512)
        : version(1), headerRecordSize(sizeof(BSSFileHeader)),
          blockSize(bSize), minBlockCapacity(50), recordCount(0),
          blockCount(0), listHeadRBN(-1), availHeadRBN(-1), stale(false) {
        // Initialize file structure type string
        strncpy(fileStructureType, "BSS_COMMA_LEN_IND", 32);
        fileStructureType[31] = '\0';
        // (Other fields like index file name could be added here)
    }

    // Writes the header object directly to the start of the file
    bool write(std::fstream& file) const {
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<const char*>(this), sizeof(BSSFileHeader));
        return file.good();
    }

    // Reads the header object directly from the start of the file
    bool read(std::fstream& file) {
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(this), sizeof(BSSFileHeader));
        return file.good();
    }

    // --- Accessors and Mutators ---
    uint32_t getBlockSize() const { return blockSize; }
    uint32_t getRecordCount() const { return recordCount; }
    uint32_t getBlockCount() const { return blockCount; }
    int getListHeadRBN() const { return listHeadRBN; }
    int getAvailHeadRBN() const { return availHeadRBN; }

    void setRecordCount(uint32_t count) { recordCount = count; }
    void setBlockCount(uint32_t count) { blockCount = count; }
    void setListHeadRBN(int rbn) { listHeadRBN = rbn; }
    void setAvailHeadRBN(int rbn) { availHeadRBN = rbn; }

private:
    // Fields from "Header Record Architecture"
    char fileStructureType[32]; // e.g., "BSS_COMMA_LEN_IND"
    uint32_t version;
    uint32_t headerRecordSize;  // Size of this struct
    // (Could add 'bytes for record size' and 'size format type' if needed)
    uint32_t blockSize;         // Default 512
    uint32_t minBlockCapacity;  // Default 50%
    // (Could add index file name and schema)
    
    uint32_t recordCount;       // Total records in the file
    uint32_t blockCount;        // Total blocks (active + avail)
    // (Could add 'fields per record' and field schemas)
    
    int listHeadRBN;            // RBN of the first block in the active sequence
    int availHeadRBN;           // RBN of the first block in the avail list
    bool stale;                 // Flag for index validity (not used yet)
};

#endif // BSSFILEHEADER_H