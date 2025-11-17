#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../headers/convertCSV.h"
#include "../headers/readBinaryFile.h"
#include "../headers/HeaderBuffer.h"
#include "../headers/IndexManager.h"
#include <algorithm>
#include <cctype>
using namespace std;

//Authors: Team 5
//Date: 2025-10-11
/*Purpose: This file contains the implementation of functions to read a CSV file and
 write its contents to a binary file with length-prefixed records. */

void processFile(string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName, ios::binary);

    if (!inputFile.is_open()) {
        cerr << "Error opening file " << inputFileName << endl;
    }
    else if (!outputFile.is_open()) {
        cerr << "Error opening file " << outputFileName << endl;
        return;
    }

    HeaderRecordBuffer header;
    header.setVersion(1); // Or appropriate version

    // Calculate number of records (lines) in the input file. We assume each CSV line
    // corresponds to one record. Skip the header line if present.
    streampos originalPos = inputFile.tellg();
    uint32_t recordCount = 0;
    string tempLine;
    bool first = true;

    // Count lines
    while (std::getline(inputFile, tempLine)) {
        if (first) { // Skip header line
            first = false;
            continue;
        }
        
        if (!tempLine.empty()) {
            ++recordCount;
        }
    }

    // Reset stream to beginning to write records after header
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    // If the CSV has a header row, consume it and adjust the count.
    // Here we attempt to be conservative: if the first non-empty line contains
    // alphabetic characters and commas, assume it's a header and subtract 1.
    streampos posAfterPeek = inputFile.tellg();
    if (std::getline(inputFile, tempLine)) {
        bool looksLikeHeader = (tempLine.find(',') != std::string::npos &&
                               std::any_of(tempLine.begin(), tempLine.end(), ::isalpha));
        if (looksLikeHeader && recordCount > 0) {
            --recordCount;
        }
    }

    // Reset to beginning again so subsequent record reads start from the first data line
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    header.setRecordCount(recordCount);
    header.setCreationDate("2025-10-14"); // Use desired date format
    header.writeHeader(outputFile);

    string line;

    while (getline(inputFile, line)) {
        // Call your length-prefixed writing function correctly
        lenRead(outputFile, line);
    }

    IndexManager index;
    index.buildIndex(outputFileName); // Scan binary file for ZIP → offset map
    index.writeIndex("../data/zip.idx"); // Save index to Data folder

    inputFile.close();
    outputFile.close();
}

// Consider renaming lenRead to something more descriptive!
void lenRead(ofstream& output, const string& record) {
    uint32_t recordLength = static_cast<uint32_t>(record.length());
    output.write(reinterpret_cast<const char*>(&recordLength), sizeof(recordLength)); // length prefix (uint32_t)
    output.write(record.c_str(), recordLength);
}

void binaryToCSV() {
    string inputCSVFileName = "../data/us_postal_codes.csv";
    string binaryFile = "../data/newBinaryPCodes.dat";
	string outputCSVFile = "../data/converted_postal_codes.csv";

    processFile(inputCSVFileName, binaryFile);
	readBinaryFile(binaryFile, outputCSVFile);

}
