#ifndef BLOCKHEADERBUFFER_H
#define BLOCKHEADERBUFFER_H

#include <cstdint>
#include <fstream>

// Block header stores metadata about each block in the blocked sequence set
class BlockHeaderBuffer {
public:
    BlockHeaderBuffer()
        : blockSize(512), recordCount(0), blockCount(0) {}

    // Write the header metadata to a binary stream
    void write(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));
        out.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        out.write(reinterpret_cast<const char*>(&blockCount), sizeof(blockCount));
    }

    // Read the header metadata from a binary stream
    bool read(std::ifstream& in) {
        if (!in.read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize))) return false;
        if (!in.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount))) return false;
        if (!in.read(reinterpret_cast<char*>(&blockCount), sizeof(blockCount))) return false;
        return true;
    }

    // Accessors
    uint32_t getBlockSize() const { return blockSize; }
    uint32_t getBlockCount() const { return blockCount; }

    // Setters for record/block counts
    void setRecordCount(uint32_t rc) { recordCount = rc; }
    void setBlockCount(uint32_t bc) { blockCount = bc; }

private:
    uint32_t blockSize;    // Maximum size of block in bytes
    uint32_t recordCount;  // Number of records in this block
    uint32_t blockCount;   // Total number of blocks in the sequence set
};

#endif
