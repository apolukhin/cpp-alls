#include "gtest/gtest.h"

#include "cppalls/core/stack_request.hpp"
#include "cppalls/core/stack_response.hpp"
#include <cstring>
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

    req.clear();
    unsigned char c;
    EXPECT_THROW(req >> c, std::exception);
}

template <class T>
void test_insertion(stack_response& resp) {
    T res(0);
    resp << res;
}

TEST(request_response, stack_response) {
    stack_response resp;

    ASSERT_FALSE(resp.end() - resp.begin());
    test_insertion<char>(resp);
    test_insertion<unsigned char>(resp);
    test_insertion<short>(resp);
    test_insertion<unsigned short>(resp);
    test_insertion<int>(resp);
    test_insertion<unsigned int>(resp);
    test_insertion<long long>(resp);
    test_insertion<unsigned long long>(resp);
    test_insertion<float>(resp);
    test_insertion<double>(resp);
    test_insertion<long double>(resp);

    ASSERT_TRUE(resp.end() - resp.begin());
    resp.clear();
    ASSERT_FALSE(resp.end() - resp.begin());
}

template <class T>
void test_and_assert_equality() {
    for (int i = -10; i < 11; ++i) {
        stack_request req;
        stack_response resp;
        ASSERT_FALSE(req.end() - req.begin());
        ASSERT_FALSE(resp.end() - resp.begin());

        T res0(i);
        resp << res0;
        req.resize(resp.end() - resp.begin());
        std::memcpy(req.begin(), resp.begin(), resp.end() - resp.begin());
        T res1;
        req >> res1;
        ASSERT_EQ(res0, res1);
    }
}

TEST(request_response, stack_response_request) {
    test_and_assert_equality<char>();
    test_and_assert_equality<unsigned char>();
    test_and_assert_equality<short>();
    test_and_assert_equality<unsigned short>();
    test_and_assert_equality<int>();
    test_and_assert_equality<unsigned int>();
    test_and_assert_equality<long long>();
    test_and_assert_equality<unsigned long long>();
    test_and_assert_equality<float>();
    test_and_assert_equality<double>();
    test_and_assert_equality<long double>();
}
