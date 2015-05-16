#include "gtest/gtest.h"

#include "cppalls/core/stack_request.hpp"
using namespace cppalls;

template <class T>
void test_and_assert_zero(stack_request& req) {
    T res;
    req >> res;
    ASSERT_EQ(res, T(0));
}

TEST(request_response, stack_request) {
    stack_request req;
    req.resize(1000);
    std::fill(req.begin(), req.end(), '\0');

    test_and_assert_zero<char>(req);
    test_and_assert_zero<unsigned char>(req);
    test_and_assert_zero<short>(req);
    test_and_assert_zero<unsigned short>(req);
    test_and_assert_zero<int>(req);
    test_and_assert_zero<unsigned int>(req);
    test_and_assert_zero<long long>(req);
    test_and_assert_zero<unsigned long long>(req);
    test_and_assert_zero<float>(req);
    test_and_assert_zero<double>(req);
    test_and_assert_zero<long double>(req);
}
