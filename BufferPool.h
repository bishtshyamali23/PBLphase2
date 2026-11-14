#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <iostream>

#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <cstddef>


using namespace std;

class BufferPool {

private:

    // Size of each database page (4 KB)
    static constexpr size_t PAGE_SIZE = 4096;

    // Represents one page stored in a Buffer Pool frame
    struct BufferFrame {
        int pageId = -1;          // ID of the database page
        char data[PAGE_SIZE]{};   // Actual page data
        bool isDirty = false;     // True if the page has been modified
        bool isValid = false;     // True if the frame contains a page
        int pinCount = 0;         // Number of active users of this page
    };

    // Stores all buffer frames available in RAM
    vector<BufferFrame> frames;

    // Maps Page ID to its corresponding frame index
    unordered_map<int, int> pageTable;

    // Maintains page usage order (Front = MRU, Back = LRU)
    list<int> lruList;

    // Stores the position of each page in the LRU list
    unordered_map<int, list<int>::iterator> lruPosition;

    // Protects shared Buffer Pool metadata from concurrent access
    mutable mutex poolMutex;

    // Tracks Buffer Pool performance
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    size_t evictions = 0;

    // Callback functions to communicate with Member 1's Page Manager
    using ReadPageCallback = function<bool(int, char*)>;
    using WritePageCallback = function<bool(int, const char*)>;

    ReadPageCallback readPageFromDisk;
    WritePageCallback writePageToDisk;

    // Updates a page's position when it is accessed
    void updateLRU(int pageId);

    // Finds an empty frame or selects a page for replacement
    int findFreeFrame();

    // Removes a page from the Buffer Pool
    bool evictPage(int frameIndex);

public:

    // Initializes the Buffer Pool with the given number of frames
    BufferPool(size_t poolSize,
               ReadPageCallback reader,
               WritePageCallback writer);

    // Fetches a database page into RAM
    char* fetchPage(int pageId);

    // Marks a page as modified
    bool markDirty(int pageId);

    // Releases a previously fetched page
    bool unpinPage(int pageId);

    // Writes a specific dirty page back to disk
    bool flushPage(int pageId);

    // Writes all dirty pages back to disk
    bool flushAll();

    // Displays cache hits, misses and eviction statistics
    void printStats() const;

};

#endif