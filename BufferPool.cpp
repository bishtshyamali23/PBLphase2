#include "BufferPool.h"
#include <stdexcept>
#include <utility>

// Constructor
BufferPool::BufferPool(size_t poolSize,
                       ReadPageCallback reader,
                       WritePageCallback writer) {

    if (poolSize == 0) {
        throw invalid_argument("Buffer Pool size cannot be zero");
    }

    if (!reader || !writer) {
        throw invalid_argument("Page Manager callbacks cannot be empty");
    }

    frames.resize(poolSize);

    readPageFromDisk = move(reader);
    writePageToDisk = move(writer);
}


// Moves the recently used page to the front of the LRU list
void BufferPool::updateLRU(int pageId) {

    auto it = lruPosition.find(pageId);

    if (it != lruPosition.end()) {
        lruList.erase(it->second);
    }

    lruList.push_front(pageId);
    lruPosition[pageId] = lruList.begin();
}


// Finds an empty frame or removes the least recently used page
int BufferPool::findFreeFrame() {

    // Check for an empty frame
    for (int i = 0; i < frames.size(); i++) {

        if (!frames[i].isValid) {
            return i;
        }
    }

    // Find the least recently used page
    for (auto it = lruList.rbegin(); it != lruList.rend(); it++) {

        int pageId = *it;
        auto page = pageTable.find(pageId);

        if (page == pageTable.end()) {
            continue;
        }

        int frameIndex = page->second;

        // Pinned pages cannot be removed
        if (frames[frameIndex].pinCount == 0) {

            if (evictPage(frameIndex)) {
                return frameIndex;
            }
        }
    }

    return -1;
}


// Removes a page from the Buffer Pool
bool BufferPool::evictPage(int frameIndex) {

    if (frameIndex < 0 || frameIndex >= frames.size()) {
        return false;
    }

    BufferFrame& frame = frames[frameIndex];

    if (!frame.isValid || frame.pinCount > 0) {
        return false;
    }

    // Save the page if it was modified
    if (frame.isDirty) {

        if (!writePageToDisk(frame.pageId, frame.data)) {
            return false;
        }
    }

    // Remove the page from the Page Table
    pageTable.erase(frame.pageId);

    // Remove the page from the LRU list
    auto it = lruPosition.find(frame.pageId);

    if (it != lruPosition.end()) {
        lruList.erase(it->second);
        lruPosition.erase(it);
    }

    // Reset the frame
    frame.pageId = -1;
    frame.isDirty = false;
    frame.isValid = false;
    frame.pinCount = 0;

    evictions++;

    return true;
}


// Fetches a page from the database into memory
char* BufferPool::fetchPage(int pageId) {

    if (pageId < 0) {
        return nullptr;
    }

    lock_guard<mutex> lock(poolMutex);

    // Check whether the page is already in memory
    auto it = pageTable.find(pageId);

    if (it != pageTable.end()) {

        // Page found: Cache Hit
        cacheHits++;

        int frameIndex = it->second;

        frames[frameIndex].pinCount++;

        updateLRU(pageId);

        return frames[frameIndex].data;
    }

    // Page not found: Cache Miss
    cacheMisses++;

    int frameIndex = findFreeFrame();

    if (frameIndex == -1) {
        return nullptr;
    }

    BufferFrame& frame = frames[frameIndex];

    // Read the page from disk
    if (!readPageFromDisk(pageId, frame.data)) {
        frame.pageId = -1;
        frame.isDirty = false;
        frame.isValid = false;
        frame.pinCount = 0;

        return nullptr;
    }

    // Store the page in the frame
    frame.pageId = pageId;
    frame.isDirty = false;
    frame.isValid = true;
    frame.pinCount = 1;

    pageTable[pageId] = frameIndex;

    updateLRU(pageId);

    return frame.data;
}


// Marks a page as modified
bool BufferPool::markDirty(int pageId) {

    lock_guard<mutex> lock(poolMutex);

    auto it = pageTable.find(pageId);

    if (it == pageTable.end()) {
        return false;
    }

    frames[it->second].isDirty = true;

    return true;
}


// Releases a page after its use
bool BufferPool::unpinPage(int pageId) {

    lock_guard<mutex> lock(poolMutex);

    auto it = pageTable.find(pageId);

    if (it == pageTable.end()) {
        return false;
    }

    BufferFrame& frame = frames[it->second];

    if (frame.pinCount <= 0) {
        return false;
    }

    frame.pinCount--;

    return true;
}


// Writes one dirty page to disk
bool BufferPool::flushPage(int pageId) {

    lock_guard<mutex> lock(poolMutex);

    auto it = pageTable.find(pageId);

    if (it == pageTable.end()) {
        return false;
    }

    BufferFrame& frame = frames[it->second];

    if (!frame.isDirty) {
        return true;
    }

    if (!writePageToDisk(pageId, frame.data)) {
        return false;
    }

    frame.isDirty = false;

    return true;
}


// Writes all dirty pages to disk
bool BufferPool::flushAll() {

    lock_guard<mutex> lock(poolMutex);

    for (auto& frame : frames) {

        if (frame.isValid && frame.isDirty) {

            if (!writePageToDisk(frame.pageId, frame.data)) {
                return false;
            }

            frame.isDirty = false;
        }
    }

    return true;
}


// Displays Buffer Pool statistics
void BufferPool::printStats() const {

    lock_guard<mutex> lock(poolMutex);

    cout << "\nBuffer Pool Statistics\n";

    cout << "Cache Hits: " << cacheHits << endl;
    cout << "Cache Misses: " << cacheMisses << endl;
    cout << "Evictions: " << evictions << endl;

    size_t total = cacheHits + cacheMisses;

    if (total > 0) {
        double hitRate = (double)cacheHits / total * 100;

        cout << "Cache Hit Rate: " << hitRate << "%" << endl;
    }
    else {
        cout << "Cache Hit Rate: 0%" << endl;
    }
}