#ifndef BLOCKBUFFER_H
#define BLOCKBUFFER_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include "ZipCodeRecordBuffer.h"

// Handles a single block of ZipCodeRecordBuffer records
class BlockBuffer {
public:
    BlockBuffer(uint32_t max = 512) 
        : maxSize(max), currentSize(0), prevBlock(-1), nextBlock(-1) {}

    // Add a record if it fits in this block
    bool addRecord(const ZipCodeRecordBuffer& record) {
        std::ostringstream ss;
        ss << record.getZipCode() << ","
           << record.getPlaceName() << ","
           << record.getState() << ","
           << record.getCounty() << ","
           << record.getLatitude() << ","
           << record.getLongitude();
        std::string serialized = ss.str();

        if (currentSize + serialized.size() > maxSize) return false;

        data.push_back(serialized);
        currentSize += serialized.size();
        return true;
    }

    // Clear block
    void clear() {
        data.clear();
        currentSize = 0;
        prevBlock = -1;
        nextBlock = -1;
    }

    uint32_t getRecordCount() const { return data.size(); }

    // Write block to a binary stream
    bool writeToStream(std::ofstream& out) const {
        uint32_t count = data.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& rec : data) {
            uint32_t len = rec.size();
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(rec.c_str(), len);
        }
        out.write(reinterpret_cast<const char*>(&prevBlock), sizeof(prevBlock));
        out.write(reinterpret_cast<const char*>(&nextBlock), sizeof(nextBlock));
        return true;
    }

    // Read block from a binary stream
    bool readFromStream(std::ifstream& in) {
        clear();
        uint32_t count;
        if (!in.read(reinterpret_cast<char*>(&count), sizeof(count))) return false;

        for (uint32_t i = 0; i < count; ++i) {
            uint32_t len;
            if (!in.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
            std::string rec(len, '\0');
            if (!in.read(&rec[0], len)) return false;
            data.push_back(rec);
        }

        if (!in.read(reinterpret_cast<char*>(&prevBlock), sizeof(prevBlock))) return false;
        if (!in.read(reinterpret_cast<char*>(&nextBlock), sizeof(nextBlock))) return false;

        currentSize = 0;
        for (const auto& rec : data) currentSize += rec.size();
        return true;
    }

    void setPrev(int32_t p) { prevBlock = p; }
    void setNext(int32_t n) { nextBlock = n; }
    int32_t getPrev() const { return prevBlock; }
    int32_t getNext() const { return nextBlock; }

private:
    uint32_t maxSize;
    uint32_t currentSize;
    int32_t prevBlock;
    int32_t nextBlock;
    std::vector<std::string> data;
};

#endif // BLOCKBUFFER_H
