#ifndef BLOCK_SEQUENCE_SET_GENERATOR_H
#define BLOCK_SEQUENCE_SET_GENERATOR_H

#include "BlockBuffer.h"
#include "BlockHeaderBuffer.h"
#include "ZipCodeRecordBuffer.h"

#include <vector>
#include <string>

class BlockSequenceSetGenerator {
public:
    BlockSequenceSetGenerator(uint32_t blockSize = 512);

    // Add a record; will create new blocks if needed
    void addRecord(const ZipCodeRecordBuffer &record);

    // Write all blocks to the output .bin file
    bool writeBlocks(const std::string &filename);

    uint32_t getBlockCount() const { return static_cast<uint32_t>(blocks.size()); }
private:
    uint32_t blockSize;
    std::vector<BlockBuffer> blocks;
    std::vector<BlockHeaderBuffer> headers;
    uint32_t currentRBN;

    void startNewBlock();
};

#endif // BLOCK_SEQUENCE_SET_GENERATOR_H
