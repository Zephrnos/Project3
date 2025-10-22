
# 🚀 CSCI 331 - Zip Code Project 3.0 🚀

Hey team! [cite_start]👋 Welcome to our collaborative space for the **CSCI 331 Group Project 3.0**[cite: 2, 3]. This is where we'll be tackling the challenge of building a sophisticated file structure for zip code data. Let's get this done!

## ✨ Our Mission ✨

[cite_start]Our main goal is to build a C++ program that creates and manages a **blocked sequence set file** for zip code records[cite: 9]. [cite_start]We're moving beyond simple files and building a persistent, indexed system that can handle additions, deletions, and efficient searches[cite: 35, 48, 51].

## 🎯 Core Features We're Building

* [cite_start]**📁 Blocked Sequence Set:** We'll generate a file with fixed-size blocks (default 512 bytes!) to hold our data[cite: 9, 12, 73].
* [cite_start]**🧠 Simple Primary Key Index:** We're creating an index file (`{<highest key>, <RBN>}`) that gets loaded into RAM for super-fast searches[cite: 35, 38, 39, 49]. [cite_start]No loading the whole data file! [cite: 50]
* [cite_start]**🏎️ Speedy Search Program:** A tool that uses our index to find zip codes passed in via command line (like `-256301`)[cite: 38, 44, 45]. [cite_start]It needs to be smart enough to say "Not found!" if a zip isn't there[cite: 46, 47].
* [cite_start]**➕ Record Addition:** Our program will add new records from a file[cite: 52]. [cite_start]This includes handling **block splits** when a block gets too full! [cite: 53]
* [cite_start]**➖ Record Deletion:** We'll also delete records by their key[cite: 57]. This is the tricky part! We need to manage:
    * [cite_start]**Block redistribution** [cite: 58]
    * [cite_start]**Block merges** [cite: 58]
    * [cite_start]Putting empty blocks on the **avail list** [cite: 14, 119]
* [cite_start]**📊 Two Dump Utilities:** We need to create two ways to "see" our file structure[cite: 21]:
    1.  [cite_start]**Physical Order Dump:** Just lists blocks as they appear in the file[cite: 22].
    2.  [cite_start]**Logical Order Dump:** Follows the predecessor/successor links to show the blocks in key order[cite: 23].
* [cite_start]**📚 Buffer Classes:** We'll use a `BlockBuffer` (to read from blocks) and a `RecordBuffer` (to unpack fields) to manage our data flow[cite: 15, 17, 18].
* [cite_start]**📜 Awesome Documentation:** We've got to document our code extensively with Doxygen and create a `Doxygen PDF`[cite: 64, 65].
* [cite_start]**📖 User Guide:** A clear and simple user guide showing everyone how to use our amazing program[cite: 66, 106].

## 🏗️ File Architecture (The Blueprint)

### 1. The Header Record (The "Brains") 🧠
[cite_start]This special record at the start of our file stores all the metadata[cite: 6, 19]:
* [cite_start]File structure type & version [cite: 68, 69]
* [cite_start]Block size (e.g., 512 bytes) [cite: 73]
* [cite_start]Min block capacity (e.g., 50%) [cite: 74]
* [cite_start]Record and block counts [cite: 77, 78]
* [cite_start]Schema info (field names, types, primary key) [cite: 79, 81, 82, 84]
* [cite_start]**RBN link** to the start of the avail-list 👻 [cite: 85]
* [cite_start]**RBN link** to the start of the active data list 📦 [cite: 86]

### 2. Active Blocks (The Data) 📦
[cite_start]These are the blocks that hold our actual zip code records[cite: 89].
* [cite_start]Record count (> 0) [cite: 93]
* [cite_start]Links to the **previous** & **next** active blocks [cite: 95]
* [cite_start]A set of records, all sorted by key! [cite: 96]

### 3. Avail List Blocks (The "Graveyard") 👻
[cite_start]These are blocks that are empty and ready to be reused[cite: 14, 97].
* [cite_start]Record count (must be 0!) [cite: 98]
* [cite_start]A link to the **next** avail block [cite: 99]
* (The rest is just empty space) [cite_start][cite: 100]

## 📬 What We Need to Turn In (The Checklist!)

We've got two big deadlines:

### [cite_start]📋 **Phase 1: Before We Code (Due 1 week prior)** [cite: 102]
* [cite_start][ ] Preliminary Design Doc 📝 [cite: 103]
* [cite_start][ ] Preliminary Test Doc 🧪 [cite: 104]

### [cite_start]🎁 **Phase 2: The Final Submission** [cite: 105]
* [cite_start][ ] The User Guide (`.txt`) 📖 [cite: 106]
* [cite_start][ ] Our length-indicated data files (`.txt`) [cite: 107]
* [cite_start][ ] The simple index file (`.txt`) [cite: 108]
* [cite_start][ ] The Doxygen PDF 📑 [cite: 109]
* [cite_start][ ] All our `.cpp` and `.h` source code (in a `.zip` file) 💻 [cite: 110, 120]
* [cite_start][ ] The final script file showing our program running [cite: 111]
* [cite_start][ ] The final Design Doc 📝 [cite: 111]
* [cite_start][ ] The final Test Doc 🧪 (this one is HUGE!) [cite: 112]

### 🧪 **Final Test Doc Requirements**
[cite_start]We *must* show tests for[cite: 112, 113]:
* [cite_start][ ] Adding a record (no split) [cite: 115]
* [cite_start][ ] Adding a record (that *causes* a block split) [cite: 116]
* [cite_start][ ] Deleting a record (no merge/redistribution) [cite: 117]
* [cite_start][ ] Deleting a record (that *causes* redistribution) [cite: 118]
* [cite_start][ ] Deleting a record (that *causes* a block merge) [cite: 119]

---

Let's do this, team! Let's communicate, split up the work, and build an awesome project. 💪
