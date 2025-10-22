# 🚀 CSCI 331 - Zip Code Project 3.0 🚀

Hey team! 👋 Welcome to our collaborative space for the **CSCI 331 Group Project 3.0**. This is where we'll be tackling the challenge of building a sophisticated file structure for zip code data. Let's get this done!

## ✨ Our Mission ✨

Our main goal is to build a C++ program that creates and manages a **blocked sequence set file** for zip code records. We're moving beyond simple files and building a persistent, indexed system that can handle additions, deletions, and efficient searches.

## 🎯 Core Features We're Building

* **📁 Blocked Sequence Set:** We'll generate a file with fixed-size blocks (default 512 bytes!) to hold our data.
* **🧠 Simple Primary Key Index:** We're creating an index file (`{<highest key>, <RBN>}`) that gets loaded into RAM for super-fast searches. No loading the whole data file!
* **🏎️ Speedy Search Program:** A tool that uses our index to find zip codes passed in via command line (like `-256301`). It needs to be smart enough to say "Not found!" if a zip isn't there.
* **➕ Record Addition:** Our program will add new records from a file. This includes handling **block splits** when a block gets too full!
* **➖ Record Deletion:** We'll also delete records by their key. This is the tricky part! We need to manage:
    * **Block redistribution**
    * **Block merges**
    * Putting empty blocks on the **avail list**
* **📊 Two Dump Utilities:** We need to create two ways to "see" our file structure:
    1.  **Physical Order Dump:** Just lists blocks as they appear in the file.
    2.  **Logical Order Dump:** Follows the predecessor/successor links to show the blocks in key order.
* **📚 Buffer Classes:** We'll use a `BlockBuffer` (to read from blocks) and a `RecordBuffer` (to unpack fields) to manage our data flow.
* **📜 Awesome Documentation:** We've got to document our code extensively with Doxygen and create a `Doxygen PDF`.
* **📖 User Guide:** A clear and simple `user_guide.txt` showing everyone how to use our amazing program.

## 🏗️ File Architecture (The Blueprint)

### 1. The Header Record (The "Brains") 🧠
This special record at the start of our file stores all the metadata:
* File type & version
* Block size (e.g., 512 bytes)
* Min block capacity (e.g., 50%)
* Record and block counts
* Schema info (field names, types, etc.)
* **RBN link** to the start of the avail-list 👻
* **RBN link** to the start of the active data list 📦

### 2. Active Blocks (The Data) 📦
These are the blocks that hold our actual zip code records.
* Record count (> 0)
* Links to the **previous** & **next** active blocks
* A set of records, all sorted by key!

### 3. Avail List Blocks (The "Graveyard") 👻
These are blocks that are empty and ready to be reused.
* Record count (must be 0!)
* A link to the **next** avail block
* (The rest is just empty space)

## 📬 What We Need to Turn In (The Checklist!)

We've got two big deadlines:

### 📋 **Phase 1: Before We Code (Due 1 week prior)**
* [ ] Preliminary Design Doc 📝
* [ ] Preliminary Test Doc 🧪

### 🎁 **Phase 2: The Final Submission**
* [ ] The User Guide (`.txt`) 📖
* [ ] Our length-indicated data files (`.txt`)
* [ ] The simple index file (`.txt`)
* [ ] The Doxygen PDF 📑
* [ ] All our `.cpp` and `.h` source code (in a `.zip` file) 💻
* [ ] The final script file showing our program running
* [ ] The final Design Doc 📝
* [ ] The final Test Doc 🧪 (this one is HUGE!)

### 🧪 **Final Test Doc Requirements**
We *must* show tests for:
* [ ] Adding a record (no split)
* [ ] Adding a record (that *causes* a block split)
* [ ] Deleting a record (no merge/redistribution)
* [ ] Deleting a record (that *causes* redistribution)
* [ ] Deleting a record (that *causes* a block merge)

---

Let's do this, team! Let's communicate, split up the work, and build an awesome project. 💪
