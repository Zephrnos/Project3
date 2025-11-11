#ifndef BLOCKBUFFER_H
#define BLOCKBUFFER_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include "ZipCodeRecordBuffer.h"

// Handles a block of multiple ZipCodeRecordBuffer records
class BlockBuffer {
public:
    BlockBuffer(uint32_t max = 512) : maxSize(max), currentSize(0) {}

    // Try to add a record to the block; return false if it won't fit
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

    // Clear the block's data
    void clear() { data.clear(); currentSize = 0; }

    uint32_t getRecordCount() const { return data.size(); }

    // Write the block to a binary file
    bool write(std::ofstream& out) const {
        uint32_t count = data.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& rec : data) {
            uint32_t len = rec.size();
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(rec.c_str(), len);
        }
        return true;
    }

    // Read a block from a binary file
    bool read(std::ifstream& in) {
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

        currentSize = 0;
        for (const auto& rec : data) currentSize += rec.size();
        return true;
    }

    // Get a string representation of a record in this block
    std::string getRecord(uint32_t index) const {
        if (index >= data.size()) return "";
        return data[index];
    }

private:
    uint32_t maxSize;       // Maximum allowed size of this block
    uint32_t currentSize;   // Current used size of block
    std::vector<std::string> data; // Serialized record strings
};

#endif
