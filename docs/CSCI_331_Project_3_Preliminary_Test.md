# CSCI 331 - Project 3 Preliminary Test Document

**Course:** CSCI 331  
**Project:** Zip Code Record Processor: Part 3 - Blocked Sequence Set  
**Date:** 11/05/2025  
**Team:** Team 5

---

## 1. Testing Overview

This document outlines the comprehensive testing strategy for the Blocked Sequence Set (BSS) implementation. Testing is divided into multiple phases to ensure all functionality works correctly before, during, and after implementation of advanced features.

### Testing Objectives:
1. Verify BSS file creation and structure
2. Validate block buffer and record buffer operations
3. Test physical and logical dump methods
4. Verify index creation and search functionality
5. Test record addition with block splitting (future)
6. Test record deletion with redistribution and merging (future)
7. Validate avail list management (future)

---

## 2. Test Environment Setup

### 2.1 Test Data Files

| File Name                    | Description                          | Size        |
|------------------------------|--------------------------------------|-------------|
| `us_postal_codes.csv`        | Original CSV data                    | ~40,000 records |
| `newBinaryPCodes.dat`        | Project 2.0 length-indicated file    | Variable    |
| `zipCodes.bss`               | BSS file (512-byte blocks)           | Variable    |
| `zipCodes.bss.idx`           | Simple index file                    | Variable    |
| `test_small.dat`             | Small test file (10-20 records)      | ~2 KB       |
| `test_medium.dat`            | Medium test file (100-200 records)   | ~20 KB      |

### 2.2 Test Block Sizes

We will test with multiple block sizes to verify flexibility:
- **512 bytes** (default)
- **256 bytes** (smaller, forces more splits)
- **1024 bytes** (larger, fewer blocks)

### 2.3 Test Tools

- **Compiler:** g++ with `-std=c++17 -Wall -Wextra`
- **Debugger:** gdb or Visual Studio debugger
- **Memory Check:** valgrind (Linux) or similar
- **Diff Tool:** For comparing dump outputs

---

## 3. Phase 1: Basic Functionality Tests

### Test 1.1: BSS File Creation

**Objective:** Verify BSS file is created correctly from Project 2.0 file

**Test Steps:**
1. Run: `./program` (creates BSS file automatically)
2. Verify file `data/zipCodes.bss` exists
3. Check file size is reasonable (> 0 bytes)
4. Verify header block (RBN 0) is written

**Expected Results:**
- BSS file created successfully
- File size = (block_size × block_count)
- Header contains correct metadata
- No error messages

**Success Criteria:**
```
✓ File created: data/zipCodes.bss
✓ Block count: [expected number]
✓ Record count: [expected number]
✓ List head RBN: 1
✓ Avail head RBN: -1
```

---

### Test 1.2: Block Buffer Read/Write

**Objective:** Verify [`BSSBlock`](../headers/BSSBlock.h) can read and write blocks correctly

**Test Steps:**
1. Create a test block with known data
2. Write block to file at RBN 5
3. Read block back from RBN 5
4. Compare original and read data

**Expected Results:**
- Block written successfully
- Block read successfully
- Data matches exactly (byte-for-byte)
- Block header preserved correctly

**Test Code:**
```cpp
BSSBlock block1(512);
// Add test records to block1
block1.write(file, 5);

BSSBlock block2(512);
block2.read(file, 5);

// Verify: block1 data == block2 data
```

---

### Test 1.3: Record Buffer Unpacking

**Objective:** Verify [`ZipCodeRecordBuffer`](../headers/ZipCodeRecordBuffer.h) unpacks records correctly

**Test Steps:**
1. Create block with 3 known records
2. Use [`unpackAllRecords()`](../src/BSSBlock.cpp:160)
3. Verify each field of each record

**Expected Results:**
- All 3 records unpacked
- All fields match original data
- No data corruption

**Test Data:**
```
Record 1: 10001, New York, NY, New York, 40.7500, -73.9967
Record 2: 90210, Beverly Hills, CA, Los Angeles, 34.0901, -118.4065
Record 3: 60601, Chicago, IL, Cook, 41.8857, -87.6180
```

---

### Test 1.4: Physical Dump

**Objective:** Verify [`dumpPhysical()`](../src/BSSFile.cpp:183) lists all blocks sequentially

**Test Steps:**
1. Create BSS file with known structure
2. Run physical dump
3. Verify output shows all blocks in order (0, 1, 2, 3, ...)

**Expected Output Format:**
```
--- Physical Block Dump ---
RBN 0: Type: H, Records: 0, Prev: -1, Next: -1, HighestKey: 
RBN 1: Type: A, Records: 15, Prev: -1, Next: 2, HighestKey: 10500
RBN 2: Type: A, Records: 15, Prev: 1, Next: 3, HighestKey: 20500
RBN 3: Type: A, Records: 15, Prev: 2, Next: -1, HighestKey: 30500
---------------------------
```

**Success Criteria:**
- All blocks listed
- RBNs are sequential (0, 1, 2, 3, ...)
- Predecessor/successor links are correct
- Record counts are reasonable

---

### Test 1.5: Logical Dump

**Objective:** Verify [`dumpLogical()`](../src/BSSFile.cpp:202) follows sequence set links

**Test Steps:**
1. Use same BSS file from Test 1.4
2. Run logical dump
3. Verify output follows successor links

**Expected Output Format:**
```
--- Logical Block Dump ---
RBN 1: Type: A, Records: 15, Prev: -1, Next: 2, HighestKey: 10500
RBN 2: Type: A, Records: 15, Prev: 1, Next: 3, HighestKey: 20500
RBN 3: Type: A, Records: 15, Prev: 2, Next: -1, HighestKey: 30500
-------------------------
```

**Success Criteria:**
- Blocks listed in logical order
- Follows successor links (not RBN order)
- Initially identical to physical dump
- No infinite loops

---

### Test 1.6: Physical vs Logical Dump Comparison

**Objective:** Verify both dumps are initially identical

**Test Steps:**
1. Run physical dump > `physical.txt`
2. Run logical dump > `logical.txt`
3. Use `diff physical.txt logical.txt`

**Expected Results:**
- Files are identical (diff shows no differences)
- Both show same block order
- Both show same metadata

**Note:** After implementing deletion/insertion with avail list, these will differ!

---

## 4. Phase 2: Index and Search Tests

### Test 2.1: Index Creation

**Objective:** Verify [`BSSIndex::build()`](../src/BSSIndex.cpp:11) creates correct index

**Test Steps:**
1. Create BSS file with known blocks
2. Build index
3. Verify index contains correct mappings

**Expected Results:**
- Index entry count = active block count
- Each entry maps highest key → correct RBN
- Index is sorted by key

**Test Verification:**
```cpp
BSSIndex index;
index.build(bssFile);
index.dump(cout);

// Verify output shows:
// Key: 10500 -> RBN: 1
// Key: 20500 -> RBN: 2
// Key: 30500 -> RBN: 3
```

---

### Test 2.2: Index File Write/Read

**Objective:** Verify index persists correctly to disk

**Test Steps:**
1. Build index
2. Write to file: `index.write("test.idx")`
3. Create new index object
4. Read from file: `index2.read("test.idx")`
5. Compare both indexes

**Expected Results:**
- File created successfully
- Index2 contains same entries as index1
- All mappings preserved
- No data corruption

---

### Test 2.3: Search for Valid Zip Codes

**Objective:** Verify search finds existing records

**Test Steps:**
1. Load index into RAM
2. Search for known zip codes: 10001, 90210, 60601
3. Verify each is found

**Expected Output:**
```
Searching for ZIP: 10001
  Index indicates block RBN: 1
  Block contains 15 records
  [FOUND]: 10001, New York, NY, New York, 40.7500, -73.9967

Searching for ZIP: 90210
  Index indicates block RBN: 45
  Block contains 12 records
  [FOUND]: 90210, Beverly Hills, CA, Los Angeles, 34.0901, -118.4065
```

**Success Criteria:**
- All valid zips found
- Correct block identified by index
- Only one block read per search
- Full record displayed

---

### Test 2.4: Search for Invalid Zip Codes

**Objective:** Verify search correctly reports non-existent records

**Test Steps:**
1. Search for invalid zip codes: 00000, 99999, 12345
2. Verify "not found" message

**Expected Output:**
```
Searching for ZIP: 00000
  [INVALID]: ZIP code 00000 not found (no matching block in index)

Searching for ZIP: 99999
  Index indicates block RBN: 150
  Block contains 8 records
  [NOT FOUND]: ZIP code 99999 not found in block 150
    (block was read, unpacked, and searched)
```

**Success Criteria:**
- Invalid zips not found
- Appropriate error messages
- Block searched if index indicates possible match
- No false positives

---

### Test 2.5: Search Test Demonstration

**Objective:** Run automated test with mix of valid/invalid zips

**Test Steps:**
1. Run: `./program --test`
2. Verify test runs automatically
3. Check summary statistics

**Expected Output:**
```
=== SEARCH TEST DEMONSTRATION ===
Testing 8 zip codes (including invalid ones)...

[Test 1] Searching for ZIP: 10001
  → Index lookup: Block RBN 1
  → Block contains 15 records
  [VALID]: Found in block 1
     10001, New York, NY, New York, 40.7500, -73.9967

[Test 2] Searching for ZIP: 00000
  [INVALID]: ZIP code 00000 not found (no matching block in index)

...

=== SEARCH TEST SUMMARY ===
Total searches: 8
Valid zip codes found: 5
Invalid zip codes: 3

[OK] Index was loaded into RAM
[OK] BSS file was NEVER fully loaded into RAM
[OK] Only individual blocks were read as needed
```

**Success Criteria:**
- All tests complete without crashes
- Correct count of valid/invalid
- Index loaded only once
- BSS file never fully loaded

---

### Test 2.6: Interactive Search Mode

**Objective:** Verify interactive mode works correctly

**Test Steps:**
1. Run: `./program -i`
2. Enter several zip codes
3. Type 'quit' to exit

**Expected Behavior:**
- Prompts for zip code input
- Searches each entered zip
- Continues until 'quit' entered
- Shows search count on exit

---

## 5. Phase 3: Sequential Processing Tests

### Test 3.1: Process All Records Sequentially

**Objective:** Verify all records can be read in order

**Test Steps:**
1. Open BSS file
2. Follow logical links from listHeadRBN
3. Unpack all records from each block
4. Count total records

**Expected Results:**
- All blocks visited
- All records unpacked
- Total count matches header.recordCount
- Records in sorted order by zip code

---

### Test 3.2: Extreme Zip Code Finder

**Objective:** Verify sequential processing finds correct extremes

**Test Steps:**
1. Run extreme zip code finder
2. Verify results for known states

**Expected Output:**
```
State: CA
  Easternmost:  92283 (Lon: -114.6315)
  Westernmost:  96162 (Lon: -124.2096)
  Northernmost: 96094 (Lat: 41.9952)
  Southernmost: 92154 (Lat: 32.5519)
```

**Success Criteria:**
- All states processed
- Extreme values are correct
- No records skipped

---

## 6. Phase 4: Future Tests (Addition/Deletion)

### Test 4.1: Add Record (No Split Required)

**Objective:** Add record to block with available space

**Test Steps:**
1. Create small BSS file with partially full blocks
2. Add new record that fits in existing block
3. Verify record added correctly
4. Run dumps to verify structure

**Expected Results:**
- Record added to correct block
- Block record count increased by 1
- Records remain sorted
- Predecessor/successor links unchanged
- Index updated if highest key changed

**Test Data:**
```
Existing block: 10001, 10002, 10003 (capacity for 5)
Add: 10002.5 (fictional zip for testing)
Result: 10001, 10002, 10002.5, 10003
```

---

### Test 4.2: Add Record (Block Split Required)

**Objective:** Add record that causes block to split

**Test Steps:**
1. Create block at full capacity
2. Add new record
3. Verify block split occurs
4. Check avail list usage

**Expected Results:**
- Original block split into two blocks
- Records distributed evenly
- New block uses avail list if available
- Predecessor/successor links updated
- Index updated with new block
- **Split event logged**

**Test Scenario:**
```
Before:
  RBN 5: [10001, 10002, 10003, 10004, 10005] (full)

Add: 10003.5

After:
  RBN 5: [10001, 10002, 10003]
  RBN 6: [10003.5, 10004, 10005]
  
Links:
  RBN 5: prev=4, next=6
  RBN 6: prev=5, next=7
```

**Log Output:**
```
[SPLIT] Block 5 split into blocks 5 and 6
  Original records: 5
  Block 5 now has: 3 records
  Block 6 now has: 3 records
  Highest key in block 5: 10003
  Highest key in block 6: 10005
```

---

### Test 4.3: Delete Record (No Merge Required)

**Objective:** Delete record from block that remains above 50% capacity

**Test Steps:**
1. Delete record from block with sufficient records
2. Verify record removed
3. Verify block remains active

**Expected Results:**
- Record removed successfully
- Block record count decreased by 1
- Records remain sorted
- Block remains active (not on avail list)
- Links unchanged

**Test Data:**
```
Before: [10001, 10002, 10003, 10004, 10005] (5 records, 50% = 2.5)
Delete: 10003
After:  [10001, 10002, 10004, 10005] (4 records, still > 50%)
```

---

### Test 4.4: Delete Record (Redistribution Required)

**Objective:** Delete record causing block to fall below 50% capacity

**Test Steps:**
1. Delete record from block near minimum capacity
2. Verify redistribution with adjacent block
3. Check both blocks remain active

**Expected Results:**
- Record deleted
- Records redistributed between blocks
- Both blocks remain active
- Both blocks now at ~50% capacity
- Links unchanged
- Index updated if highest keys changed
- **Redistribution event logged**

**Test Scenario:**
```
Before:
  RBN 5: [10001, 10002, 10003] (3 records, min=2.5)
  RBN 6: [10004, 10005, 10006, 10007] (4 records)

Delete: 10002 from RBN 5

After (redistribute):
  RBN 5: [10001, 10003, 10004] (3 records)
  RBN 6: [10005, 10006, 10007] (3 records)
```

**Log Output:**
```
[REDISTRIBUTION] Blocks 5 and 6 redistributed
  Block 5: 2 records → 3 records
  Block 6: 4 records → 3 records
  New highest key in block 5: 10004
  New highest key in block 6: 10007
```

---

### Test 4.5: Delete Record (Merge Required)

**Objective:** Delete record causing blocks to merge

**Test Steps:**
1. Delete record from small block
2. Verify merge with adjacent block
3. Verify rightmost block added to avail list

**Expected Results:**
- Record deleted
- Blocks merged into one
- Rightmost block cleared and added to avail list
- Links updated to skip merged block
- Index updated (merged block entry removed)
- **Merge event logged**

**Test Scenario:**
```
Before:
  RBN 5: [10001, 10002] (2 records)
  RBN 6: [10003, 10004] (2 records)

Delete: 10002 from RBN 5

After (merge):
  RBN 5: [10001, 10003, 10004] (3 records)
  RBN 6: [empty - on avail list]
  
Links:
  RBN 5: prev=4, next=7 (skips RBN 6)
  RBN 6: type='V', next=availHead
```

**Log Output:**
```
[MERGE] Blocks 5 and 6 merged into block 5
  Combined records: 3
  Block 6 cleared and added to avail list
  Avail list head: 6
  Index entry for block 6 removed
```

---

### Test 4.6: Avail List Management

**Objective:** Verify deleted blocks are reused correctly

**Test Steps:**
1. Delete records to create avail blocks
2. Add new records
3. Verify avail blocks are reused before creating new blocks

**Expected Results:**
- Deleted blocks added to avail list
- Avail list maintained as linked list
- New blocks use avail list first
- Block count doesn't increase unnecessarily

**Test Scenario:**
```
1. Delete records → RBN 10 added to avail list
2. Delete records → RBN 15 added to avail list
3. Avail list: 15 → 10 → -1

4. Add record requiring new block
5. RBN 15 reused (removed from avail list)
6. Avail list: 10 → -1

7. Add another record requiring new block
8. RBN 10 reused
9. Avail list: -1 (empty)
```

---

### Test 4.7: Physical vs Logical Dump After Operations

**Objective:** Verify dumps differ after non-appending operations

**Test Steps:**
1. Perform deletions and insertions using avail list
2. Run physical dump
3. Run logical dump
4. Compare outputs

**Expected Results:**
- Physical dump shows blocks in file order (0, 1, 2, 3, ...)
- Logical dump shows blocks in key order (may skip RBNs)
- Dumps are different
- Both dumps are valid

**Example:**
```
Physical Dump:
RBN 0: Header
RBN 1: Active (keys 10001-10500)
RBN 2: Active (keys 15001-15500) ← reused avail block
RBN 3: Active (keys 10501-11000)
RBN 4: Active (keys 11001-11500)

Logical Dump:
RBN 1: Active (keys 10001-10500)
RBN 3: Active (keys 10501-11000)
RBN 4: Active (keys 11001-11500)
RBN 2: Active (keys 15001-15500)
```

---

## 7. Performance Tests

### Test 7.1: Large File Handling

**Objective:** Verify system handles large datasets

**Test Steps:**
1. Create BSS file with 40,000+ records
2. Build index
3. Perform searches
4. Measure performance

**Expected Results:**
- File created successfully
- Index builds in reasonable time (< 10 seconds)
- Searches complete quickly (< 1 second each)
- Memory usage reasonable

---

### Test 7.2: Index Efficiency

**Objective:** Verify index reduces block reads

**Test Steps:**
1. Search 100 random zip codes with index
2. Count total block reads
3. Compare to sequential search

**Expected Results:**
- With index: ~100 block reads (one per search)
- Without index: ~5000+ block reads (sequential scan)
- Index provides significant speedup

---

## 8. Error Handling Tests

### Test 8.1: File Not Found

**Test:** Open non-existent BSS file  
**Expected:** Clear error message, graceful exit

### Test 8.2: Corrupted Block

**Test:** Manually corrupt a block, try to read  
**Expected:** Error detected, appropriate handling

### Test 8.3: Invalid Index

**Test:** Corrupt index file, try to use  
**Expected:** Index rebuild triggered

### Test 8.4: Disk Full

**Test:** Simulate disk full during write  
**Expected:** Error message, no data corruption

---

## 9. Test Execution Checklist

### Pre-Implementation Tests (Current Phase)
- [x] Test 1.1: BSS File Creation
- [x] Test 1.2: Block Buffer Read/Write
- [x] Test 1.3: Record Buffer Unpacking
- [x] Test 1.4: Physical Dump
- [x] Test 1.5: Logical Dump
- [x] Test 1.6: Physical vs Logical Comparison
- [x] Test 2.1: Index Creation
- [x] Test 2.2: Index File Write/Read
- [x] Test 2.3: Search for Valid Zip Codes
- [x] Test 2.4: Search for Invalid Zip Codes
- [x] Test 2.5: Search Test Demonstration
- [x] Test 2.6: Interactive Search Mode
- [x] Test 3.1: Sequential Processing
- [x] Test 3.2: Extreme Zip Code Finder

### Post-Implementation Tests (Future)
- [ ] Test 4.1: Add Record (No Split)
- [ ] Test 4.2: Add Record (Block Split)
- [ ] Test 4.3: Delete Record (No Merge)
- [ ] Test 4.4: Delete Record (Redistribution)
- [ ] Test 4.5: Delete Record (Merge)
- [ ] Test 4.6: Avail List Management
- [ ] Test 4.7: Physical vs Logical Dump Difference
- [ ] Test 7.1: Large File Handling
- [ ] Test 7.2: Index Efficiency
- [ ] Error Handling Tests

---

## 10. Test Data Generation

### Small Test File (10-20 records)
```
10001, New York, NY, New York, 40.7500, -73.9967
10002, New York, NY, New York, 40.7500, -73.9967
10003, New York, NY, New York, 40.7500, -73.9967
...
10020, New York, NY, New York, 40.7500, -73.9967
```

### Test File with Known Structure
- Block 1: Zips 10001-10015 (15 records)
- Block 2: Zips 10016-10030 (15 records)
- Block 3: Zips 10031-10045 (15 records)

This allows predictable testing of block operations.

---

## 11. Success Criteria Summary

### Phase 1 (Basic Functionality)
✓ BSS file created correctly  
✓ Blocks read/written accurately  
✓ Records unpacked correctly  
✓ Both dump methods work  
✓ Dumps initially identical

### Phase 2 (Index and Search)
✓ Index built correctly  
✓ Index persists to disk  
✓ Valid zips found  
✓ Invalid zips reported correctly  
✓ Only indexed blocks read

### Phase 3 (Sequential Processing)
✓ All records processed  
✓ Correct order maintained  
✓ Extreme values found

### Phase 4 (Future - Addition/Deletion)
☐ Records added correctly  
☐ Block splits handled  
☐ Records deleted correctly  
☐ Redistribution works  
☐ Merges work  
☐ Avail list managed  
☐ Dumps differ after operations

---

## 12. Test Reporting Template

For each test, document:

```
Test ID: [e.g., 2.3]
Test Name: [e.g., Search for Valid Zip Codes]
Date: [execution date]
Tester: [name]
Status: [PASS/FAIL/BLOCKED]

Steps Executed:
1. [step 1]
2. [step 2]
...

Actual Results:
[what actually happened]

Expected Results:
[what should have happened]

Pass/Fail Criteria Met: [Yes/No]

Issues Found:
[any bugs or problems]

Notes:
[additional observations]
```

---

## 13. Regression Testing

After implementing new features, re-run all previous tests to ensure:
- Existing functionality still works
- No new bugs introduced
- Performance hasn't degraded

---

## 14. Notes for Test Execution

### Tips:
- Use small test files initially (easier to debug)
- Verify dumps manually for small files
- Use diff tool to compare expected vs actual output
- Log all operations for debugging
- Test edge cases (empty blocks, single record, etc.)
- Test with different block sizes

### Common Issues to Watch For:
- Off-by-one errors in RBN calculations
- Incorrect predecessor/successor links
- Memory leaks in buffer classes
- Index not updated after modifications
- Avail list corruption
- Infinite loops in link following

---

**Document Version:** 1.0  
**Last Updated:** November 17, 2025  
**Status:** Preliminary Test Plan - Ready for Review
