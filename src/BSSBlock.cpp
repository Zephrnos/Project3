#include "../headers/BSSBlock.h"
#include <cstring> // For memcpy, memset
#include <string>
#include <vector>

BSSBlock::BSSBlock(uint32_t bSize) : blockSize(bSize), buffer(nullptr), highestKey("") {
    buffer = new char[blockSize];
    clear();
}

BSSBlock::~BSSBlock() {
    delete[] buffer;
}

// Initializes block to an empty, active state
void BSSBlock::clear() {
    memset(buffer, 0, blockSize);
    BlockHeader* header = getHeader();
    header->recordCount = 0;
    header->successorRBN = -1;
    header->predecessorRBN = -1;
    header->blockType = 'A';
    currentSize = sizeof(BlockHeader);
    highestKey = "";
}

/**
 * @brief Tries to add a record to this block.
 * @param record The record object to pack.
 * @return True if the record fit, false otherwise.
 */
bool BSSBlock::addRecord(const ZipCodeRecordBuffer& record) {
    std::string packedRecord = record.pack();
    uint16_t recordLen = (uint16_t)packedRecord.length();
    
    // Check if it fits (Record Length + Record Data)
    if (currentSize + sizeof(recordLen) + recordLen > blockSize) {
        return false;
    }

    // Write length
    char* writePos = buffer + currentSize;
    memcpy(writePos, &recordLen, sizeof(recordLen));
    currentSize += sizeof(recordLen);

    // Write data
    writePos = buffer + currentSize;
    memcpy(writePos, packedRecord.c_str(), recordLen);
    currentSize += recordLen;

    // Update block header and highest key
    getHeader()->recordCount++;
    std::string zip = record.getZipCode();
    if (zip > highestKey) {
        highestKey = zip;
    }
    return true;
}

/**
 * @brief Reads a block from the file at a specific RBN.
 * @param file The file stream.
 * @param rbn The Relative Block Number to read.
 * @param bSize The block size (from file header).
 * @return True on success.
 */
bool BSSBlock::read(std::fstream& file, int rbn, uint32_t bSize) {
    if (blockSize != bSize) {
        delete[] buffer;
        blockSize = bSize;
        buffer = new char[blockSize];
    }
    
    file.seekg((long long)rbn * blockSize, std::ios::beg);
    file.read(buffer, blockSize);
    
    // After reading, parse the block to set internal state
    currentSize = sizeof(BlockHeader); // Start after header
    highestKey = "";
    BlockHeader* header = getHeader();

    for (uint32_t i = 0; i < header->recordCount; ++i) {
        if (currentSize + sizeof(uint16_t) > blockSize) break; // Corrupt block
        
        // Read length
        uint16_t recordLen;
        memcpy(&recordLen, buffer + currentSize, sizeof(uint16_t));
        currentSize += sizeof(uint16_t);

        if (currentSize + recordLen > blockSize) break; // Corrupt block

        // Get record string
        std::string recStr(buffer + currentSize, recordLen);
        currentSize += recordLen;

        // Unpack just to find the key
        ZipCodeRecordBuffer tempRec;
        if (tempRec.unpack(recStr)) {
            if (tempRec.getZipCode() > highestKey) {
                highestKey = tempRec.getZipCode();
            }
        }
    }
    // Reset currentSize to its 'packed' state
    currentSize = sizeof(BlockHeader);
    for(uint32_t i=0; i<getHeader()->recordCount; ++i) {
        uint16_t recordLen;
        memcpy(&recordLen, buffer + currentSize, sizeof(uint16_t));
        currentSize += sizeof(uint16_t) + recordLen;
    }

    return file.good();
}

// Writes the block's buffer to the file at a specific RBN
bool BSSBlock::write(std::fstream& file, int rbn) const {
    file.seekp((long long)rbn * blockSize, std::ios::beg);
    file.write(buffer, blockSize);
    return file.good();
}

// Gets all records from this block, unpacked.
std::vector<ZipCodeRecordBuffer> BSSBlock::unpackAllRecords() const {
    std::vector<ZipCodeRecordBuffer> records;
    BlockHeader* header = getHeader();
    char* readPos = buffer + sizeof(BlockHeader);
    
    for (uint32_t i = 0; i < header->recordCount; ++i) {
        uint16_t recordLen;
        memcpy(&recordLen, readPos, sizeof(uint16_t));
        readPos += sizeof(uint16_t);

        std::string recStr(readPos, recordLen);
        readPos += recordLen;

        ZipCodeRecordBuffer rec;
        if (rec.unpack(recStr)) {
            records.push_back(rec);
        }
    }
    return records;
}