#ifndef BLOCKBUFFER_H
#define BLOCKBUFFER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "ZipCodeRecordBuffer.h"

// Simple fixed-size block buffer to hold multiple ZipCodeRecordBuffer objects.
class BlockBuffer {
public:
    BlockBuffer() : maxSize(512), currentSize(0) {}  // Default 512 block

    bool addRecord(const ZipCodeRecordBuffer &record) {
        std::ostringstream ss;
        record.WriteRecord(ss);
        std::string serialized = ss.str();

        if (currentSize + serialized.size() > maxSize)
            return false; // Not enough space

        data.push_back(serialized);
        currentSize += serialized.size();
        return true;
    }

    void writeToFile(const std::string &filename) const {
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            std::cerr << "Error: cannot open file for writing.\n";
            return;
        }

        // Write each record on a separate line
        for (const auto &line : data) {
            out << line << '\n';
        }
    }

    void readFromFile(const std::string &filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Error: cannot open file for reading.\n";
            return;
        }

        data.clear();
        currentSize = 0;
        std::string line;
        while (std::getline(in, line)) {
            data.push_back(line);
            currentSize += line.size();
        }
    }

    void debugPrint(std::ostream &out) const {
        out << "=== BlockBuffer Debug Output ===\n";
        for (const auto &line : data) {
            out << line << '\n';
        }
        out << "===============================\n";
    }

private:
    size_t maxSize;
    size_t currentSize;
    std::vector<std::string> data;
};

#endif // BLOCKBUFFER_H
