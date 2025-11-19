# Project 3 Preliminary Documents - Overview

**Course:** CSCI 331  
**Project:** Zip Code Record Processor: Part 3 - Blocked Sequence Set  
**Date:** 11/18/2025  
**Team:** Team 5

---

## 📋 Document Summary

This directory contains the preliminary design and test documents for Project 3, submitted **one week prior to final submission** as required by the project specification.

### Documents Included:

1. **[CSCI_331_Project_3_Preliminary_Design.md](CSCI_331_Project_3_Preliminary_Design.md)**
   - Comprehensive design document covering architecture, classes, algorithms, and implementation plan
   - 476 lines of detailed technical specifications
   - Includes file structure, data flow, and class relationships

2. **[CSCI_331_Project_3_Preliminary_Test.md](CSCI_331_Project_3_Preliminary_Test.md)**
   - Complete testing strategy with 40+ test cases
   - 876 lines covering unit tests, integration tests, and future test scenarios
   - Includes test data, expected results, and success criteria

---

## 🎯 Project Status

### [OK] Currently Implemented Features:

The following features are **already implemented** and working in the codebase:

1. **BSS File Creation** - [`BSSFile::create()`](../src/BSSFile.cpp:16)
   - Converts Project 2.0 length-indicated files to blocked sequence set format
   - Creates fixed-size blocks (default 512 bytes)
   - Establishes predecessor/successor links

2. **Block Buffer Class** - [`BSSBlock`](../headers/BSSBlock.h)
   - Reads and writes individual blocks
   - Packs/unpacks records within blocks
   - Manages block-level metadata

3. **Record Buffer Class** - [`ZipCodeRecordBuffer`](../headers/ZipCodeRecordBuffer.h)
   - Unpacks individual records from blocks
   - Provides field-level access to zip code data
   - Handles comma-separated, length-indicated format

4. **File Header Management** - [`BSSFileHeader`](../headers/BSSFileHeader.h)
   - Stores comprehensive file metadata
   - Tracks block count, record count, and RBN links
   - Maintains field schema information

5. **Physical Dump Method** - [`BSSFile::dumpPhysical()`](../src/BSSFile.cpp:183)
   - Lists all blocks in physical file order (RBN 0, 1, 2, 3, ...)
   - Shows block metadata and links
   - Useful for debugging file structure

6. **Logical Dump Method** - [`BSSFile::dumpLogical()`](../src/BSSFile.cpp:202)
   - Lists blocks by following successor links
   - Shows logical sequence set ordering
   - Initially identical to physical dump

7. **Simple Index** - [`BSSIndex`](../headers/BSSIndex.h)
   - Maps highest key in each block to RBN
   - Enables efficient block lookup
   - Persists to disk as binary file

8. **Index-Based Search** - [`searchWithIndex()`](../src/main.cpp:228)
   - Loads index into RAM (not entire file)
   - Uses index to locate target block
   - Reads only necessary blocks

9. **Command-Line Interface** - [`main()`](../src/main.cpp:541)
   - Interactive mode (`-i` or `--interactive`)
   - Test mode (`--test`)
   - Direct search mode (`-Z<zipcode>`)

10. **Sequential Processing** - Multiple functions
    - Processes all records in logical order
    - Finds extreme zip codes by state
    - Demonstrates buffer class hierarchy

### 🚧 Future Implementation (Not Yet Complete):

The following features are **planned but not yet implemented**:

1. **Record Addition with Block Splitting**
   - Add records to existing blocks
   - Split blocks when capacity exceeded
   - Update predecessor/successor links
   - Modify index as needed

2. **Record Deletion with Redistribution**
   - Remove records from blocks
   - Redistribute records when block falls below 50%
   - Maintain minimum capacity constraint

3. **Record Deletion with Block Merging**
   - Merge blocks when redistribution insufficient
   - Clear rightmost block
   - Add cleared block to avail list

4. **Avail List Management**
   - Maintain linked list of deleted blocks
   - Reuse avail blocks before creating new ones
   - Update avail list head in file header

5. **Operation Logging**
   - Log block split events
   - Log block merge events
   - Log redistribution events
   - Log index modifications

---

## 📊 Key Achievements Demonstrated

### Requirement 8: Two Dump Methods [OK]

**Question:** Does the code achieve the requirement for two dump methods?

**Answer:** **YES!** The code successfully implements both required dump methods:

#### Physical Dump ([`dumpPhysical()`](../src/BSSFile.cpp:183))
```cpp
for (uint32_t rbn = 0; rbn < header.getBlockCount(); ++rbn) {
    if (!readBlock(rbn, block)) break;
    // Display block info
}
```
- Iterates through **all RBNs sequentially** (0, 1, 2, 3, ...)
- Shows blocks in their **physical file order**
- Includes ALL blocks (header, active, avail)

#### Logical Dump ([`dumpLogical()`](../src/BSSFile.cpp:202))
```cpp
int rbn = header.getListHeadRBN();
while (rbn != -1) {
    if (!readBlock(rbn, block)) break;
    // Display block info
    rbn = h->successorRBN;  // Follow the chain!
}
```
- Starts at the **list head**
- **Follows successor links** through the sequence
- Shows only **active blocks in logical order**

#### Why They're Initially Identical

During initial file creation, blocks are written sequentially, so:
- Physical order: RBN 1 → 2 → 3 → 4 → ...
- Logical order: RBN 1 → 2 → 3 → 4 → ... (same!)

#### When They Become Different

After implementing deletion/insertion with avail list reuse:
- **Physical:** Shows all blocks in file position order
- **Logical:** Follows links, may skip RBNs or show non-sequential order

**Example after reusing avail block:**
```
Physical: RBN 0, 1, 2, 3, 4, 5 (sequential)
Logical:  RBN 1 → 5 → 2 → 3 → 4 (following links)
```

### Why It's Not "Deliberately Visible"

The implementation is **implicit** rather than explicit:
1. Both methods use similar output formatting
2. No visual separation or highlighting of the difference
3. Initial state masks the distinction
4. The key difference is subtle: `for` loop vs `while` loop with link following

---

## 🔍 How to Verify the Implementation

### Test the Dump Methods:

```bash
# Compile and run
g++ -std=c++17 src/*.cpp -I headers -o program

# Run in demo mode (shows logical dump)
./program

# The output will show:
--- Logical Block Dump ---
RBN 1: Type: A, Records: 15, Prev: -1, Next: 2, HighestKey: 10500
RBN 2: Type: A, Records: 15, Prev: 1, Next: 3, HighestKey: 20500
...
```

### Test the Search Functionality:

```bash
# Run search test
./program --test

# Or search specific zip codes
./program data/zipCodes.bss -Z10001 -Z90210

# Or use interactive mode
./program -i
```

---

## 📁 File Structure

```
CSCI331P3/
├── docs/
│   ├── CSCI_331_Project_3_Preliminary_Design.md    ← Design document
│   ├── CSCI_331_Project_3_Preliminary_Test.md      ← Test document
│   ├── PRELIMINARY_DOCUMENTS_README.md             ← This file
│   └── CSCI 331 — Fall 2025 — Group Project — Zip Code 3.0.pdf
├── headers/
│   ├── BSSFile.h           ← Main BSS file manager
│   ├── BSSBlock.h          ← Block buffer class
│   ├── BSSFileHeader.h     ← File header class
│   ├── BSSIndex.h          ← Simple index class
│   └── ZipCodeRecordBuffer.h  ← Record buffer class
├── src/
│   ├── BSSFile.cpp         ← BSS file implementation
│   ├── BSSBlock.cpp        ← Block buffer implementation
│   ├── BSSIndex.cpp        ← Index implementation
│   └── main.cpp            ← Main program with CLI
└── data/
    ├── newBinaryPCodes.dat    ← Project 2.0 input file
    ├── zipCodes.bss           ← Generated BSS file
    └── zipCodes.bss.idx       ← Generated index file
```

---

## 🎓 Academic Integrity Note

These preliminary documents represent our team's original work and planning for Project 3. They demonstrate:

1. **Understanding** of blocked sequence set concepts
2. **Planning** before implementation
3. **Comprehensive** test strategy
4. **Professional** documentation standards

The documents are submitted **one week prior to final submission** as required by the project specification.

---

## 📞 Team Contact

**Team 5**  
CSCI 331 - Fall 2025  
St. Cloud State University

For questions about these documents, please contact the team through the course management system.

---

## 📝 Document Versions

| Document | Version | Date | Status |
|----------|---------|------|--------|
| Preliminary Design | 1.0 | 11/18/2025 | Ready for Review |
| Preliminary Test | 1.0 | 11/18/2025 | Ready for Review |
| This README | 1.0 | 11/18/2025 | Complete |

---

## [OK] Submission Checklist

- [x] Preliminary design document created
- [x] Preliminary test document created
- [x] Documents follow Project 2 format
- [x] All sections completed
- [x] Code references included
- [x] Test cases defined
- [x] Success criteria specified
- [x] Ready for instructor review

---

**Next Steps:**
1. Submit these preliminary documents for review
2. Receive feedback from instructor
3. Continue implementation of remaining features
4. Update documents based on feedback
5. Prepare final submission with complete implementation

---

*Last Updated: November 19, 2025*
