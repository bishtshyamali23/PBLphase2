# Buffer Pool and LRU Page Replacement

## Overview

This project implements a Buffer Pool Manager for a custom Database Management System (DBMS) using C++17.

The Buffer Pool Manager stores frequently accessed database pages in main memory to reduce disk I/O operations and improve database performance.

It uses the **LRU (Least Recently Used) Page Replacement Algorithm** to manage memory when the buffer pool becomes full.

## Features

- **Page Fetching:** Loads database pages from storage into memory.
- **LRU Page Replacement:** Replaces the least recently used page when the buffer pool is full.
- **Cache Hit and Miss Tracking:** Tracks how frequently pages are found in memory.
- **Dirty Page Management:** Identifies modified pages that need to be written to disk.
- **Page Flushing:** Writes modified pages back to storage.
- **Page Pinning and Unpinning:** Prevents pages currently in use from being replaced.
- **Thread Safety:** Uses mutex locking to protect shared buffer pool operations.
- **Performance Statistics:** Displays cache hits, cache misses, evictions, and cache hit rate.

## Technologies Used

- C++
- C++17
- STL (Standard Template Library)
- Mutex for synchronization
- Git and GitHub

## Project Structure

```text
PBLphase2/
│
├── BufferPool.h
├── BufferPool.cpp
│
├── tests/
│   └── BufferPoolTest.cpp
│
└── README.md
```

## How It Works

1. When a page is requested, the Buffer Pool first checks whether it already exists in memory.
2. If the page exists, it is returned from memory (Cache Hit).
3. If the page does not exist, it is loaded from storage (Cache Miss).
4. If the buffer pool is full, the LRU algorithm selects the least recently used unpinned page for replacement.
5. If the selected page is dirty, it is written back to storage before eviction.
6. Modified pages can also be flushed manually to storage.

## LRU Page Replacement

The Least Recently Used (LRU) algorithm removes the page that has not been accessed for the longest time.

A doubly linked list (`list`) maintains the order of recently accessed pages.

- Most recently used page: Front of the list.
- Least recently used page: Back of the list.

A hash map is used to efficiently locate pages and their positions.

## Compilation

Compile the Buffer Pool test using:

```bash
g++ -std=c++17 -pthread BufferPool.cpp tests/BufferPoolTest.cpp -o test
```

## Run the Program

On Windows:

```powershell
.\test.exe
```

## Testing

The test program checks:

- Fetching pages into the buffer pool.
- Accessing previously loaded pages.
- Cache hit and miss counting.
- LRU page replacement.
- Modifying and marking pages as dirty.
- Flushing dirty pages to storage.
- Displaying buffer pool statistics.

The current test uses an in-memory storage simulation to verify Buffer Pool functionality.

## Team Project

This module is part of a Custom DBMS Storage Engine project.

The Buffer Pool Manager is responsible for managing database pages in memory and reducing unnecessary disk access.

## Author

bishtshyamali23