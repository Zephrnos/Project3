#ifndef BSSBLOCK_H
#define BSSBLOCK_H

#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdint>
#include "ZipCodeRecordBuffer.h"

/**
 * @brief Represents a single block in the file.
 * @note This class implements the "Block Architecture" from the spec.
 *
 * It manages a raw byte buffer and provides methods to pack records
 * into it and unpack them. It contains its own block-level header.
 */
class BSSBlock {
public:
    // This internal struct defines the block-level header
    struct BlockHeader {
        uint32_t recordCount;
        int successorRBN;
        int predecessorRBN;
        char blockType; // 'A' = Active, 'V' = Avail, 'H' = Header (for RBN 0)
    };

    BSSBlock(uint32_t bSize = 512);
    ~BSSBlock();

    // Initializes block to an empty, active state
    void clear();

    /**
     * @brief Converts this block to an avail list block.
     * @param nextAvailRBN The RBN of the next block in the avail list (-1 if this is the last)
     * @note Avail blocks have recordCount == 0 and are filled with blanks (spaces)
     */
    void makeAvailBlock(int nextAvailRBN);

    /**
     * @brief Tries to add a record to this block.
     * @param record The record object to pack.
     * @return True if the record fit, false otherwise.
     */
    bool addRecord(const ZipCodeRecordBuffer& record);

    /**
     * @brief Reads a block from the file at a specific RBN.
     * @param file The file stream.
     * @param rbn The Relative Block Number to read.
     * @param bSize The block size (from file header).
     * @return True on success.
     */
    bool read(std::fstream& file, int rbn, uint32_t bSize);

    // Writes the block's buffer to the file at a specific RBN
    bool write(std::fstream& file, int rbn) const;

    // Gets all records from this block, unpacked.
    std::vector<ZipCodeRecordBuffer> unpackAllRecords() const;

    // --- Accessors ---
    std::string getHighestKey() const { return highestKey; }

    // (Inlined for performance, as it's a simple cast)
    BlockHeader* getHeader() const { return reinterpret_cast<BlockHeader*>(buffer); }

private:
    uint32_t blockSize;
    uint32_t currentSize; // Current write position in buffer
    char* buffer;         // The raw byte buffer
    std::string highestKey; // Highest key in this block
};

#endif // BSSBLOCK_H