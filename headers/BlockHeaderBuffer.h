#ifndef BLOCKHEADERBUFFER_H
#define BLOCKHEADERBUFFER_H

#include <cstdint>
#include <fstream>

// Header for a single block in the blocked sequence set
class BlockHeaderBuffer {
public:
    BlockHeaderBuffer()
        : recordCount(0), prevRBN(-1), nextRBN(-1) {}

    // Serialize the block header to a binary stream
    void writeHeader(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        out.write(reinterpret_cast<const char*>(&prevRBN), sizeof(prevRBN));
        out.write(reinterpret_cast<const char*>(&nextRBN), sizeof(nextRBN));
    }

    // Deserialize the block header from a binary stream
    bool readHeader(std::ifstream& in) {
        if (!in.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount))) return false;
        if (!in.read(reinterpret_cast<char*>(&prevRBN), sizeof(prevRBN))) return false;
        if (!in.read(reinterpret_cast<char*>(&nextRBN), sizeof(nextRBN))) return false;
        return true;
    }

    // Getters and setters
    void setRecordCount(uint32_t rc) { recordCount = rc; }
    void setPrevRBN(int32_t rbn) { prevRBN = rbn; }
    void setNextRBN(int32_t rbn) { nextRBN = rbn; }

    uint32_t getRecordCount() const { return recordCount; }
    int32_t getPrevRBN() const { return prevRBN; }
    int32_t getNextRBN() const { return nextRBN; }

private:
    uint32_t recordCount;  // number of records currently in this block
    int32_t prevRBN;       // RBN of previous block in logical order
    int32_t nextRBN;       // RBN of next block in logical order
};

#endif // BLOCKHEADERBUFFER_H
