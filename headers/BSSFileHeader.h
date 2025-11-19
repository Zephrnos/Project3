#ifndef BSSFILEHEADER_H
#define BSSFILEHEADER_H

#include <cstdint>
#include <fstream>
#include <string>
#include <cstring>

/**
 * @brief Manages the master header for the Blocked Sequence Set file.
 * @note This class implements the "Header Record Architecture" from Task 6.
 *
 * It stores metadata about the entire file, such as block size, counts,
 * and RBN links to the first active block and the first available block.
 */
class BSSFileHeader {
public:
    BSSFileHeader(uint32_t bSize = 512)
        : version(1), headerRecordSize(sizeof(BSSFileHeader)),
          blockSize(bSize), minBlockCapacity(50), recordCount(0),
          blockCount(0), listHeadRBN(-1), availHeadRBN(-1), stale(false),
          // NEW fields:
          recordSizeFieldBytes(4),   // 4-byte length integer
          sizeFormatType('B'),       // 'B' = binary, 'A' = ASCII
          fieldCount(6),             // Zip, Place, State, County, Lat, Lon
          primaryKeyFieldIndex(0)    // ZipCode is primary key 
    {
        // Initialize file structure type string
        memset(fileStructureType, 0, sizeof(fileStructureType));
        strncpy(fileStructureType, "BSS_COMMA_LEN_IND", sizeof(fileStructureType) - 1);
        
        // Index file name (can be overridden with setter)
        std::memset(indexFileName, 0, sizeof(indexFileName));
        std::strncpy(indexFileName, "data/zip_bss.idx", sizeof(indexFileName) - 1);

        // Initialize field schema arrays
        std::memset(fieldNames,   0, sizeof(fieldNames));
        std::memset(fieldTypes,   0, sizeof(fieldTypes));
        std::memset(fieldFormats, 0, sizeof(fieldFormats));

        setFieldSchema(0, "ZipCode",   "CHAR",   "TEXT");
        setFieldSchema(1, "PlaceName", "CHAR",   "TEXT");
        setFieldSchema(2, "State",     "CHAR",   "TEXT");
        setFieldSchema(3, "County",    "CHAR",   "TEXT");
        setFieldSchema(4, "Latitude",  "DOUBLE", "NUMERIC");
        setFieldSchema(5, "Longitude", "DOUBLE", "NUMERIC");
    }

    // Writes the header object directly to the start of the file
    bool write(std::fstream& file) const {
        file.seekp(0, std::ios::beg);
        // Only write up to blockSize to avoid overwriting block 1
        size_t writeSize = (sizeof(BSSFileHeader) < blockSize) ? sizeof(BSSFileHeader) : blockSize;
        file.write(reinterpret_cast<const char*>(this), writeSize);
        
        // If header is smaller than block, pad with zeros
        if (writeSize < blockSize) {
            std::vector<char> padding(blockSize - writeSize, 0);
            file.write(padding.data(), padding.size());
        }
        return file.good();
    }

    // Reads the header object directly from the start of the file
    bool read(std::fstream& file) {
        file.seekg(0, std::ios::beg);
        // Only read the actual struct size, not the full block
        file.read(reinterpret_cast<char*>(this), sizeof(BSSFileHeader));
        return file.good();
    }

    // --- Accessors and Mutators ---
    uint32_t getBlockSize() const { return blockSize; }
    uint32_t getRecordCount() const { return recordCount; }
    uint32_t getBlockCount() const { return blockCount; }
    int getListHeadRBN() const { return listHeadRBN; }
    int getAvailHeadRBN() const { return availHeadRBN; }

    void setRecordCount(uint32_t count) { recordCount = count; }
    void setBlockCount(uint32_t count) { blockCount = count; }
    void setListHeadRBN(int rbn) { listHeadRBN = rbn; }
    void setAvailHeadRBN(int rbn) { availHeadRBN = rbn; }

//--- header-architecture info ---

// Number of bytes for the record size integer
    uint32_t getRecordSizeFieldBytes() const { return recordSizeFieldBytes; }
    void setRecordSizeFieldBytes(uint32_t v) { recordSizeFieldBytes = v; }

    // 'A' = ASCII length, 'B' = binary length
    char getSizeFormatType() const { return sizeFormatType; }
    void setSizeFormatType(char t) { sizeFormatType = t; }

    // Index file name
    std::string getIndexFileName() const {
        return std::string(indexFileName);
    }
    void setIndexFileName(const std::string& name) {
        std::memset(indexFileName, 0, sizeof(indexFileName));
        std::strncpy(indexFileName, name.c_str(), sizeof(indexFileName) - 1);
    }

    // Field schema info
    uint16_t getFieldCount() const { return fieldCount; }
    void     setFieldCount(uint16_t c) { fieldCount = c; }

    std::string getFieldName(uint16_t i) const {
        if (i >= fieldCount) return "";
        return std::string(fieldNames[i]);
    }
    std::string getFieldType(uint16_t i) const {
        if (i >= fieldCount) return "";
        return std::string(fieldTypes[i]);
    }
    std::string getFieldFormat(uint16_t i) const {
        if (i >= fieldCount) return "";
        return std::string(fieldFormats[i]);
    }

    // Primary key field identification
    uint16_t getPrimaryKeyFieldIndex() const { return primaryKeyFieldIndex; }
    void     setPrimaryKeyFieldIndex(uint16_t idx) { primaryKeyFieldIndex = idx; }

private:
    // Helper for setting field schemas
    void setFieldSchema(uint16_t i,
                        const char* name,
                        const char* type,
                        const char* format)
    {
        if (i >= 6) return; // fixed 6 fields for this project
        std::strncpy(fieldNames[i],   name,   sizeof(fieldNames[i])   - 1);
        std::strncpy(fieldTypes[i],   type,   sizeof(fieldTypes[i])   - 1);
        std::strncpy(fieldFormats[i], format, sizeof(fieldFormats[i]) - 1);
    }
    
    // Fields from "Header Record Architecture"
    char fileStructureType[32]; // e.g., "BSS_COMMA_LEN_IND"
    uint32_t version;
    uint32_t headerRecordSize;  // Size of this struct
    uint32_t blockSize;         // Default 512
    uint32_t minBlockCapacity;  // Default 50%
    
    uint32_t recordCount;       // Total records in the file
    uint32_t blockCount;        // Total blocks (active + avail)
    
    int listHeadRBN;            // RBN of the first block in the active sequence
    int availHeadRBN;           // RBN of the first block in the avail list
    bool stale;                 // Flag for index validity (not used yet)

    // Additional fields for header-architecture
    uint32_t recordSizeFieldBytes;   // #bytes in record size int
    char     sizeFormatType;         // 'A' or 'B'

    char     indexFileName[64];      // index file name/path

    uint16_t fieldCount;             // how many fields in each record
    uint16_t primaryKeyFieldIndex;   // which field is PK (0 = ZipCode)

    // Simple fixed-size schema arrays
    char fieldNames[6][32];          // names of fields
    char fieldTypes[6][16];          // "CHAR", "DOUBLE", etc.
    char fieldFormats[6][16];        // "TEXT", "NUMERIC", etc.
};

#endif // BSSFILEHEADER_H