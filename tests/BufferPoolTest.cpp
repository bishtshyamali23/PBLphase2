#include "../BufferPool.h"
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

int main() {

    // Simulate database pages stored on disk
    vector<vector<char>> diskPages(5, vector<char>(4096, 0));

    // Function to read a page from disk
    auto reader = [&](int pageId, char* data) {

        if (pageId < 0 || pageId >= diskPages.size()) {
            return false;
        }

        memcpy(data, diskPages[pageId].data(), 4096);

        return true;
    };

    // Function to write a page to disk
    auto writer = [&](int pageId, const char* data) {

        if (pageId < 0 || pageId >= diskPages.size()) {
            return false;
        }

        memcpy(diskPages[pageId].data(), data, 4096);

        return true;
    };

    // Create a Buffer Pool with 2 frames
    BufferPool pool(2, reader, writer);

    cout << "Buffer Pool Testing Started\n\n";

    // Fetch page 0
    cout << "Fetching Page 0\n";

    char* page0 = pool.fetchPage(0);

    if (page0 != nullptr) {
        cout << "Page 0 loaded successfully\n";
        pool.unpinPage(0);
    }

    // Fetch page 1
    cout << "\nFetching Page 1\n";

    char* page1 = pool.fetchPage(1);

    if (page1 != nullptr) {
        cout << "Page 1 loaded successfully\n";
        pool.unpinPage(1);
    }

    // Fetch page 0 again to test cache hit
    cout << "\nFetching Page 0 Again\n";

    page0 = pool.fetchPage(0);

    if (page0 != nullptr) {
        cout << "Page 0 found in Buffer Pool\n";
        pool.unpinPage(0);
    }

    // Fetch page 2 to test LRU eviction
    cout << "\nFetching Page 2\n";

    char* page2 = pool.fetchPage(2);

    if (page2 != nullptr) {
        cout << "Page 2 loaded successfully\n";
    }

    // Modify page 2
    if (page2 != nullptr) {

        page2[0] = 'A';

        pool.markDirty(2);

        cout << "Page 2 modified successfully\n";

        pool.unpinPage(2);
    }

    // Write all dirty pages to disk
    cout << "\nFlushing All Pages\n";

    if (pool.flushAll()) {
        cout << "All dirty pages written successfully\n";
    }

    // Check whether modified data reached disk
    cout << "\nChecking Disk Data\n";

    cout << "First character of Page 2: "
         << diskPages[2][0] << endl;

    // Display Buffer Pool statistics
    pool.printStats();

    cout << "\nBuffer Pool Testing Completed\n";

    return 0;
}