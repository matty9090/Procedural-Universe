#include "gtest/gtest.h"
#include "Services/Log.hpp"

TEST(IndependentMethod, LogInfo)
{
    testing::internal::CaptureStdout();
    LOGM("Testing")
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Info] Testing\n") << "Log output format unexpected";
}

TEST(IndependentMethod, LogWarning)
{
    testing::internal::CaptureStdout();
    LOGW("Testing")
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Warning] Testing\n") << "Log output format unexpected";
}

TEST(IndependentMethod, LogError)
{
    testing::internal::CaptureStdout();
    LOGE("Testing")
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Error] Testing\n") << "Log output format unexpected";
}