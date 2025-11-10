#include "BlockSequenceSetGenerator.h"
#include "BlockHeaderBuffer.h"
#include "BlockBuffer.h"
#include "ZipCodeRecordBuffer.h"
#include <fstream>
#include <iostream>

BlockSequenceSetGenerator::BlockSequenceSetGenerator(uint32_t blockSize)
    : blockSize(blockSize), currentRBN(0) {
    startNewBlock();
}

void BlockSequenceSetGenerator::startNewBlock() {
    // Create new block
    BlockBuffer buf(blockSize);
    blocks.push_back(buf);

    // Create block header
    BlockHeaderBuffer header;
    if (!headers.empty()) {
        header.setPrevRBN(currentRBN);              // current block is previous for new block
        headers.back().setNextRBN(static_cast<int32_t>(blocks.size())); // update previous block nextRBN
    }
    headers.push_back(header);

    currentRBN = static_cast<uint32_t>(blocks.size() - 1);
}

void BlockSequenceSetGenerator::addRecord(const ZipCodeRecordBuffer &record) {
    BlockBuffer &currentBlock = blocks.back();

    if (!currentBlock.addRecord(record)) {
        // current record does not fit, finalize this block and start a new one
        startNewBlock();
        blocks.back().addRecord(record);
    }

    // Update record count in header
    headers.back().setRecordCount(static_cast<uint32_t>(blocks.back().getRecordCount()));
}

bool BlockSequenceSetGenerator::writeBlocks(const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    for (size_t i = 0; i < blocks.size(); ++i) {
        // Write block header
        headers[i].writeHeader(out);

        // Write block data
        blocks[i].writeToStream(out);
    }

    out.close();
    return true;
}
