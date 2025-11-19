# Preliminary vs Final Documents - Key Differences

**Course:** CSCI 331  
**Project:** Zip Code Record Processor: Part 3  
**Date:** 11/18/2025

---

## 📋 Overview

This guide explains the key differences between **preliminary** and **final** documents for academic projects.

---

## 🔄 Document Evolution

### Preliminary Documents (Week Before Submission)
**Purpose:** Planning and design phase
**Status:** Proposed/Planned
**Audience:** Instructor for feedback

### Final Documents (With Project Submission)
**Purpose:** Documentation of actual implementation
**Status:** Completed/Tested
**Audience:** Instructor for grading, future developers

---

## 🎯 Key Differences

### 1. **Design Document Differences**

| Aspect | Preliminary | Final |
|--------|-------------|-------|
| **Tense** | Future tense ("will implement") | Past tense ("implemented") |
| **Status** | Planned features | Actual implementation |
| **Code References** | Proposed structure | Actual file/line numbers |
| **Algorithms** | Pseudocode/theory | Actual implementation details |
| **Completeness** | May have TBD sections | All sections complete |
| **Diagrams** | Conceptual | Reflects actual code |
| **Issues** | Anticipated challenges | Actual problems encountered |
| **Changes** | N/A | Documents deviations from plan |

**Example - Preliminary:**
```
5.1 BSS File Creation (Planned)

We will implement a method that:
1. Will read Project 2.0 file
2. Will sort records by zip code
3. Will pack records into blocks
```

**Example - Final:**
```
5.1 BSS File Creation (Implemented)

The create() method in BSSFile.cpp (lines 16-141):
1. Reads Project 2.0 file using HeaderRecordBuffer
2. Sorts 40,000+ records using std::sort
3. Packs records into 512-byte blocks
4. Successfully tested with full dataset

Implementation Notes:
- Initially planned 256-byte blocks, changed to 512 for efficiency
- Added error handling for corrupted records (lines 66-77)
- Performance: Creates BSS file in ~2 seconds
```

---

### 2. **Test Document Differences**

| Aspect | Preliminary | Final |
|--------|-------------|-------|
| **Test Cases** | Planned tests | Executed tests |
| **Results** | Expected results | Actual results |
| **Status** | Checkboxes empty | Checkboxes marked |
| **Evidence** | N/A | Screenshots, output logs |
| **Pass/Fail** | N/A | Actual pass/fail status |
| **Issues Found** | N/A | Bugs discovered and fixed |
| **Test Data** | Sample data | Actual test files used |
| **Metrics** | Estimated | Measured (time, memory) |

**Example - Preliminary:**
```
Test 2.3: Search for Valid Zip Codes

Objective: Verify search finds existing records

Test Steps:
1. Load index into RAM
2. Search for known zip codes: 10001, 90210, 60601
3. Verify each is found

Expected Results:
- All valid zips found
- Correct block identified by index
- Only one block read per search

Status: [ ] Not tested yet
```

**Example - Final:**
```
Test 2.3: Search for Valid Zip Codes

Objective: Verify search finds existing records

Test Steps:
1. Load index into RAM
2. Search for known zip codes: 10001, 90210, 60601
3. Verify each is found

Actual Results:
✓ All 3 zip codes found successfully
✓ Index correctly identified blocks: RBN 1, 45, 67
✓ Only one block read per search (verified with debug output)
✓ Average search time: 0.003 seconds

Status: [x] PASSED

Test Date: 11/18/2025
Tester: Team 5
Environment: Windows 10, g++ 11.2.0

Output Log:
```
Searching for ZIP: 10001
  Index indicates block RBN: 1
  Block contains 15 records
  [FOUND]: 10001, New York, NY, New York, 40.7500, -73.9967
```

Issues Found: None
```

---

### 3. **Content Additions in Final Documents**

#### Final Design Document Adds:
1. **Implementation Details Section**
   - Actual code structure
   - File organization
   - Line number references
   - Memory usage statistics

2. **Deviations from Plan**
   - What changed and why
   - Design decisions made during implementation
   - Optimizations applied

3. **Performance Metrics**
   - Actual execution times
   - Memory consumption
   - File sizes

4. **Known Issues/Limitations**
   - Bugs that remain
   - Features not implemented
   - Future improvements needed

5. **Lessons Learned**
   - What worked well
   - What was challenging
   - What would be done differently

#### Final Test Document Adds:
1. **Test Execution Records**
   - Date/time of each test
   - Who ran the test
   - Environment details

2. **Actual Test Results**
   - Pass/fail status for each test
   - Actual output vs expected
   - Screenshots/logs

3. **Bug Reports**
   - Issues discovered during testing
   - How they were fixed
   - Regression test results

4. **Test Coverage Analysis**
   - What was tested
   - What wasn't tested
   - Coverage percentage

5. **Performance Benchmarks**
   - Actual timing data
   - Memory profiling results
   - Comparison to requirements

---

## 📊 Document Maturity Levels

### Preliminary (Planning Phase)
```
Completeness:     ████░░░░░░ 40%
Detail Level:     ███░░░░░░░ 30%
Accuracy:         ██░░░░░░░░ 20% (estimates)
Test Evidence:    ░░░░░░░░░░  0%
```

### Final (Completion Phase)
```
Completeness:     ██████████ 100%
Detail Level:     █████████░ 90%
Accuracy:         ██████████ 100% (measured)
Test Evidence:    █████████░ 90%
```

---

## 🎓 Academic Requirements

### Why Both Are Required:

1. **Preliminary Documents:**
   - Demonstrate planning skills
   - Allow instructor feedback before implementation
   - Show understanding of requirements
   - Identify potential issues early

2. **Final Documents:**
   - Prove implementation matches design
   - Document actual testing performed
   - Provide evidence of working code
   - Serve as project documentation

---

## [OK] Checklist for Final Documents

### Final Design Document Must Include:
- [x] All sections from preliminary document
- [x] Past tense throughout ("implemented", "tested")
- [x] Actual code references with line numbers
- [x] Implementation details and decisions
- [x] Deviations from preliminary design explained
- [x] Performance metrics (actual measurements)
- [x] Known issues and limitations
- [x] Lessons learned section

### Final Test Document Must Include:
- [x] All test cases from preliminary document
- [x] Actual test execution results
- [x] Pass/fail status for each test
- [x] Test evidence (logs, screenshots)
- [x] Bug reports and resolutions
- [x] Test environment details
- [x] Performance benchmarks
- [x] Test coverage analysis
- [x] Regression test results

---

## 📝 Example Comparison

### Section: "BSS File Creation"

**Preliminary Version:**
```
We will create a method to generate BSS files from Project 2.0 files.
The method will read records, sort them, and pack them into blocks.
We expect this to handle approximately 40,000 records.
```

**Final Version:**
```
We implemented the create() method (BSSFile.cpp:16-141) to generate
BSS files from Project 2.0 files. The method successfully:
- Read 41,692 records from newBinaryPCodes.dat
- Sorted records in 0.8 seconds using std::sort
- Packed records into 278 blocks of 512 bytes each
- Created zipCodes.bss (142,336 bytes) in 2.1 seconds total

The implementation differs from the preliminary design in that we
added error handling for corrupted records (skipped 12 invalid records)
and optimized block packing to achieve 95% average block utilization.
```

---

## 🔍 Quality Indicators

### Good Final Documents Show:
✓ Actual implementation matches or exceeds preliminary design  
✓ All tests executed with documented results  
✓ Issues encountered and resolved  
✓ Performance meets or exceeds requirements  
✓ Code is well-documented and maintainable  
✓ Lessons learned for future projects  

### Red Flags in Final Documents:
✗ Still using future tense  
✗ No test results or evidence  
✗ Missing sections from preliminary  
✗ No explanation of design changes  
✗ Vague descriptions without specifics  
✗ No performance data  

---

## 📚 Summary

| Document Type | When | Purpose | Key Content |
|---------------|------|---------|-------------|
| **Preliminary** | 1 week before | Planning | What you WILL do |
| **Final** | With submission | Documentation | What you DID do |

**Remember:** Final documents should tell the complete story of your project from planning through implementation and testing!

---

*This guide helps ensure your final documents meet academic standards and provide complete project documentation.*
