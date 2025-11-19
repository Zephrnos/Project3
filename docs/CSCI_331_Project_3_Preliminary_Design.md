# CSCI 331 - Project 3 Preliminary Design Document

**Course:** CSCI 331  
**Project:** Zip Code Record Processor: Part 3 - Blocked Sequence Set  
**Date:** 11/04/2025  
**Team:** Team 5

---

## 1. Project Overview

The purpose of this project is to create and manage a **Blocked Sequence Set (BSS)** file structure for zip code records. This builds upon Projects 1.0 and 2.0 by implementing a sophisticated block-based storage system with indexing capabilities.

### Key Objectives:

1. Generate a blocked sequence set file from the Project 2.0 length-indicated data file
2. Implement block buffer and record buffer classes for reading/unpacking records
3. Create two dump methods: physical ordering and logical ordering
4. Build and maintain a simple primary key index for efficient searching
5. Support record addition with block splitting
6. Support record deletion with block redistribution and merging
7. Manage an avail list for reusing deleted blocks

Extensive documentation of the source code is required using Doxygen.

---

## 2. Data Description

### Input Data Source:
- **Project 2.0 File:** Length-indicated, comma-separated zip code records

### Record Fields:

| Field       | Description                     | Example      |
|-------------|---------------------------------|--------------|
| ZipCode     | 5-digit zip code (Primary Key)  | 90210        |
| PlaceName   | City name                       | Beverly Hills|
| State       | 2-character state abbreviation  | CA           |
| County      | County name                     | Los Angeles  |
| Latitude    | Decimal latitude                | 34.0901      |
| Longitude   | Decimal longitude               | -118.4065    |

---

## 3. File Structure Architecture

### 3.1 Header Record (RBN 0)

The file header occupies the first block (RBN 0) and contains:

| Component                  | Description                                    |
|----------------------------|------------------------------------------------|
| File Structure Type        | "BSS_COMMA_LEN_IND"                           |
| Version                    | Version number (1)                             |
| Header Record Size         | Size of header in bytes                        |
| Record Size Field Bytes    | Number of bytes for record length (4)          |
| Size Format Type           | 'B' = Binary, 'A' = ASCII                      |
| Block Size                 | Default: 512 bytes                             |
| Minimum Block Capacity     | Default: 50%                                   |
| Index File Name            | Path to index file                             |
| Record Count               | Total records in file                          |
| Block Count                | Total blocks (active + avail)                  |
| Field Count                | Number of fields per record (6)                |
| Field Schema               | Names, types, formats for each field           |
| Primary Key Field Index    | Which field is primary key (0 = ZipCode)       |
| List Head RBN              | RBN of first active block                      |
| Avail Head RBN             | RBN of first available block                   |
| Stale Flag                 | Indicates if index needs rebuilding            |

### 3.2 Active Block Structure

Each active block contains:

| Component           | Description                                    |
|---------------------|------------------------------------------------|
| Record Count        | Number of records in block (> 0)               |
| Predecessor RBN     | Link to previous block in sequence             |
| Successor RBN       | Link to next block in sequence                 |
| Block Type          | 'A' = Active                                   |
| Record Data         | Length-indicated, comma-separated records      |

### 3.3 Avail Block Structure

Each available (deleted) block contains:

| Component           | Description                                    |
|---------------------|------------------------------------------------|
| Record Count        | Must be 0                                      |
| Predecessor RBN     | Not used (-1)                                  |
| Successor RBN       | Link to next avail block                       |
| Block Type          | 'V' = aVail                                    |
| Padding             | Filled with blanks (spaces)                    |

### 3.4 Simple Index File Structure

The index file stores ordered pairs: `{<highest key in block>, <RBN>}`

Binary format:
```
[4 bytes: entry count]
For each entry:
  [2 bytes: key length]
  [N bytes: key string]
  [4 bytes: RBN]
```

---

## 4. Classes and Data Structures

### 4.1 Core Classes

#### [`BSSFile`](../headers/BSSFile.h)
- **Purpose:** Main orchestrator for BSS file operations
- **Key Methods:**
  - [`create()`](../src/BSSFile.cpp:16) - Creates BSS file from Project 2.0 file
  - [`open()`](../src/BSSFile.cpp:144) - Opens existing BSS file
  - [`readBlock()`](../src/BSSFile.cpp:169) - Reads a block by RBN
  - [`writeBlock()`](../src/BSSFile.cpp:175) - Writes a block by RBN
  - [`dumpPhysical()`](../src/BSSFile.cpp:183) - Dumps blocks in physical order
  - [`dumpLogical()`](../src/BSSFile.cpp:202) - Dumps blocks in logical order

#### [`BSSBlock`](../headers/BSSBlock.h)
- **Purpose:** Block buffer class for managing individual blocks
- **Key Methods:**
  - [`addRecord()`](../src/BSSBlock.cpp:52) - Adds record to block
  - [`read()`](../src/BSSBlock.cpp:87) - Reads block from file
  - [`write()`](../src/BSSBlock.cpp:153) - Writes block to file
  - [`unpackAllRecords()`](../src/BSSBlock.cpp:160) - Unpacks all records
  - [`makeAvailBlock()`](../src/BSSBlock.cpp:32) - Converts to avail block
  - [`getHighestKey()`](../headers/BSSBlock.h:64) - Returns highest key in block

#### [`BSSFileHeader`](../headers/BSSFileHeader.h)
- **Purpose:** Manages file-level metadata
- **Key Methods:**
  - [`write()`](../headers/BSSFileHeader.h:49) - Writes header to file
  - [`read()`](../headers/BSSFileHeader.h:64) - Reads header from file
  - Accessors for all header fields

#### [`BSSIndex`](../headers/BSSIndex.h)
- **Purpose:** Simple primary key index for efficient searching
- **Key Methods:**
  - [`build()`](../src/BSSIndex.cpp:11) - Builds index from BSS file
  - [`findRBN()`](../src/BSSIndex.cpp:83) - Finds block containing key
  - [`write()`](../src/BSSIndex.cpp:105) - Writes index to file
  - [`read()`](../src/BSSIndex.cpp:133) - Reads index from file
  - [`dump()`](../src/BSSIndex.cpp:167) - Displays index contents

#### [`ZipCodeRecordBuffer`](../headers/ZipCodeRecordBuffer.h)
- **Purpose:** Record buffer class for unpacking individual records
- **Key Methods:**
  - `pack()` - Packs record into string format
  - `unpack()` - Unpacks record from string format
  - Field accessors: `getZipCode()`, `getPlaceName()`, etc.

### 4.2 Data Structures

- **`std::map<std::string, int>`** - Index mapping (highest key → RBN)
- **`std::vector<ZipCodeRecordBuffer>`** - Container for unpacked records
- **`char* buffer`** - Raw byte buffer for block data

---

## 5. Program Flow and Algorithms

### 5.1 BSS File Creation

```
1. Open Project 2.0 length-indicated file
2. Read and parse all records
3. Sort records by ZipCode (primary key)
4. Initialize BSS file header (RBN 0)
5. Pack records into blocks:
   a. Create new block
   b. Add records until block is full
   c. Set predecessor/successor links
   d. Write block to file
   e. Update block count
6. Write final header with metadata
```

### 5.2 Index Building

```
1. Open BSS file and read header
2. Start at listHeadRBN
3. For each active block:
   a. Read block
   b. Get highest key from block
   c. Store mapping: highestKey → RBN
   d. Follow successor link to next block
4. Write index to binary file
```

### 5.3 Index-Based Search

```
1. Load index into RAM (std::map)
2. For each search key:
   a. Use index.findRBN(key) to locate block
   b. If RBN == -1: key not found
   c. Read only the indexed block
   d. Unpack records from block
   e. Search within block for exact match
   f. Display result or "not found"
```

### 5.4 Physical Dump

```
1. For RBN = 0 to blockCount-1:
   a. Read block at RBN
   b. Display: RBN, type, record count, prev, next, highest key
```

### 5.5 Logical Dump

```
1. Start at listHeadRBN
2. While RBN != -1:
   a. Read block at RBN
   b. Display: RBN, type, record count, prev, next, highest key
   c. RBN = block.successorRBN
```

### 5.6 Record Addition (Future Implementation)

```
1. Use index to find target block
2. Read block
3. If record fits:
   a. Insert record in sorted order
   b. Update highest key if needed
   c. Write block back
   d. Update index if highest key changed
4. If block is full (split required):
   a. Create new block (use avail list if available)
   b. Distribute records between blocks
   c. Update predecessor/successor links
   d. Write both blocks
   e. Update index with new block entry
   f. Log split event
```

### 5.7 Record Deletion (Future Implementation)

```
1. Use index to find block containing record
2. Read block and remove record
3. Check block capacity:
   a. If >= 50%: write block back
   b. If < 50%: attempt redistribution or merge
4. Redistribution:
   a. Read adjacent block
   b. Redistribute records evenly
   c. Update both blocks
   d. Update index if highest keys changed
5. Merge:
   a. Combine with adjacent block
   b. Clear rightmost block
   c. Add cleared block to avail list
   d. Update links
   e. Remove merged block from index
   f. Log merge event
```

---

## 6. Testing Plan

### 6.1 Unit Tests

| Test # | Description                          | Expected Result                    |
|--------|--------------------------------------|------------------------------------|
| 1      | Create BSS file from P2.0 file       | File created with correct blocks   |
| 2      | Read block by RBN                    | Block data matches written data    |
| 3      | Write block to file                  | Block persists correctly           |
| 4      | Unpack records from block            | All records extracted correctly    |
| 5      | Build index from BSS file            | Index contains all block mappings  |
| 6      | Search using index                   | Correct block located              |
| 7      | Physical dump                        | All blocks listed sequentially     |
| 8      | Logical dump                         | Blocks listed in key order         |

### 6.2 Integration Tests

| Test # | Description                          | Expected Result                    |
|--------|--------------------------------------|------------------------------------|
| 1      | Search for valid zip codes           | Records found and displayed        |
| 2      | Search for invalid zip codes         | "Not found" message displayed      |
| 3      | Compare physical vs logical dumps    | Initially identical                |
| 4      | Verify index accuracy                | All searches use correct blocks    |
| 5      | Sequential processing                | All records processed in order     |

### 6.3 Future Tests (Addition/Deletion)

| Test # | Description                          | Expected Result                    |
|--------|--------------------------------------|------------------------------------|
| 1      | Add record (no split)                | Record added, block updated        |
| 2      | Add record (causes split)            | Two blocks created, links updated  |
| 3      | Delete record (no merge)             | Record removed, block updated      |
| 4      | Delete record (redistribution)       | Records redistributed between blocks|
| 5      | Delete record (causes merge)         | Blocks merged, avail list updated  |
| 6      | Verify avail list usage              | Deleted blocks reused correctly    |
| 7      | Physical vs logical dump differ      | Different after non-appending ops  |

---

## 7. Tools and Environment

### Development Tools:
- **Compiler:** g++ (C++17 or later)
- **Build System:** VSCode tasks / Makefile
- **Documentation:** Doxygen
- **Version Control:** Git

### Key Libraries:
- `<fstream>` - File I/O
- `<vector>` - Dynamic arrays
- `<map>` - Index storage
- `<string>` - String manipulation
- `<algorithm>` - Sorting

### File Locations:
- **Source:** `src/*.cpp`
- **Headers:** `headers/*.h`
- **Data:** `data/*.dat`, `data/*.bss`, `data/*.idx`
- **Documentation:** `docs/*.md`, `docs/*.pdf`

---

## 8. Command-Line Interface

### Usage Patterns:

```bash
# Interactive mode
./program -i
./program --interactive

# Search test demonstration
./program --test

# Search specific zip codes
./program <bss_file> -Z<zip1> [-Z<zip2> ...]

# Examples
./program data/zipCodes.bss -Z10001
./program data/zipCodes.bss -Z10001 -Z90210 -Z60601
```

### Command-Line Flags:
- `-i, --interactive` - Start interactive search mode
- `--test` - Run automated search test
- `-Z<zipcode>` - Search for specific zip code

---

## 9. Assumptions and Constraints

### Assumptions:
- All input files are well-formed and follow Project 2.0 format
- ZipCode field is unique (primary key)
- Records fit within block size constraints
- System has sufficient memory for index (not entire file)
- File system supports binary file operations

### Constraints:
- **Block Size:** Default 512 bytes (configurable)
- **Minimum Capacity:** 50% (configurable)
- **Platform:** Windows, macOS, or Linux
- **Language:** C++17 or later
- **Index Storage:** Must fit in RAM
- **Data File:** Remains on disk (never fully loaded)

---

## 10. Implementation Status

### [OK] Completed Features:
1. BSS file creation from Project 2.0 file
2. Block buffer class ([`BSSBlock`](../headers/BSSBlock.h))
3. Record buffer class ([`ZipCodeRecordBuffer`](../headers/ZipCodeRecordBuffer.h))
4. File header management ([`BSSFileHeader`](../headers/BSSFileHeader.h))
5. Physical dump method ([`dumpPhysical()`](../src/BSSFile.cpp:183))
6. Logical dump method ([`dumpLogical()`](../src/BSSFile.cpp:202))
7. Simple index creation and management ([`BSSIndex`](../headers/BSSIndex.h))
8. Index-based search functionality
9. Sequential record processing
10. Command-line interface with multiple modes

### 🚧 Future Implementation:
1. Record addition with block splitting
2. Record deletion with redistribution
3. Record deletion with block merging
4. Avail list management and reuse
5. Block split logging
6. Block merge logging
7. Index modification logging
8. Comprehensive error handling
9. Performance optimization

---

## 11. Notes for Future Updates

### Priority Enhancements:
- Implement record addition with block splitting logic
- Implement record deletion with merge/redistribution
- Add comprehensive logging for all operations
- Optimize index search with binary search
- Add validation for block capacity constraints
- Implement avail list block reuse
- Add transaction support for atomic operations
- Create comprehensive test suite for edge cases

### Documentation Needs:
- Complete Doxygen comments for all methods
- Create user guide with examples
- Document block split algorithm
- Document merge/redistribution algorithm
- Add architecture diagrams
- Create troubleshooting guide

---

## 12. References

- Folk, M. J., Zoellick, B., & Riccardi, G. (2011). *File Structures: An Object-Oriented Approach with C++*
  - Section 6.2.2: Avail Lists
  - Section 10.1-10.3: Indexed Sequential Files
  - Figure 10.3: Simple Index Structure
- Project 2.0 Specification
- Project 3.0 Requirements Document

---

**Document Version:** 1.0  
**Last Updated:** November 17, 2025  
**Status:** Preliminary Design - Ready for Review
