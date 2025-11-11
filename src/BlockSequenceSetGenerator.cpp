#include "BlockSequenceSetGenerator.h"
#include <fstream>
#include <iostream>

// Initialize the generator with the block size
BlockSequenceSetGenerator::BlockSequenceSetGenerator(uint32_t blockSize)
    : blockSize(blockSize) {}

// Add a record to the current block, or start a new block if needed
void BlockSequenceSetGenerator::addRecord(const ZipCodeRecordBuffer& record) {
    // If no blocks yet or record won't fit in current block, create a new block
    if (blocks.empty() || !blocks.back().addRecord(record)) {
        BlockBuffer block(blockSize);
        block.addRecord(record);
        blocks.push_back(block);

        BlockHeaderBuffer header;
        header.setRecordCount(block.getRecordCount());
        headers.push_back(header);
    } else {
        // Update record count in header for the current block
        headers.back().setRecordCount(blocks.back().getRecordCount());
    }
}

// Write all blocks and their headers to a blocked sequence set file
bool BlockSequenceSetGenerator::writeBlocks(const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) return false;

    uint32_t totalBlocks = static_cast<uint32_t>(blocks.size());
    // First write the total number of blocks for Step 4 reading
    out.write(reinterpret_cast<const char*>(&totalBlocks), sizeof(totalBlocks));

    // Write each block header and then the block itself
    for (size_t i = 0; i < blocks.size(); ++i) {
        headers[i].write(out);
        blocks[i].write(out);
    }

    out.close();
    return true;
}
