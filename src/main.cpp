#include <map>
#include <iomanip>
#include <string>
#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>
#include "ZipCodeRecordBuffer.h"
#include "HeaderBuffer.h"
#include "convertCSV.h"
#include "IndexManager.h"
#include "BlockBuffer.h"

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

int main(int argc, char* argv[]) {
    // --- Step 1: Ensure binary and index exist ---
    const string binaryFile = "data/newBinaryPCodes.dat";
    const string indexFile = "data/zip.idx";

    ifstream testBin(binaryFile, ios::binary);
    if (!testBin.good()) {
        cout << "Binary or index missing — rebuilding from CSV...\n";
        binaryToCSV(); // creates zip_len.dat and zip.idx
    }
    testBin.close();

    // --- Part 1: Compute state extremes (from converted CSV) ---
    map<string, StateRecord> all_states;
    ZipCodeRecordBuffer buffer;
    ifstream file("data/converted_postal_codes.csv");

    if (!file.is_open()) {
        cerr << "Error opening data/converted_postal_codes.csv" << endl;
        return 1;
    }

    string header;
    getline(file, header);

    while (buffer.ReadRecord(file)) {
        string state = buffer.getState();
        string zip = buffer.getZipCode();
        double latitude = buffer.getLatitude();
        double longitude = buffer.getLongitude();

        if (all_states.find(state) == all_states.end()) {
            all_states[state] = StateRecord{};
        }

        StateRecord& record = all_states[state];
        if (longitude > record.easternmost_lon) { record.easternmost_lon = longitude; record.easternmost_zip = zip; }
        if (longitude < record.westernmost_lon) { record.westernmost_lon = longitude; record.westernmost_zip = zip; }
        if (latitude > record.northernmost_lat) { record.northernmost_lat = latitude; record.northernmost_zip = zip; }
        if (latitude < record.southernmost_lat) { record.southernmost_lat = latitude; record.southernmost_zip = zip; }
    }
    file.close();
// Extreme headers for zipcode project 1
    // Print state extremes summary
    cout << left << setw(8) << "State"
         << setw(15) << "Easternmost"
         << setw(15) << "Westernmost"
         << setw(15) << "Northernmost"
         << setw(15) << "Southernmost"
         << "\n";
    cout << string(68, '-') << "\n";

    for (const auto& pair : all_states) {
        const auto& record = pair.second;
        cout << left << setw(8) << pair.first
             << setw(15) << record.easternmost_zip
             << setw(15) << record.westernmost_zip
             << setw(15) << record.northernmost_zip
             << setw(15) << record.southernmost_zip
             << "\n";
    }

    // --- Part 2: Load index and handle ZIP code flags ---
    IndexManager index;
    index.readIndex("data/zip.idx");

    cout << "\n--- ZIP Code Search Results ---\n";

    bool foundAny = false;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        // Look for flags that start with "-Z"
        if (arg.rfind("-Z", 0) == 0 && arg.size() > 2) {
            string zipInput = arg.substr(2); // Get the ZIP after "-Z"
            foundAny = true;

            uint64_t offset = index.findOffset(zipInput);
            if (offset == UINT64_MAX) {
                cout << "ZIP code " << zipInput << " not found.\n";
                continue;
            }

            ifstream binFile("data/newBinaryPCodes.dat", ios::binary);
            if (!binFile.is_open()) {
                cerr << "Error opening binary data file.\n";
                return 1;
            }

            binFile.seekg(offset, ios::beg);

            uint32_t recordLength;
            binFile.read(reinterpret_cast<char*>(&recordLength), sizeof(recordLength));

            string record(recordLength, '\0');
            binFile.read(&record[0], recordLength);

            ZipCodeRecordBuffer zipBuffer;
            istringstream ss(record);
            if (zipBuffer.ReadRecord(ss)) {
                cout << "---------------------------------------------\n";
                cout << "ZIP Code: " << zipBuffer.getZipCode() << "\n"
                     << "Place Name: " << zipBuffer.getPlaceName() << "\n"
                     << "State: " << zipBuffer.getState() << "\n"
                     << "County: " << zipBuffer.getCounty() << "\n"
                     << "Latitude: " << zipBuffer.getLatitude() << "\n"
                     << "Longitude: " << zipBuffer.getLongitude() << "\n";
            } else {
                cerr << "Error parsing record for ZIP code " << zipInput << endl;
            }

            binFile.close();
        }
    }

    if (!foundAny) {
        cout << "No ZIP codes provided. Use flags like: -Z56301 -Z90210\n";
    }


    
    //PROJECT 2 ZIP code interactive lookup
    /*
    // Interactive ZIP code lookup
    cout << "\n=== Interactive ZIP Code Lookup ===\n";
    cout << "Enter ZIP codes to search (numbers only) or enter 'q' to quit \n";
    
    string zipInput;

    while (true) {
        cout << "\nEnter ZIP code: ";
        cin >> zipInput;
        
        if (zipInput == "q" || zipInput == "Q") break;
        
        cout << "\nSearching for ZIP code " << zipInput << "... (please wait)\n";
        uint64_t offset = index.findOffset(zipInput);
        if (offset == UINT64_MAX) {
            cout << "ZIP code " << zipInput << " not found.\n";
            cout << "\n========================================\n";
            continue;
        }

        ifstream binFile("data/newBinaryPCodes.dat", ios::binary);
        if (!binFile.is_open()) {
            cerr << "Error opening binary data file.\n";
            break;
        }

        binFile.seekg(offset, ios::beg);

        uint32_t recordLength;
        binFile.read(reinterpret_cast<char*>(&recordLength), sizeof(recordLength));

        string record(recordLength, '\0');
        binFile.read(&record[0], recordLength);

        ZipCodeRecordBuffer zipBuffer;
        istringstream ss(record);
        if (zipBuffer.ReadRecord(ss)) {
            cout << "\nFound ZIP code! Details:\n";
            cout << "---------------------------------------------\n";
            cout << "ZIP Code: " << zipBuffer.getZipCode() << "\n"
                 << "Place Name: " << zipBuffer.getPlaceName() << "\n"
                 << "State: " << zipBuffer.getState() << "\n"
                 << "County: " << zipBuffer.getCounty() << "\n"
                 << "Latitude: " << zipBuffer.getLatitude() << "\n"
                 << "Longitude: " << zipBuffer.getLongitude() << "\n"
                 << "---------------------------------------------\n";
        } else {
            cerr << "Error parsing record for ZIP code " << zipInput << endl;
        }

        binFile.close();
    }
    */

    //PROJECT 3 BlockBuffer Test
    

    // Create fake CSV input for three records
    std::stringstream csvData;
    csvData << "56301,St. Cloud,MN,Stearns,45.5533,-94.1718\n";
    csvData << "55414,Minneapolis,MN,Hennepin,44.9735,-93.2272\n";
    csvData << "55044,Lakeville,MN,Dakota,44.6497,-93.2427\n";

    // Read records
    ZipCodeRecordBuffer rec1, rec2, rec3;
    rec1.ReadRecord(csvData);
    rec2.ReadRecord(csvData);
    rec3.ReadRecord(csvData);

    // Create and populate BlockBuffer
    BlockBuffer block;
    block.addRecord(rec1);
    block.addRecord(rec2);
    block.addRecord(rec3);

    // Write the block to file
    block.writeToFile("data/block_test.bin");

    // Read it back
    BlockBuffer loaded;
    loaded.readFromFile("data/block_test.bin");

    // Print results
    loaded.debugPrint(std::cout);
    

    cout << "\nProgram complete.\n";
    return 0;
}