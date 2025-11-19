#ifndef BSSFILE_H
#define BSSFILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "BSSFileHeader.h"
#include "BSSBlock.h"
#include "ZipCodeRecordBuffer.h"
#include "BSSIndex.h"
#include "HeaderBuffer.h" // <-- Added Project 2.0 header

/**
 * @brief Main class for managing a Blocked Sequence Set file.
 *
 * This class orchestrates all file operations: creating, opening,
 * reading/writing blocks, and performing dumps.
 */
class BSSFile {
public:
    BSSFile();

    /**
     * @brief Creates a new .bss file from a Project 2.0 .dat file.
     * @note Implements Task 3. Reads from the length-indicated file.
     */
    bool create(const std::string& bssFilename, const std::string& proj2DatFile);

    // Opens an existing .bss file
    bool open(const std::string& bssFilename);

    void close();

    // Reads a block into the provided block object
    bool readBlock(int rbn, BSSBlock& block);
    
    // Writes a block from the provided block object
    bool writeBlock(int rbn, const BSSBlock& block);

    /**
     * @brief Dumps blocks in their physical RBN order (Task 8).
     */
    void dumpPhysical(std::ostream& os);

    /**
     * @brief Dumps blocks by following the logical sequence set links (Task 8).
     */
    void dumpLogical(std::ostream& os);

    /**
     * @brief Adds a record to the BSS file, handling block splits if necessary.
     * @param record The record to add
     * @return True if successful, false otherwise
     */
    bool addRecord(const ZipCodeRecordBuffer& record);

    /**
     * @brief Deletes a record from the BSS file by zip code.
     * @param zipCode The zip code of the record to delete
     * @return True if successful, false otherwise
     */
    bool deleteRecord(const std::string& zipCode);

    /**
     * @brief Gets an available block from the avail list, or creates a new one.
     * @return RBN of available block, or -1 if error
     */
    int getAvailBlock();

    /**
     * @brief Adds a block to the avail list.
     * @param rbn The RBN of the block to add to avail list
     */
    void addToAvailList(int rbn);

    // --- Accessors ---
    const BSSFileHeader& getHeader() const;

private:
    /**
     * @brief Splits a full block into two blocks.
     * @param fullBlockRBN The RBN of the block to split
     * @param newRecord The record that triggered the split
     * @return True if successful
     */
    bool splitBlock(int fullBlockRBN, const ZipCodeRecordBuffer& newRecord);

    /**
     * @brief Finds the correct block to insert a record.
     * @param zipCode The zip code to search for
     * @return RBN of the block where record should be inserted
     */
    int findInsertionBlock(const std::string& zipCode);

    /**
     * @brief Redistributes records between two adjacent blocks.
     * @param rbn1 The RBN of the first block
     * @param rbn2 The RBN of the second block
     * @return True if successful
     */
    bool redistributeBlocks(int rbn1, int rbn2);

    /**
     * @brief Merges two adjacent blocks into one.
     * @param rbn1 The RBN of the first block
     * @param rbn2 The RBN of the second block (will be cleared)
     * @return True if successful
     */
    bool mergeBlocks(int rbn1, int rbn2);

    /**
     * @brief Checks if two blocks should be merged.
     * @param block1 The first block
     * @param block2 The second block
     * @return True if merge is needed
     */
    bool shouldMerge(const BSSBlock& block1, const BSSBlock& block2) const;

    /**
     * @brief Checks if a block is below minimum capacity (50%).
     * @param block The block to check
     * @return True if below minimum capacity
     */
    bool isBelowMinCapacity(const BSSBlock& block) const;

    /**
     * @brief Calculates the minimum number of records for 50% capacity.
     * @return Minimum record count
     */
    uint32_t getMinRecordCount() const;

    std::fstream file;
    BSSFileHeader header;
    uint32_t blockSize;
};

#endif // BSSFILE_H