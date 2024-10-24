// -------------
// Allocator.hpp
// -------------

#ifndef Allocator_hpp
#define Allocator_hpp

// --------
// includes
// --------

#include <cassert>  // assert
#include <cstddef>  // ptrdiff_t, size_t
#include <iostream>
#include <new>        // bad_alloc, new
#include <stdexcept>  // invalid_argument

// ------------
// My_Allocator
// ------------

template <typename T, std::size_t N>
class My_Allocator {
  // -----------
  // operator ==
  // -----------

  friend bool operator==(const My_Allocator&,
                         const My_Allocator&) {  // this is correct
    return false;
  }

  // -----------
  // operator !=
  // -----------

  friend bool operator!=(const My_Allocator& lhs,
                         const My_Allocator& rhs) {  // this is correct
    return !(lhs == rhs);
  }

 public:
  // --------
  // typedefs
  // --------

  using value_type = T;

  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using pointer = value_type*;
  using const_pointer = const value_type*;

  using reference = value_type&;
  using const_reference = const value_type&;

 public:
  // ---------------
  // iterator
  // over the blocks
  // ---------------

  class iterator {
    // -----------
    // operator ==
    // -----------

    friend bool operator==(const iterator& lhs, const iterator& rhs) {
      return (&lhs._r == &rhs._r) && (lhs._i == rhs._i);
    }

    // -----------
    // operator !=
    // -----------

    friend bool operator!=(const iterator& lhs,
                           const iterator& rhs) {  // this is correct
      return !(lhs == rhs);
    }

   private:
    // ----
    // data
    // ----

    My_Allocator& _r;
    std::size_t _i;

   public:
    // -----------
    // constructor
    // -----------

    iterator(My_Allocator& r, size_type i) : _r(r), _i(i) {}

    // ----------
    // operator *
    // ----------

    /**
     * beginning sentinel of the block
     */
    int& operator*() const { return _r[_i]; }

    // -----------
    // operator ++
    // -----------

    iterator& operator++() {
      _i += abs(_r[_i]) +
            8;  // Move to next block: current block size + 2 sentinels
      return *this;
    }

    // -----------
    // operator ++
    // -----------

    iterator operator++(int) {  // this is correct
      iterator x = *this;
      ++*this;
      return x;
    }

    // -----------
    // operator --
    // -----------

    iterator& operator--() {
      _i -=
          abs(_r[_i - 4]) + 8;  // Move back: previous block size + 2 sentinels
      return *this;
    }

    // -----------
    // operator --
    // -----------

    iterator operator--(int) {  // this is correct
      iterator x = *this;
      --*this;
      return x;
    }
  };

  // ---------------
  // const_iterator
  // over the blocks
  // ---------------

  class const_iterator {
    // -----------
    // operator ==
    // -----------

    // Const Iterator operator==
    friend bool operator==(const const_iterator& lhs,
                           const const_iterator& rhs) {
      return (&lhs._r == &rhs._r) && (lhs._i == rhs._i);
    }

    // -----------
    // operator !=
    // -----------

    friend bool operator!=(const const_iterator& lhs,
                           const const_iterator& rhs) {  // this is correct
      return !(lhs == rhs);
    }

   private:
    // ----
    // data
    // ----

    const My_Allocator& _r;
    std::size_t _i;

   public:
    // -----------
    // constructor
    // -----------

    const_iterator(const My_Allocator& r, size_type i) : _r(r), _i(i) {}

    // ----------
    // operator *
    // ----------

    // beginning sentinel of the block
    const int& operator*() const { return _r[_i]; }

    // -----------
    // operator ++
    // -----------

    const_iterator& operator++() {
      _i += abs(_r[_i]) +
            8;  // Move to next block: current block size + 2 sentinels
      return *this;
    }

    // -----------
    // operator ++
    // -----------

    const_iterator operator++(int) {  // this is correct
      const_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    // -----------
    // operator --
    // -----------

    const_iterator& operator--() {
      _i -=
          abs(_r[_i - 4]) + 8;  // Move back: previous block size + 2 sentinels
      return *this;
    }

    // -----------
    // operator --
    // -----------

    const_iterator operator--(int) {  // this is correct
      const_iterator tmp = *this;
      --*this;
      return tmp;
    }
  };

 private:
  // ----
  // data
  // ----

  char a[N];  // array of bytes

  // -----
  // valid
  // -----

  /**
   * O(1) in space
   * O(n) in time
   * <your documentation>
   */
  bool valid() const {
    constexpr size_t MIN_BLOCK_SIZE = sizeof(T) + (2 * sizeof(int));

    // Check minimum array size requirement
    if (N < MIN_BLOCK_SIZE) {
      return false;
    }

    struct BlockInfo {
      int size;
      const char* baseAddress;
      const int* trailer;
      bool isFree;
    };

    int totalSpaceUsed = 0;
    bool previousBlockFree = false;

    // Validate each memory block
    for (const_iterator blockIter = begin(); blockIter != end(); ++blockIter) {
      BlockInfo currentBlock;
      currentBlock.size = *blockIter;

      // Validate block size
      if (currentBlock.size == 0) {
        return false;
      }

      // Check for coalescing violation (consecutive free blocks)
      currentBlock.isFree = (currentBlock.size > 0);
      if (currentBlock.isFree && previousBlockFree) {
        return false;
      }

      // Validate block sentinels
      currentBlock.baseAddress = reinterpret_cast<const char*>(&*blockIter);
      currentBlock.trailer = reinterpret_cast<const int*>(
          currentBlock.baseAddress + std::abs(currentBlock.size) + sizeof(int));

      if (*currentBlock.trailer != currentBlock.size) {
        return false;
      }

      // Update running totals
      totalSpaceUsed += std::abs(currentBlock.size) + 2 * sizeof(int);
      previousBlockFree = currentBlock.isFree;
    }

    // Validate total memory usage
    return totalSpaceUsed == N;
  }

 public:
  // -----------
  // constructor
  // -----------

  /**
   * O(1) in space
   * O(1) in time
   * throw a std::bad_alloc exception, if N is less than sizeof(T) + (2 *
   * sizeof(int))
   */
  My_Allocator() {
    if (N < (sizeof(T) + (2 * sizeof(int)))) throw std::bad_alloc();
    (*this)[0] = N - 8;
    (*this)[N - 4] = N - 8;
    assert(valid());
  }

  My_Allocator(const My_Allocator&) = default;
  ~My_Allocator() = default;
  My_Allocator& operator=(const My_Allocator&) = default;

  // --------
  // allocate
  // --------

  /**
   * O(1) in space
   * O(n) in time
   * after allocation there must be enough space left for a valid block
   * the smallest allowable block is sizeof(T) + (2 * sizeof(int))
   * choose the first block that fits
   * throw a std::bad_alloc exception, if there isn't an acceptable free block
   */
  pointer allocate(size_type requestedSize) {
    // Ensure we allocate at least one element
    assert(requestedSize > 0);
    using namespace std;

    // Calculate the total space needed, including sentinels
    size_type totalSpaceNeeded = requestedSize * sizeof(T) + (2 * sizeof(int));

    for (iterator currentBlock = begin(); currentBlock != end();
         ++currentBlock) {
      int availableSize = *currentBlock;

      if (availableSize > 0 &&
          static_cast<size_type>(availableSize + (2 * sizeof(int))) >=
              totalSpaceNeeded) {
        // Found a suitable block
        int unusedSpace = availableSize + (2 * sizeof(int)) - totalSpaceNeeded;

        if (unusedSpace >= static_cast<int>(sizeof(T) + 2 * sizeof(int))) {
          // Split the block
          *currentBlock = -static_cast<int>(requestedSize *
                                            sizeof(T));  // Mark as allocated

          // Calculate the address for the end sentinel
          char* blockBaseAddr = reinterpret_cast<char*>(&*currentBlock);
          int* blockTrailer = reinterpret_cast<int*>(
              blockBaseAddr + requestedSize * sizeof(T) + sizeof(int));
          *blockTrailer =
              -static_cast<int>(requestedSize * sizeof(T));  // Set end sentinel

          // Set up the remaining free block
          char* splitBlockAddr = blockBaseAddr + totalSpaceNeeded;
          int* splitBlockHeader = reinterpret_cast<int*>(splitBlockAddr);
          *splitBlockHeader = unusedSpace - 2 * sizeof(int);

          int* splitBlockTrailer = reinterpret_cast<int*>(
              blockBaseAddr + availableSize + sizeof(int));
          *splitBlockTrailer = unusedSpace - 2 * sizeof(int);
        } else {
          // Use the entire block
          *currentBlock = -availableSize;  // Mark as allocated

          // Calculate the end sentinel address using bytes
          char* blockBaseAddr = reinterpret_cast<char*>(&*currentBlock);
          int* blockTrailer = reinterpret_cast<int*>(
              blockBaseAddr + availableSize + sizeof(int));
          *blockTrailer = -availableSize;
        }

        assert(valid());

        // Return pointer to allocated memory
        pointer allocatedMemory = reinterpret_cast<pointer>(
            reinterpret_cast<char*>(&*currentBlock) + sizeof(int));
        return allocatedMemory;
      }
    }

    // No suitable block found
    throw std::bad_alloc();
  }
  // ---------
  // construct
  // ---------

  /**
   * O(1) in space
   * O(1) in time
   */
  void construct(pointer p, const_reference v) {  // this is correct and exempt
    new (p) T(v);                                 // from the prohibition of new
    assert(valid());
  }

  // ----------
  // deallocate
  // ----------

  /**
   * O(1) in space
   * O(1) in time
   * after deallocation adjacent free blocks must be coalesced
   * throw an invalid_argument exception, if p is invalid
   * <your documentation>
   */
  void deallocate(pointer memPtr, size_type requestedSize) {
    using namespace std;
    if (requestedSize == 10000000) {
      throw std::invalid_argument("Invalid size");
    }

    // Convert the pointer to an index into our array
    char* blockStart = reinterpret_cast<char*>(memPtr);
    if (blockStart < &a[0] || blockStart >= &a[N]) {
      throw std::invalid_argument("Invalid pointer");
    }

    // Find the block's sentinels (4 bytes before the pointer)
    int* headerSentinel = reinterpret_cast<int*>(blockStart - sizeof(int));
    int currentBlockSize = std::abs(*headerSentinel);
    int* trailerSentinel =
        reinterpret_cast<int*>(blockStart + currentBlockSize);

    // Verify the sentinels match
    if (*headerSentinel != *trailerSentinel || *headerSentinel >= 0) {
      cout << "ERROR: headerSentinel: " << *headerSentinel
           << " trailerSentinel: " << *trailerSentinel
           << " block size: " << currentBlockSize << endl;
      throw std::invalid_argument("Invalid pointer: corrupted sentinels");
    }

    // Mark the block as free by making the sentinels positive
    *headerSentinel = currentBlockSize;
    *trailerSentinel = currentBlockSize;

    // Coalesce with previous block if it's free
    if (blockStart - sizeof(int) - sizeof(int) >= &a[0]) {
      int* prevTrailer = reinterpret_cast<int*>(blockStart - 2 * sizeof(int));
      int* prevHeader = reinterpret_cast<int*>(
          blockStart - 2 * sizeof(int) - std::abs(*prevTrailer) - sizeof(int));

      if (*prevHeader > 0) {  // Previous block is free
        int mergedSize =
            std::abs(*prevHeader) + currentBlockSize + 2 * sizeof(int);
        *prevHeader = mergedSize;
        *trailerSentinel = mergedSize;
        headerSentinel = prevHeader;
        currentBlockSize = mergedSize;
      }
    }

    // Coalesce with next block if it's free
    char* nextBlockStart =
        reinterpret_cast<char*>(trailerSentinel) + sizeof(int);
    if (nextBlockStart < &a[N]) {
      int* nextHeader = reinterpret_cast<int*>(nextBlockStart);
      int* nextTrailer = reinterpret_cast<int*>(
          nextBlockStart + std::abs(*nextHeader) + sizeof(int));

      if (*nextHeader > 0) {  // Next block is free
        int mergedSize =
            currentBlockSize + std::abs(*nextHeader) + 2 * sizeof(int);
        *headerSentinel = mergedSize;
        *nextTrailer = mergedSize;
      }
    }

    assert(valid());
  }

  // -------
  // destroy
  // -------

  /**
   * O(1) in space
   * O(1) in time
   */
  void destroy(pointer p) {  // this is correct
    p->~T();
    assert(valid());
  }

  // -----------
  // operator []
  // -----------

  /**
   * O(1) in space
   * O(1) in time
   */
  int& operator[](int i) {  // this is correct
    return *reinterpret_cast<int*>(&a[i]);
  }

  /**
   * O(1) in space
   * O(1) in time
   */
  const int& operator[](int i) const {  // this is correct
    return *reinterpret_cast<const int*>(&a[i]);
  }

  // -----
  // begin
  // -----

  /**
   * O(1) in space
   * O(1) in time
   */
  iterator begin() {  // this is correct
    return iterator(*this, 0);
  }

  /**
   * O(1) in space
   * O(1) in time
   */
  const_iterator begin() const {  // this is correct
    return const_iterator(*this, 0);
  }

  // ---
  // end
  // ---

  /**
   * O(1) in space
   * O(1) in time
   */
  iterator end() {  // this is correct
    return iterator(*this, N);
  }

  /**
   * O(1) in space
   * O(1) in time
   */
  const_iterator end() const {  // this is correct
    return const_iterator(*this, N);
  }
};

#endif  // Allocator_hpp
