#include <iostream>
#include <sstream>
#include <vector>

#include "Allocator.hpp"

using namespace std;

int main() {
  using MemoryAllocator = My_Allocator<double, 1000>;
  using AllocatorPtr = MemoryAllocator::pointer;
  using AllocatorIter = MemoryAllocator::iterator;

  int numTestCases;
  cin >> numTestCases;
  cin.ignore();  // Consume the newline
  bool isFirstTestCase = true;

  while (numTestCases--) {
    MemoryAllocator memAllocator;
    vector<AllocatorPtr> activeAllocations;
    string inputLine;

    if (isFirstTestCase) {
      getline(cin, inputLine);  // Read the blank line
      isFirstTestCase = false;
    }

    // Process allocation/deallocation requests
    while (getline(cin, inputLine) && !inputLine.empty()) {
      int memoryRequest;
      istringstream inputStream(inputLine);
      inputStream >> memoryRequest;

      if (memoryRequest > 0) {
        // Handle allocation request
        try {
          AllocatorPtr allocatedMemory = memAllocator.allocate(memoryRequest);
          activeAllocations.push_back(allocatedMemory);
        } catch (const bad_alloc& e) {
          cout << "Allocation failed: " << e.what() << endl;
        }
      } else if (memoryRequest < 0) {
        // Handle deallocation request
        size_t deallocationIndex = -memoryRequest - 1;

        if (deallocationIndex < activeAllocations.size()) {
          AllocatorPtr memoryToFree = 0;
          AllocatorIter currentBlock = memAllocator.begin();
          AllocatorIter lastBlock = memAllocator.end();

          // Find the block to deallocate
          size_t busyBlockCount = 0;
          while (currentBlock != lastBlock) {
            if (*currentBlock < 0) {  // Block is in use
              if (busyBlockCount == deallocationIndex) {
                memoryToFree =
                    reinterpret_cast<AllocatorPtr>(&*currentBlock + 1);
                break;
              }
              ++busyBlockCount;
            }
            ++currentBlock;
          }

          memAllocator.deallocate(memoryToFree, 0);
          activeAllocations.erase(activeAllocations.begin() +
                                  deallocationIndex);
        }
      }
    }

    // Print final memory state
    AllocatorIter currentBlock = memAllocator.begin();
    AllocatorIter lastBlock = memAllocator.end();

    if (currentBlock != lastBlock) {
      cout << *currentBlock;
      ++currentBlock;

      while (currentBlock != lastBlock) {
        cout << " " << *currentBlock;
        ++currentBlock;
      }
    }
    cout << endl;
  }
  return 0;
}
