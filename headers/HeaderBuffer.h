#ifndef HeaderBuffer_H
#define HeaderBuffer_H

#include <fstream>
#include <string>
#include <cstdint>

class HeaderRecordBuffer {
public:
    HeaderRecordBuffer() 
        : version(0), recordCount(0), creationDate("") {
    }

    // Write the header to a binary output stream
    void writeHeader(std::ofstream& output) {
        output.write(reinterpret_cast<const char*>(&version), sizeof(version));
        output.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        
        uint32_t dateLength = creationDate.length();
        output.write(reinterpret_cast<const char*>(&dateLength), sizeof(dateLength));
        output.write(creationDate.c_str(), dateLength);
    }

    // Read the header from a binary input stream
    bool readHeader(std::ifstream& input) {
        if (!input.read(reinterpret_cast<char*>(&version), sizeof(version))) return false;
        if (!input.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount))) return false;

        uint32_t dateLength = 0;
        if (!input.read(reinterpret_cast<char*>(&dateLength), sizeof(dateLength))) return false;

        creationDate.resize(dateLength);
        if (!input.read(&creationDate[0], dateLength)) return false;

        return true;
    }

    // Getters/setters
    void setVersion(uint32_t v) { version = v; }
    void setRecordCount(uint32_t c) { recordCount = c; }
    void setCreationDate(const std::string& d) { creationDate = d; }

    uint32_t getVersion() const { return version; }
    uint32_t getRecordCount() const { return recordCount; }
    std::string getCreationDate() const { return creationDate; }

private:
    uint32_t version;
    uint32_t recordCount;
    std::string creationDate;  // variable-length string
};

#endif // HeaderBuffer_H
