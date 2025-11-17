#include "../headers/BSSFile.h"
#include "../headers/HeaderBuffer.h" // For reading P2.0 header
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>

BSSFile::BSSFile() : blockSize(512) {
    // Default block size, will be overwritten when file is opened
}

/**
 * @brief Creates a new .bss file from a Project 2.0 .dat file.
 * @note Implements Task 3. Reads from the length-indicated file.
 */
bool BSSFile::create(const std::string& bssFilename, const std::string& proj2DatFile) {
    file.open(bssFilename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "Error: Could not create/open file: " << bssFilename << std::endl;
        return false;
    }

    std::ifstream datFile(proj2DatFile, std::ios::binary);
    if (!datFile) {
        std::cerr << "Error: Could not open Project 2.0 DAT file: " << proj2DatFile << std::endl;
        return false;
    }

    // 1. Initialize and write BSS header
    header = BSSFileHeader(blockSize);
    header.setBlockCount(1); // Reserve RBN 0 for header
    header.setListHeadRBN(-1);
    header.setAvailHeadRBN(-1);
    header.write(file);

    // 2. Read all records from Project 2.0 .dat file
    std::vector<ZipCodeRecordBuffer> records;
    
    // Read the Project 2.0 header first
    HeaderRecordBuffer p2Header;
    if (!p2Header.readHeader(datFile)) {
        std::cerr << "Error reading header from " << proj2DatFile << std::endl;
        datFile.close();
        return false;
    }
    
    uint32_t p2RecordCount = p2Header.getRecordCount();
    ZipCodeRecordBuffer tempRec;

    for (uint32_t i = 0; i < p2RecordCount; ++i) {
        uint32_t recordLength;
        if (!datFile.read(reinterpret_cast<char*>(&recordLength), sizeof(recordLength))) {
            std::cerr << "Error reading record length at record " << i << std::endl;
            break;
        }

        std::string recordString(recordLength, '\0');
        if (!datFile.read(&recordString[0], recordLength)) {
            std::cerr << "Error reading record data at record " << i << std::endl;
            break;
        }

        // Use the unpack method to parse the string
        if (tempRec.unpack(recordString)) {
            records.push_back(tempRec);
        } else {
             std::cerr << "Error unpacking record " << i << std::endl;
        }
    }
    datFile.close();

    // Sort records by Zip Code (primary key)
    std::sort(records.begin(), records.end(), [](const auto& a, const auto& b) {
        return a.getZipCode() < b.getZipCode();
    });

    // 3. Pack records into blocks
    header.setRecordCount((uint32_t)records.size());
    int currentRBN = 1; // Start at RBN 1
    int prevRBN = -1;
    
    BSSBlock block(blockSize);
    
    for (const auto& rec : records) {
        if (!block.addRecord(rec)) {
            // Block is full, write it and start a new one
            block.getHeader()->predecessorRBN = prevRBN;
            block.getHeader()->successorRBN = currentRBN + 1;
            block.write(file, currentRBN);

            if (prevRBN == -1) {
                header.setListHeadRBN(currentRBN); // First block
            }
            
            prevRBN = currentRBN;
            currentRBN++;
            block.clear();
            block.addRecord(rec); // Add record to new block
        }
    }

    // 4. Write the last block
    if (block.getHeader()->recordCount > 0) {
        block.getHeader()->predecessorRBN = prevRBN;
        block.getHeader()->successorRBN = -1; // Last block
        block.write(file, currentRBN);
        if (prevRBN == -1) {
             header.setListHeadRBN(currentRBN); // Only one block
        }
    } else {
        // Adjust successor of previous block if last block was empty
        if (prevRBN != -1) {
            BSSBlock prevBlock(blockSize);
            readBlock(prevRBN, prevBlock);
            prevBlock.getHeader()->successorRBN = -1;
            writeBlock(prevRBN, prevBlock);
        }
    }
    
    header.setBlockCount(currentRBN + 1);

    // 5. Write updated header
    header.write(file);
    file.close();
    return true;
}

// Opens an existing .bss file
bool BSSFile::open(const std::string& bssFilename) {
    file.open(bssFilename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;
    
    if (!header.read(file)) return false;
    blockSize = header.getBlockSize();
    return true;
}

void BSSFile::close() {
    if (file.is_open()) file.close();
}

// Reads a block into the provided block object
bool BSSFile::readBlock(int rbn, BSSBlock& block) {
    if (!file.is_open()) return false;
    return block.read(file, rbn, blockSize);
}

// Writes a block from the provided block object
bool BSSFile::writeBlock(int rbn, const BSSBlock& block) {
    if (!file.is_open()) return false;
    return block.write(file, rbn);
}

/**
 * @brief Dumps blocks in their physical RBN order (Task 8).
 */
void BSSFile::dumpPhysical(std::ostream& os) {
    os << "\n--- Physical Block Dump ---\n";
    BSSBlock block(blockSize);
    for (uint32_t rbn = 0; rbn < header.getBlockCount(); ++rbn) {
        if (!readBlock(rbn, block)) break;
        BSSBlock::BlockHeader* h = block.getHeader();
        os << "RBN " << rbn << ": "
           << "Type: " << h->blockType << ", "
           << "Records: " << h->recordCount << ", "
           << "Prev: " << h->predecessorRBN << ", "
           << "Next: " << h->successorRBN << ", "
           << "HighestKey: " << block.getHighestKey() << "\n";
    }
    os << "---------------------------\n";
}

/**
 * @brief Dumps blocks by following the logical sequence set links (Task 8).
 */
void BSSFile::dumpLogical(std::ostream& os) {
    os << "\n--- Logical Block Dump ---\n";
    BSSBlock block(blockSize);
    int rbn = header.getListHeadRBN();
    if (rbn == -1) {
        os << "(No active blocks in sequence set)\n";
        return;
    }
    
    while (rbn != -1) {
        if (!readBlock(rbn, block)) {
            os << "Error reading RBN " << rbn << "!\n";
            break;
        }
        BSSBlock::BlockHeader* h = block.getHeader();
        os << "RBN " << rbn << ": "
           << "Type: " << h->blockType << ", "
           << "Records: " << h->recordCount << ", "
           << "Prev: " << h->predecessorRBN << ", "
           << "Next: " << h->successorRBN << ", "
           << "HighestKey: " << block.getHighestKey() << "\n";
        
        rbn = h->successorRBN;
    }
    os << "-------------------------\n";
}

// --- Accessors ---
const BSSFileHeader& BSSFile::getHeader() const {
    return header;
}