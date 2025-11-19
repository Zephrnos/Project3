#include <map>
#include <iomanip>
#include <string>
#include <iostream>
#include <limits> // For numeric_limits
#include <sstream>
#include <fstream>
#include <vector>
#include "ZipCodeRecordBuffer.h"
#include "HeaderBuffer.h"
#include "convertCSV.h"
#include "IndexManager.h"
#include "BSSFile.h"
#include "BSSBlock.h"
#include "BSSIndex.h"

using namespace std;

// Struct to hold the four extreme zip codes for each state
struct StateRecord {
    string easternmost_zip;
    double easternmost_lon = -numeric_limits<double>::max();
    string westernmost_zip;
    double westernmost_lon = numeric_limits<double>::max();
    string northernmost_zip;
    double northernmost_lat = -numeric_limits<double>::max();
    string southernmost_zip;
    double southernmost_lat = numeric_limits<double>::max();
};

 const string binaryFile = "Data/newBinaryPCodes.dat";
 const string indexFile = "Data/zip.idx";

//  void getP2File() {
//  ifstream testBin(binaryFile, ios::binary);
//  if (!testBin.good()) {
//      cout << "Binary or index missing — rebuilding from CSV...\n";
//      binaryToCSV(); // creates zip_len.dat and zip.idx
//  }
//  testBin.close();
//  }

/**
 * @brief Creates a blocked sequence set file from the Project 2.0 binary file
 */

void createBSSFile(const string& binaryFile, const string& bssFile) {
    cout << "\n=== Creating Blocked Sequence Set File ===\n";
    BSSFile file;
    if (!file.create(bssFile, binaryFile)) {
        cerr << "Error: Failed to create BSS file.\n";
        return;
    }
    file.close();
    cout << "BSS file '" << bssFile << "' created successfully.\n";
}

/**
 * @brief Searches for zip codes in the BSS file using block buffer and record buffer
 */
void searchBSSFile(const string& bssFile, const vector<string>& zipCodes) {
    cout << "\n=== Searching BSS File ===\n";
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file '" << bssFile << "'.\n";
        return;
    }

    // Get file header information
    const BSSFileHeader& header = file.getHeader();
    cout << "Block Size: " << header.getBlockSize() << " bytes\n";
    cout << "Total Blocks: " << header.getBlockCount() << "\n";
    cout << "Total Records: " << header.getRecordCount() << "\n\n";

    // Search for each zip code
    for (const auto& zip : zipCodes) {
        cout << "Searching for ZIP: " << zip << "\n";
        bool found = false;

        // Sequential search through blocks (can be optimized with index)
        int rbn = header.getListHeadRBN();
        while (rbn != -1 && !found) {
            // Step 1: Block buffer reads the block
            BSSBlock block(header.getBlockSize());
            if (!file.readBlock(rbn, block)) {
                cerr << "Error reading block " << rbn << "\n";
                break;
            }

            // Step 2: Block buffer unpacks records into record buffers
            vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();

            // Step 3: Record buffer provides field access
            for (const auto& rec : records) {
                if (rec.getZipCode() == zip) {
                    cout << "  Found in Block " << rbn << ": ";
                    rec.print();
                    found = true;
                    break;
                }
            }

            // Move to next block in sequence
            rbn = block.getHeader()->successorRBN;
        }

        if (!found) {
            cout << "  ZIP code " << zip << " not found.\n";
        }
        cout << "\n";
    }

    file.close();
}

/**
 * @brief Displays all records from BSS file using buffer class hierarchy
 */
void displayAllRecords(const string& bssFile) {
    cout << "\n=== Displaying All Records from BSS File ===\n";
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file.\n";
        return;
    }

    const BSSFileHeader& header = file.getHeader();
    int rbn = header.getListHeadRBN();
    int recordCount = 0;

    while (rbn != -1) {
        // Block buffer reads block
        BSSBlock block(header.getBlockSize());
        if (!file.readBlock(rbn, block)) break;

        // Block buffer unpacks to record buffers
        vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();

        // Display records using record buffer accessors
        cout << "\n--- Block " << rbn << " (" << records.size() << " records) ---\n";
        for (const auto& rec : records) {
            rec.print();
            recordCount++;
        }

        rbn = block.getHeader()->successorRBN;
    }

    cout << "\nTotal records displayed: " << recordCount << "\n";
    file.close();
}

/**
 * @brief Finds extreme zip codes by state using BSS file
 */
void findExtremeZipCodes(const string& bssFile) {
    cout << "\n=== Finding Extreme Zip Codes by State ===\n";
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file.\n";
        return;
    }

    map<string, StateRecord> stateMap;
    const BSSFileHeader& header = file.getHeader();
    int rbn = header.getListHeadRBN();

    // Process all blocks sequentially
    while (rbn != -1) {
        BSSBlock block(header.getBlockSize());
        if (!file.readBlock(rbn, block)) break;

        // Unpack records from block buffer to record buffers
        vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();

        // Process each record
        for (const auto& rec : records) {
            string state = rec.getState();
            string zip = rec.getZipCode();
            double lat = rec.getLatitude();
            double lon = rec.getLongitude();

            StateRecord& sr = stateMap[state];

            // Update extremes
            if (lon > sr.easternmost_lon) {
                sr.easternmost_lon = lon;
                sr.easternmost_zip = zip;
            }
            if (lon < sr.westernmost_lon) {
                sr.westernmost_lon = lon;
                sr.westernmost_zip = zip;
            }
            if (lat > sr.northernmost_lat) {
                sr.northernmost_lat = lat;
                sr.northernmost_zip = zip;
            }
            if (lat < sr.southernmost_lat) {
                sr.southernmost_lat = lat;
                sr.southernmost_zip = zip;
            }
        }

        rbn = block.getHeader()->successorRBN;
    }

    file.close();

    // Display results
    cout << "\nExtreme Zip Codes by State:\n";
    cout << string(80, '-') << "\n";
    for (const auto& entry : stateMap) {
        cout << "State: " << entry.first << "\n";
        cout << "  Easternmost:  " << entry.second.easternmost_zip
             << " (Lon: " << entry.second.easternmost_lon << ")\n";
        cout << "  Westernmost:  " << entry.second.westernmost_zip
             << " (Lon: " << entry.second.westernmost_lon << ")\n";
        cout << "  Northernmost: " << entry.second.northernmost_zip
             << " (Lat: " << entry.second.northernmost_lat << ")\n";
        cout << "  Southernmost: " << entry.second.southernmost_zip
             << " (Lat: " << entry.second.southernmost_lat << ")\n\n";
    }
}

/**
 * @brief Searches for zip codes using index-based lookup
 */
void searchWithIndex(const string& bssFile, const string& indexFile, const vector<string>& zipCodes) {
    cout << "\n=== Index-Based Zip Code Search ===\n";
    
    // Open BSS file
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file '" << bssFile << "'.\n";
        return;
    }

    // Load index from file (or build if doesn't exist)
    BSSIndex index;
    ifstream testIdx(indexFile, ios::binary);
    if (testIdx.good()) {
        testIdx.close();
        cout << "Loading existing index from '" << indexFile << "'...\n";
        if (!index.read(indexFile)) {
            cerr << "Error reading index. Rebuilding...\n";
            index.build(file);
            index.write(indexFile);
        }
    } else {
        testIdx.close();
        cout << "Index not found. Building new index...\n";
        index.build(file);
        index.write(indexFile);
        cout << "Index saved to '" << indexFile << "'.\n";
    }

    // Search for each zip code using the index
    for (const auto& zip : zipCodes) {
        cout << "\nSearching for ZIP: " << zip << "\n";
        
        // Step 1: Use index to find the block
        int rbn = index.findRBN(zip);
        if (rbn == -1) {
            cout << "  ZIP code " << zip << " not found (no matching block in index).\n";
            continue;
        }

        cout << "  Index indicates block RBN: " << rbn << "\n";

        // Step 2: Read the indexed block
        BSSBlock block(file.getHeader().getBlockSize());
        if (!file.readBlock(rbn, block)) {
            cerr << "  Error reading block " << rbn << "\n";
            continue;
        }

        // Step 3: Unpack records from the block
        vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();
        cout << "  Block contains " << records.size() << " records.\n";

        // Step 4: Search within the block
        bool found = false;
        for (const auto& rec : records) {
            if (rec.getZipCode() == zip) {
                cout << "  ✓ Found: ";
                rec.print();
                found = true;
                break;
            }
        }

        // Step 5: Confirm if not found after searching the block
        if (!found) {
            cout << "  ✗ ZIP code " << zip << " not found in block " << rbn
                 << " (block was read, unpacked, and searched).\n";
        }
    }

    file.close();
}

/**
 * @brief Test demonstration: Search for valid and invalid zip codes
 */
void runSearchTest(const string& bssFile, const string& indexFile) {
    cout << "\n" << string(80, '=') << "\n";
    cout << "SEARCH TEST DEMONSTRATION\n";
    cout << string(80, '=') << "\n\n";
    
    // Test zip codes: mix of valid and invalid
    vector<string> testZips = {
        "10001",  // Valid - New York, NY
        "90210",  // Valid - Beverly Hills, CA
        "60601",  // Valid - Chicago, IL
        "33139",  // Valid - Miami Beach, FL
        "98101",  // Valid - Seattle, WA
        "00000",  // Invalid - doesn't exist
        "99999",  // Invalid - doesn't exist
        "12345"   // May or may not exist
    };
    
    cout << "Testing " << testZips.size() << " zip codes (including invalid ones)...\n";
    cout << string(80, '-') << "\n";
    
    // Open BSS file
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file.\n";
        return;
    }

    // Load index into RAM (never load entire BSS file into RAM)
    BSSIndex index;
    cout << "\n[Loading index into RAM...]\n";
    if (!index.read(indexFile)) {
        cout << "[Index not found - building new index...]\n";
        index.build(file);
        index.write(indexFile);
        cout << "[Index saved to '" << indexFile << "']\n";
    }
    cout << "[Index loaded successfully - BSS file remains on disk]\n\n";

    // Search for each zip code
    int validCount = 0;
    int invalidCount = 0;
    
    for (const auto& zip : testZips) {
        cout << "\n[Test " << (validCount + invalidCount + 1) << "] Searching for ZIP: " << zip << "\n";
        
        // Step 1: Use index to find the block (index is in RAM)
        int rbn = index.findRBN(zip);
        if (rbn == -1) {
            cout << "  ✗ INVALID: ZIP code " << zip << " not found (no matching block in index)\n";
            invalidCount++;
            continue;
        }

        cout << "  → Index lookup: Block RBN " << rbn << "\n";

        // Step 2: Read ONLY the indexed block (not entire file)
        BSSBlock block(file.getHeader().getBlockSize());
        if (!file.readBlock(rbn, block)) {
            cerr << "  ✗ ERROR: Could not read block " << rbn << "\n";
            invalidCount++;
            continue;
        }

        // Step 3: Unpack records from the block
        vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();
        cout << "  → Block contains " << records.size() << " records\n";

        // Step 4: Search within the block
        bool found = false;
        for (const auto& rec : records) {
            if (rec.getZipCode() == zip) {
                cout << "  ✓ VALID: Found in block " << rbn << "\n";
                cout << "     ";
                rec.print();
                found = true;
                validCount++;
                break;
            }
        }

        // Step 5: Confirm not found after searching the block
        if (!found) {
            cout << "  ✗ INVALID: ZIP code " << zip << " not found in block " << rbn << "\n";
            cout << "     (Block was read, unpacked, and searched - record does not exist)\n";
            invalidCount++;
        }
    }

    file.close();
    
    // Summary
    cout << "\n" << string(80, '=') << "\n";
    cout << "SEARCH TEST SUMMARY\n";
    cout << string(80, '=') << "\n";
    cout << "Total searches: " << testZips.size() << "\n";
    cout << "Valid zip codes found: " << validCount << "\n";
    cout << "Invalid zip codes: " << invalidCount << "\n";
    cout << "\n✓ Index was loaded into RAM\n";
    cout << "✓ BSS file was NEVER fully loaded into RAM\n";
    cout << "✓ Only individual blocks were read as needed\n";
    cout << string(80, '=') << "\n\n";
}

/**
 * @brief Interactive zip code lookup with continuous prompting
 */
void interactiveSearch(const string& bssFile, const string& indexFile) {
    cout << "\n" << string(80, '=') << "\n";
    cout << "INTERACTIVE ZIP CODE LOOKUP\n";
    cout << string(80, '=') << "\n\n";
    
    // Open BSS file
    BSSFile file;
    if (!file.open(bssFile)) {
        cerr << "Error: Could not open BSS file '" << bssFile << "'.\n";
        return;
    }

    cout << "BSS File: " << bssFile << "\n";
    cout << "Block Size: " << file.getHeader().getBlockSize() << " bytes\n";
    cout << "Total Blocks: " << file.getHeader().getBlockCount() << "\n";
    cout << "Total Records: " << file.getHeader().getRecordCount() << "\n\n";

    // Load or build index
    BSSIndex index;
    ifstream testIdx(indexFile, ios::binary);
    if (testIdx.good()) {
        testIdx.close();
        cout << "Loading existing index from '" << indexFile << "'...\n";
        if (!index.read(indexFile)) {
            cout << "Error reading index. Building new index...\n";
            index.build(file);
            index.write(indexFile);
            cout << "Index saved to '" << indexFile << "'.\n";
        }
    } else {
        testIdx.close();
        cout << "Index not found. Building new index...\n";
        index.build(file);
        index.write(indexFile);
        cout << "Index saved to '" << indexFile << "'.\n";
    }
    cout << "Index ready for searching.\n\n";

    // Interactive loop
    string zipCode;
    int searchCount = 0;
    
    cout << "Enter zip codes to search (type 'quit', 'q', or 'exit' to stop)\n";
    cout << string(80, '-') << "\n";
    
    while (true) {
        cout << "\nEnter zip code: ";
        cin >> zipCode;
        
        // Check for quit commands
        if (zipCode == "quit" || zipCode == "q" || zipCode == "exit") {
            cout << "\n" << string(80, '-') << "\n";
            cout << "Total searches performed: " << searchCount << "\n";
            cout << "Exiting interactive mode.\n";
            break;
        }

        searchCount++;
        
        // Search for the zip code
        cout << "\n  Searching for ZIP: " << zipCode << "\n";
        
        // Step 1: Use index to find the block
        int rbn = index.findRBN(zipCode);
        if (rbn == -1) {
            cout << "  ✗ ZIP code " << zipCode << " not found (no matching block in index).\n";
            continue;
        }

        cout << "  → Index indicates block RBN: " << rbn << "\n";

        // Step 2: Read the indexed block
        BSSBlock block(file.getHeader().getBlockSize());
        if (!file.readBlock(rbn, block)) {
            cerr << "  ✗ Error reading block " << rbn << "\n";
            continue;
        }

        // Step 3: Unpack records from the block
        vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();
        cout << "  → Block contains " << records.size() << " records\n";

        // Step 4: Search within the block
        bool found = false;
        for (const auto& rec : records) {
            if (rec.getZipCode() == zipCode) {
                cout << "  ✓ FOUND:\n    ";
                rec.print();
                found = true;
                break;
            }
        }

        // Step 5: Confirm if not found after searching the block
        if (!found) {
            cout << "  ✗ ZIP code " << zipCode << " not found in block " << rbn << "\n";
            cout << "    (block was read, unpacked, and searched)\n";
        }
    }

    file.close();
}

void printUsage(const char* programName) {
    cout << "\nUsage:\n";
    cout << "  " << programName << " -i | --interactive\n";
    cout << "      Start interactive zip code lookup mode\n\n";
    cout << "  " << programName << " --test\n";
    cout << "      Run search test demonstration with valid and invalid zip codes\n\n";
    cout << "  " << programName << " <bss_file> -Z<zip1> [-Z<zip2> ...]\n";
    cout << "      Search for specific zip codes\n\n";
    cout << "  " << programName << "\n";
    cout << "      Run in demo mode with predefined searches\n\n";
    cout << "Arguments:\n";
    cout << "  -i, --interactive  Start interactive mode\n";
    cout << "  --test             Run search test demonstration\n";
    cout << "  <bss_file>         Path to the blocked sequence set file\n";
    cout << "  -Z<zipcode>        Zip code to search for (e.g., -Z10001)\n\n";
    cout << "Examples:\n";
    cout << "  " << programName << " -i\n";
    cout << "  " << programName << " --test\n";
    cout << "  " << programName << " Data/zipCodes.bss -Z10001\n";
    cout << "  " << programName << " Data/zipCodes.bss -Z10001 -Z90210 -Z60601\n\n";
}

int main(int argc, char* argv[]) {
    const string defaultBinaryFile = "Data/newBinaryPCodes.dat";
    const string defaultBssFile = "Data/zipCodes.bss";
    const string defaultBssIndexFile = "Data/zipCodes.bss.idx";

    // Check for test mode flag
    if (argc == 2 && string(argv[1]) == "--test") {
        cout << "=== SEARCH TEST MODE ===\n\n";
        
        // Ensure binary file exists
        ifstream testBin(defaultBinaryFile, ios::binary);
        if (!testBin.good()) {
            cout << "Binary file missing — rebuilding from CSV...\n";
            binaryToCSV();
        }
        testBin.close();

        // Create BSS file if needed
        ifstream testBSS(defaultBssFile, ios::binary);
        if (!testBSS.good()) {
            cout << "BSS file missing — creating from binary file...\n";
            createBSSFile(defaultBinaryFile, defaultBssFile);
        }
        testBSS.close();

        // Run search test demonstration
        runSearchTest(defaultBssFile, defaultBssIndexFile);
        return 0;
    }

    // Check for interactive mode flag
    if (argc == 2 && (string(argv[1]) == "-i" || string(argv[1]) == "--interactive")) {
        cout << "=== INTERACTIVE MODE ===\n\n";
        
        // Ensure binary file exists
        ifstream testBin(defaultBinaryFile, ios::binary);
        if (!testBin.good()) {
            cout << "Binary file missing — rebuilding from CSV...\n";
            binaryToCSV();
        }
        testBin.close();

        // Create BSS file if needed
        ifstream testBSS(defaultBssFile, ios::binary);
        if (!testBSS.good()) {
            cout << "BSS file missing — creating from binary file...\n";
            createBSSFile(defaultBinaryFile, defaultBssFile);
        }
        testBSS.close();

        // Start interactive search
        interactiveSearch(defaultBssFile, defaultBssIndexFile);
        return 0;
    }

    // Check for command-line arguments
    if (argc < 2) {
        // No arguments - run in demo mode with interactive at the end
        cout << "=== Running in DEMO MODE (no command-line arguments) ===\n";
        
        // Ensure binary file exists
        ifstream testBin(defaultBinaryFile, ios::binary);
        if (!testBin.good()) {
            cout << "Binary file missing — rebuilding from CSV...\n";
            binaryToCSV();
        }
        testBin.close();

        // Create BSS file if needed
        ifstream testBSS(defaultBssFile, ios::binary);
        if (!testBSS.good()) {
            cout << "BSS file missing — creating from binary file...\n";
            createBSSFile(defaultBinaryFile, defaultBssFile);
        }
        testBSS.close();

        // Demonstrate BSS operations
        cout << "\n" << string(80, '=') << "\n";
        cout << "BLOCKED SEQUENCE SET FILE OPERATIONS\n";
        cout << string(80, '=') << "\n";

        BSSFile file;
        if (file.open(defaultBssFile)) {
            file.dumpLogical(cout);
            file.close();
        }

        // Demo search with index
        vector<string> demoZips = {"10001", "90210", "60601", "33139", "98101"};
        searchWithIndex(defaultBssFile, defaultBssIndexFile, demoZips);

        // Find extreme zip codes
        findExtremeZipCodes(defaultBssFile);

        cout << "\n" << string(80, '=') << "\n";
        cout << "DEMO COMPLETE - Starting Interactive Mode\n";
        cout << string(80, '=') << "\n";
        
        // Automatically start interactive mode after demo
        interactiveSearch(defaultBssFile, defaultBssIndexFile);
        
        return 0;
    }

    // Command-line mode: Parse arguments
    string bssFile = argv[1];
    vector<string> zipCodes;

    // Parse -Z flags
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg.length() > 2 && arg.substr(0, 2) == "-Z") {
            zipCodes.push_back(arg.substr(2));
        } else {
            cerr << "Warning: Ignoring invalid argument '" << arg << "'\n";
        }
    }

    // Validate arguments
    if (zipCodes.empty()) {
        cerr << "Error: No zip codes specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    // Check if BSS file exists
    ifstream testBSS(bssFile, ios::binary);
    if (!testBSS.good()) {
        cerr << "Error: BSS file '" << bssFile << "' not found.\n";
        cerr << "Please create the BSS file first or run without arguments for demo mode.\n";
        return 1;
    }
    testBSS.close();

    // Generate index file name
    string indexFile = bssFile + ".idx";

    // Perform index-based search
    searchWithIndex(bssFile, indexFile, zipCodes);

    return 0;
}