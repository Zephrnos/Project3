#include <iostream>
#include <string>
#include <vector>
#include "../headers/BSSFile.h"
#include "../headers/BSSIndex.h"
#include "../headers/ZipCodeRecordBuffer.h"

// Note: I've updated the include paths to match your folder structure
// (e.g., "../headers/BSSFile.h")

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  main_p3 --create <proj2_dat_file> <bss_file> <index_file>\n";
    std::cout << "          (Creates a new BSS file and its index from a Project 2.0 .dat file)\n\n";
    std::cout << "  main_p3 --search <bss_file> <index_file> -Z<zip1> [-Z<zip2> ...]\n";
    std::cout << "          (Searches for one or more Zip Codes)\n\n";
    std::cout << "  main_p3 --dump-logical <bss_file>\n";
    std::cout << "          (Dumps blocks by following sequence set links)\n\n";
    std::cout << "  main_p3 --dump-physical <bss_file>\n";
    std::cout << "          (Dumps all blocks in physical file order)\n\n";
    std::cout << "  main_p3 --dump-index <index_file>\n";
    std::cout << "          (Dumps the contents of the index file)\n\n";
}

/**
 * @brief Handles the --create command.
 */
int doCreate(const std::string& proj2DatFile, const std::string& bssFile, const std::string& indexFile) {
    std::cout << "Creating BSS file '" << bssFile << "' from '" << proj2DatFile << "'...\n";
    BSSFile file;
    if (!file.create(bssFile, proj2DatFile)) {
        std::cerr << "Error: File creation failed.\n";
        return 1;
    }
    file.close();
    std::cout << "BSS file created successfully.\n";

    // Re-open file to build index
    std::cout << "Building index '" << indexFile << "'...\n";
    if (!file.open(bssFile)) {
        std::cerr << "Error: Could not re-open BSS file to build index.\n";
        return 1;
    }

    BSSIndex index;
    index.build(file);
    if (!index.write(indexFile)) {
        std::cerr << "Error: Could not write index file.\n";
        return 1;
    }
    
    file.close();
    std::cout << "Index built and saved successfully.\n";
    return 0;
}

/**
 * @brief Handles the --search command (Task 11).
 */
int doSearch(const std::string& bssFile, const std::string& indexFile, const std::vector<std::string>& zips) {
    BSSIndex index;
    std::cout << "Loading index '" << indexFile << "'...\n";
    if (!index.read(indexFile)) {
        std::cerr << "Error: Could not read index file.\n";
        return 1;
    }

    BSSFile file;
    if (!file.open(bssFile)) {
        std::cerr << "Error: Could not open BSS file '" << bssFile << "'.\n";
        return 1;
    }

    std::cout << "\n--- ZIP Code Search Results ---\n";
    for (const auto& zip : zips) {
        int rbn = index.findRBN(zip);
        if (rbn == -1) {
            std::cout << "ZIP code " << zip << " not found (no matching block).\n";
            continue;
        }

        // Read the block
        BSSBlock block(file.getHeader().getBlockSize());
        if (!file.readBlock(rbn, block)) {
            std::cerr << "Error reading block " << rbn << " for ZIP " << zip << ".\n";
            continue;
        }

        // Unpack all records from this block and search
        std::vector<ZipCodeRecordBuffer> records = block.unpackAllRecords();
        bool found = false;
        for (const auto& rec : records) {
            if (rec.getZipCode() == zip) {
                rec.print();
                found = true;
                break;
            }
        }

        if (!found) {
            std::cout << "ZIP code " << zip << " not found (in block " << rbn << ").\n";
        }
    }

    file.close();
    return 0;
}

/**
 * @brief Main driver for Project 3.
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    
    try {
        if (command == "--create" && argc == 5) {
            return doCreate(argv[2], argv[3], argv[4]);
        } 
        else if (command == "--search" && argc >= 5) {
            std::string bssFile = argv[2];
            std::string indexFile = argv[3];
            std::vector<std::string> zips;
            for (int i = 4; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg.rfind("-Z", 0) == 0 && arg.size() > 2) {
                    zips.push_back(arg.substr(2));
                }
            }
            if (zips.empty()) {
                std::cerr << "Error: No -Z<zip> flags provided for search.\n";
                printUsage();
                return 1;
            }
            return doSearch(bssFile, indexFile, zips);
        }
        else if (command == "--dump-logical" && argc == 3) {
            BSSFile file;
            if (!file.open(argv[2])) { std::cerr << "Error opening file.\n"; return 1; }
            file.dumpLogical(std::cout);
            file.close();
            return 0;
        }
        else if (command == "--dump-physical" && argc == 3) {
            BSSFile file;
            if (!file.open(argv[2])) { std::cerr << "Error opening file.\n"; return 1; }
            file.dumpPhysical(std::cout);
            file.close();
            return 0;
        }
        else if (command == "--dump-index" && argc == 3) {
            BSSIndex index;
            if (!index.read(argv[2])) { std::cerr << "Error reading index.\n"; return 1; }
            index.dump(std::cout);
            return 0;
        }
        else {
            std::cerr << "Error: Invalid command or wrong number of arguments.\n";
            printUsage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
}