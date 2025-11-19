#include "../headers/BSSFile.h"
#include "../headers/HeaderBuffer.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>

BSSFile::BSSFile() : blockSize(512) {
}

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

    header = BSSFileHeader(blockSize);
    header.setBlockCount(1);
    header.setListHeadRBN(-1);
    header.setAvailHeadRBN(-1);
    header.write(file);

    std::vector<ZipCodeRecordBuffer> records;
    HeaderRecordBuffer p2Header;
    if (!p2Header.readHeader(datFile)) {
        std::cerr << "Error reading header from " << proj2DatFile << std::endl;
        datFile.close();
        return false;
    }
    
    uint32_t p2RecordCount = p2Header.getRecordCount();
    ZipCodeRecordBuffer tempRec;
    uint32_t successfulRecords = 0;
    uint32_t skippedRecords = 0;

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

        if (tempRec.unpack(recordString)) {
            records.push_back(tempRec);
            successfulRecords++;
        } else {
            skippedRecords++;
            if (skippedRecords <= 3) {
                std::cerr << "Skipping invalid record " << i << " (likely header fragment)\n";
                if (recordString.length() < 100) {
                    std::cerr << "  Content: '" << recordString << "'\n";
                }
            }
        }
    }
    
    std::cout << "Loaded " << successfulRecords << " valid records (skipped "
              << skippedRecords << " invalid records)\n";
    datFile.close();

    std::sort(records.begin(), records.end(), [](const auto& a, const auto& b) {
        return a.getZipCode() < b.getZipCode();
    });

    header.setRecordCount((uint32_t)records.size());
    int currentRBN = 1;
    int prevRBN = -1;
    
    BSSBlock block(blockSize);
    
    for (const auto& rec : records) {
        if (!block.addRecord(rec)) {
            block.getHeader()->predecessorRBN = prevRBN;
            block.getHeader()->successorRBN = currentRBN + 1;
            block.write(file, currentRBN);

            if (prevRBN == -1) {
                header.setListHeadRBN(currentRBN);
            }
            
            prevRBN = currentRBN;
            currentRBN++;
            block.clear();
            block.addRecord(rec);
        }
    }

    if (block.getHeader()->recordCount > 0) {
        block.getHeader()->predecessorRBN = prevRBN;
        block.getHeader()->successorRBN = -1;
        block.write(file, currentRBN);
        if (prevRBN == -1) {
             header.setListHeadRBN(currentRBN);
        }
    } else {
        if (prevRBN != -1) {
            BSSBlock prevBlock(blockSize);
            readBlock(prevRBN, prevBlock);
            prevBlock.getHeader()->successorRBN = -1;
            writeBlock(prevRBN, prevBlock);
        }
    }
    
    header.setBlockCount(currentRBN + 1);
    std::cout << "Writing header: blockCount=" << header.getBlockCount()
              << ", recordCount=" << header.getRecordCount()
              << ", listHeadRBN=" << header.getListHeadRBN() << "\n";
    header.write(file);
    file.close();
    return true;
}

bool BSSFile::open(const std::string& bssFilename) {
    std::cout << "[BSSFile::open] Opening file: " << bssFilename << "\n";
    file.open(bssFilename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "[BSSFile::open] Failed to open file\n";
        return false;
    }
    
    std::cout << "[BSSFile::open] Reading header...\n";
    if (!header.read(file)) {
        std::cerr << "[BSSFile::open] Failed to read header\n";
        return false;
    }
    
    blockSize = header.getBlockSize();
    std::cout << "[BSSFile::open] Header read successfully. blockSize=" << blockSize
              << ", listHeadRBN=" << header.getListHeadRBN() << "\n";
    return true;
}

void BSSFile::close() {
    if (file.is_open()) file.close();
}

bool BSSFile::readBlock(int rbn, BSSBlock& block) {
    if (!file.is_open()) return false;
    return block.read(file, rbn, blockSize);
}

bool BSSFile::writeBlock(int rbn, const BSSBlock& block) {
    if (!file.is_open()) return false;
    return block.write(file, rbn);
}

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

bool BSSFile::addRecord(const ZipCodeRecordBuffer& record) {
    if (!file.is_open()) {
        std::cerr << "Error: File not open for adding record\n";
        return false;
    }

    std::string zipCode = record.getZipCode();
    int targetRBN = findInsertionBlock(zipCode);
    
    if (targetRBN == -1) {
        std::cerr << "Error: Could not find insertion block\n";
        return false;
    }

    BSSBlock block(blockSize);
    if (!readBlock(targetRBN, block)) {
        std::cerr << "Error: Could not read block " << targetRBN << "\n";
        return false;
    }

    if (block.addRecord(record)) {
        if (!writeBlock(targetRBN, block)) {
            std::cerr << "Error: Could not write block " << targetRBN << "\n";
            return false;
        }
        
        header.setRecordCount(header.getRecordCount() + 1);
        header.write(file);
        
        std::cout << "[ADD] Record " << zipCode << " added to block " << targetRBN << " (no split)\n";
        return true;
    } else {
        std::cout << "[SPLIT] Block " << targetRBN << " is full, splitting...\n";
        return splitBlock(targetRBN, record);
    }
}

bool BSSFile::deleteRecord(const std::string& zipCode) {
    if (!file.is_open()) {
        std::cerr << "Error: File not open for deleting record\n";
        return false;
    }

    int targetRBN = findInsertionBlock(zipCode);
    
    if (targetRBN == -1) {
        std::cerr << "Error: Could not find block for zip code " << zipCode << "\n";
        return false;
    }

    BSSBlock block(blockSize);
    if (!readBlock(targetRBN, block)) {
        std::cerr << "Error: Could not read block " << targetRBN << "\n";
        return false;
    }

    std::vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();
    
    bool found = false;
    for (auto it = records.begin(); it != records.end(); ++it) {
        if (it->getZipCode() == zipCode) {
            records.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "[DELETE] Record " << zipCode << " not found in block " << targetRBN << "\n";
        return false;
    }

    uint32_t minRecords = getMinRecordCount();
    
    if (records.size() >= minRecords) {
        block.clear();
        for (const auto& rec : records) {
            block.addRecord(rec);
        }
        
        if (!writeBlock(targetRBN, block)) {
            std::cerr << "Error: Could not write block " << targetRBN << "\n";
            return false;
        }
        
        header.setRecordCount(header.getRecordCount() - 1);
        header.write(file);
        
        std::cout << "[DELETE] Record " << zipCode << " deleted from block " << targetRBN 
                  << " (no redistribution needed, " << records.size() << " records remain)\n";
        return true;
    } else {
        std::cout << "[DELETE] Record " << zipCode << " deleted from block " << targetRBN 
                  << " (" << records.size() << " records remain, below minimum of " << minRecords << ")\n";
        
        BSSBlock::BlockHeader* h = block.getHeader();
        int adjacentRBN = -1;
        
        if (h->successorRBN != -1) {
            adjacentRBN = h->successorRBN;
        } else if (h->predecessorRBN != -1) {
            adjacentRBN = h->predecessorRBN;
        }
        
        if (adjacentRBN == -1) {
            block.clear();
            for (const auto& rec : records) {
                block.addRecord(rec);
            }
            writeBlock(targetRBN, block);
            header.setRecordCount(header.getRecordCount() - 1);
            header.write(file);
            std::cout << "[DELETE] Only one block in file, no redistribution possible\n";
            return true;
        }
        
        // Check if we should merge or redistribute
        BSSBlock adjacentBlock(blockSize);
        if (!readBlock(adjacentRBN, adjacentBlock)) {
            std::cerr << "Error: Could not read adjacent block\n";
            return false;
        }
        
        if (shouldMerge(block, adjacentBlock)) {
            std::cout << "[MERGE] Merging blocks " << targetRBN << " and " << adjacentRBN << "\n";
            if (mergeBlocks(targetRBN, adjacentRBN)) {
                header.setRecordCount(header.getRecordCount() - 1);
                header.write(file);
                return true;
            }
        } else {
            std::cout << "[REDISTRIBUTE] Redistributing blocks " << targetRBN << " and " << adjacentRBN << "\n";
            if (redistributeBlocks(targetRBN, adjacentRBN)) {
                header.setRecordCount(header.getRecordCount() - 1);
                header.write(file);
                return true;
            }
        }
        
        std::cerr << "Error: Could not handle underflow\n";
        return false;
    }
}

int BSSFile::getAvailBlock() {
    int availRBN = header.getAvailHeadRBN();
    
    if (availRBN != -1) {
        BSSBlock availBlock(blockSize);
        if (readBlock(availRBN, availBlock)) {
            int nextAvailRBN = availBlock.getHeader()->successorRBN;
            header.setAvailHeadRBN(nextAvailRBN);
            header.write(file);
            
            std::cout << "[AVAIL] Reusing block " << availRBN << " from avail list\n";
            return availRBN;
        }
    }
    
    int newRBN = header.getBlockCount();
    header.setBlockCount(newRBN + 1);
    header.write(file);
    
    std::cout << "[NEW] Creating new block " << newRBN << "\n";
    return newRBN;
}

void BSSFile::addToAvailList(int rbn) {
    BSSBlock block(blockSize);
    int currentAvailHead = header.getAvailHeadRBN();
    block.makeAvailBlock(currentAvailHead);
    writeBlock(rbn, block);
    header.setAvailHeadRBN(rbn);
    header.write(file);
    std::cout << "[AVAIL] Block " << rbn << " added to avail list\n";
}

bool BSSFile::splitBlock(int fullBlockRBN, const ZipCodeRecordBuffer& newRecord) {
    BSSBlock fullBlock(blockSize);
    if (!readBlock(fullBlockRBN, fullBlock)) {
        std::cerr << "Error: Could not read block " << fullBlockRBN << " for splitting\n";
        return false;
    }

    std::vector<ZipCodeRecordBuffer> records = fullBlock.unpackAllRecords();
    records.push_back(newRecord);
    
    std::sort(records.begin(), records.end(), [](const auto& a, const auto& b) {
        return a.getZipCode() < b.getZipCode();
    });

    size_t midPoint = records.size() / 2;
    
    BSSBlock block1(blockSize);
    for (size_t i = 0; i < midPoint; ++i) {
        if (!block1.addRecord(records[i])) {
            std::cerr << "Error: Could not add record to block1 during split\n";
            return false;
        }
    }

    int newBlockRBN = getAvailBlock();
    if (newBlockRBN == -1) {
        std::cerr << "Error: Could not get available block for split\n";
        return false;
    }

    BSSBlock block2(blockSize);
    for (size_t i = midPoint; i < records.size(); ++i) {
        if (!block2.addRecord(records[i])) {
            std::cerr << "Error: Could not add record to block2 during split\n";
            return false;
        }
    }

    BSSBlock::BlockHeader* h1 = block1.getHeader();
    BSSBlock::BlockHeader* h2 = block2.getHeader();
    BSSBlock::BlockHeader* oldH = fullBlock.getHeader();

    h1->predecessorRBN = oldH->predecessorRBN;
    h1->successorRBN = newBlockRBN;
    h2->predecessorRBN = fullBlockRBN;
    h2->successorRBN = oldH->successorRBN;

    if (!writeBlock(fullBlockRBN, block1)) {
        std::cerr << "Error: Could not write block1 during split\n";
        return false;
    }
    
    if (!writeBlock(newBlockRBN, block2)) {
        std::cerr << "Error: Could not write block2 during split\n";
        return false;
    }

    if (h2->successorRBN != -1) {
        BSSBlock nextBlock(blockSize);
        if (readBlock(h2->successorRBN, nextBlock)) {
            nextBlock.getHeader()->predecessorRBN = newBlockRBN;
            writeBlock(h2->successorRBN, nextBlock);
        }
    }

    if (h1->predecessorRBN != -1) {
        BSSBlock prevBlock(blockSize);
        if (readBlock(h1->predecessorRBN, prevBlock)) {
            prevBlock.getHeader()->successorRBN = fullBlockRBN;
            writeBlock(h1->predecessorRBN, prevBlock);
        }
    } else {
        header.setListHeadRBN(fullBlockRBN);
    }

    header.setRecordCount(header.getRecordCount() + 1);
    header.write(file);

    std::cout << "[SPLIT] Block " << fullBlockRBN << " split into blocks " 
              << fullBlockRBN << " and " << newBlockRBN << "\n";
    std::cout << "  Block " << fullBlockRBN << " now has " << h1->recordCount 
              << " records (highest: " << block1.getHighestKey() << ")\n";
    std::cout << "  Block " << newBlockRBN << " now has " << h2->recordCount 
              << " records (highest: " << block2.getHighestKey() << ")\n";

    return true;
}

int BSSFile::findInsertionBlock(const std::string& zipCode) {
    int rbn = header.getListHeadRBN();
    
    if (rbn == -1) {
        std::cerr << "Error: No active blocks in file\n";
        return -1;
    }

    BSSBlock block(blockSize);
    
    while (rbn != -1) {
        if (!readBlock(rbn, block)) {
            std::cerr << "Error: Could not read block " << rbn << "\n";
            return -1;
        }

        std::string highestKey = block.getHighestKey();
        int nextRBN = block.getHeader()->successorRBN;

        if (nextRBN == -1 || zipCode <= highestKey) {
            return rbn;
        }

        rbn = nextRBN;
    }

    return -1;
}

bool BSSFile::redistributeBlocks(int rbn1, int rbn2) {
    BSSBlock block1(blockSize);
    BSSBlock block2(blockSize);
    
    if (!readBlock(rbn1, block1) || !readBlock(rbn2, block2)) {
        std::cerr << "Error: Could not read blocks for redistribution\n";
        return false;
    }

    std::vector<ZipCodeRecordBuffer> records1 = block1.unpackAllRecords();
    std::vector<ZipCodeRecordBuffer> records2 = block2.unpackAllRecords();
    
    std::vector<ZipCodeRecordBuffer> allRecords;
    allRecords.insert(allRecords.end(), records1.begin(), records1.end());
    allRecords.insert(allRecords.end(), records2.begin(), records2.end());
    
    std::sort(allRecords.begin(), allRecords.end(), [](const auto& a, const auto& b) {
        return a.getZipCode() < b.getZipCode();
    });

    size_t midPoint = allRecords.size() / 2;
    
    block1.clear();
    for (size_t i = 0; i < midPoint; ++i) {
        if (!block1.addRecord(allRecords[i])) {
            std::cerr << "Error: Could not add record to block1 during redistribution\n";
            return false;
        }
    }

    block2.clear();
    for (size_t i = midPoint; i < allRecords.size(); ++i) {
        if (!block2.addRecord(allRecords[i])) {
            std::cerr << "Error: Could not add record to block2 during redistribution\n";
            return false;
        }
    }

    if (!writeBlock(rbn1, block1) || !writeBlock(rbn2, block2)) {
        std::cerr << "Error: Could not write blocks during redistribution\n";
        return false;
    }

    std::cout << "[REDISTRIBUTE] Blocks " << rbn1 << " and " << rbn2 << " redistributed\n";
    std::cout << "  Block " << rbn1 << ": " << block1.getHeader()->recordCount 
              << " records (highest: " << block1.getHighestKey() << ")\n";
    std::cout << "  Block " << rbn2 << ": " << block2.getHeader()->recordCount 
              << " records (highest: " << block2.getHighestKey() << ")\n";

    return true;
}

bool BSSFile::mergeBlocks(int rbn1, int rbn2) {
    BSSBlock block1(blockSize);
    BSSBlock block2(blockSize);
    
    if (!readBlock(rbn1, block1) || !readBlock(rbn2, block2)) {
        std::cerr << "Error: Could not read blocks for merging\n";
        return false;
    }

    std::vector<ZipCodeRecordBuffer> records1 = block1.unpackAllRecords();
    std::vector<ZipCodeRecordBuffer> records2 = block2.unpackAllRecords();
    
    std::vector<ZipCodeRecordBuffer> allRecords;
    allRecords.insert(allRecords.end(), records1.begin(), records1.end());
    allRecords.insert(allRecords.end(), records2.begin(), records2.end());
    
    std::sort(allRecords.begin(), allRecords.end(), [](const auto& a, const auto& b) {
        return a.getZipCode() < b.getZipCode();
    });

    block1.clear();
    for (const auto& rec : allRecords) {
        if (!block1.addRecord(rec)) {
            std::cerr << "Error: Could not add record to block1 during merge\n";
            return false;
        }
    }

    BSSBlock::BlockHeader* h1 = block1.getHeader();
    BSSBlock::BlockHeader* h2 = block2.getHeader();
    
    h1->successorRBN = h2->successorRBN;

    if (!writeBlock(rbn1, block1)) {
        std::cerr << "Error: Could not write merged block\n";
        return false;
    }

    if (h1->successorRBN != -1) {
        BSSBlock nextBlock(blockSize);
        if (readBlock(h1->successorRBN, nextBlock)) {
            nextBlock.getHeader()->predecessorRBN = rbn1;
            writeBlock(h1->successorRBN, nextBlock);
        }
    }

    addToAvailList(rbn2);

    std::cout << "[MERGE] Blocks " << rbn1 << " and " << rbn2 << " merged into block " << rbn1 << "\n";
    std::cout << "  Merged block " << rbn1 << " now has " << h1->recordCount 
              << " records (highest: " << block1.getHighestKey() << ")\n";
    std::cout << "  Block " << rbn2 << " cleared and added to avail list\n";

    return true;
}

bool BSSFile::shouldMerge(const BSSBlock& block1, const BSSBlock& block2) const {
    uint32_t totalRecords = block1.getHeader()->recordCount + block2.getHeader()->recordCount;
    uint32_t headerSize = sizeof(BSSBlock::BlockHeader);
    uint32_t availableSpace = blockSize - headerSize;
    uint32_t avgRecordSize = 70;
    uint32_t maxRecords = availableSpace / avgRecordSize;
    return totalRecords <= maxRecords;
}

bool BSSFile::isBelowMinCapacity(const BSSBlock& block) const {
    uint32_t minRecords = getMinRecordCount();
    return block.getHeader()->recordCount < minRecords;
}

uint32_t BSSFile::getMinRecordCount() const {
    uint32_t headerSize = sizeof(BSSBlock::BlockHeader);
    uint32_t availableSpace = blockSize - headerSize;
    uint32_t avgRecordSize = 62;
    uint32_t maxRecords = availableSpace / avgRecordSize;
    uint32_t minRecords = maxRecords / 2;
    return (minRecords > 0) ? minRecords : 1;
}

const BSSFileHeader& BSSFile::getHeader() const {
    return header;
}