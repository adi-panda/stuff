// -----------------
// TestAllocator.cpp
// -----------------

// https://github.com/google/googletest
// https://github.com/google/googletest/blob/master/docs/reference/assertions.md

// --------
// includes
// --------

#include <algorithm> // count
#include <cstddef>   // ptrdiff_t
#include <string>    // string

#include "gtest/gtest.h"

#include "Allocator.hpp"

using namespace std;

struct A {
    friend bool operator == (const A&, const A&) {
        A::log += "==(A, A) ";
        return true;}

    static string log;

    A  ()         = default;
    A  (int)      {log += "A(int) ";}
    A  (const A&) {log += "A(A) ";}
    ~A ()         {log += "~A() ";}};

string A::log;

TEST(AllocatorFixture, test0) {
    using allocator_type = My_Allocator<A, 1000>;
    using value_type     = typename allocator_type::value_type;
    using size_type      = typename allocator_type::size_type;
    using pointer        = typename allocator_type::pointer;

    allocator_type  x;
    const size_type s = 2;
    const pointer   b = x.allocate(s);
    const pointer   e = b + s;

    const value_type v = 0;
    ASSERT_EQ(A::log, "A(int) ");

    pointer p = b;
    while (p != e) {
        x.construct(p, v);
        ++p;}
    ASSERT_EQ(A::log, "A(int) A(A) A(A) ");

    ASSERT_EQ(count(b, e, v), ptrdiff_t(s));
    ASSERT_EQ(A::log, "A(int) A(A) A(A) ==(A, A) ==(A, A) ");

    p = e;
    while (b != p) {
        --p;
        x.destroy(p);}
    ASSERT_EQ(A::log, "A(int) A(A) A(A) ==(A, A) ==(A, A) ~A() ~A() ");
    x.deallocate(b, s);}

TEST(AllocatorFixture, test1) {
    using allocator_type = My_Allocator<A, 1000>;

    allocator_type x;        // read/write
    ASSERT_EQ(x[  0], 992);
    ASSERT_EQ(x[996], 992);}

TEST(AllocatorFixture, test2) {
    using allocator_type = My_Allocator<A, 1000>;

    const allocator_type x;  // read-only
    ASSERT_EQ(x[  0], 992);
    ASSERT_EQ(x[996], 992);}
