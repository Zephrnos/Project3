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

    // --- Accessors ---
    const BSSFileHeader& getHeader() const;

private:
    std::fstream file;
    BSSFileHeader header;
    uint32_t blockSize;
};

#endif // BSSFILE_H