#include "gtest/gtest.h"
#include "Services/Log.hpp"

TEST(IndependentMethod, LogInfoImplicit)
{
    testing::internal::CaptureStdout();
    FLog::Get().Log("Testing");
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Info] Testing\n") << "Log output format unexpected";
}

TEST(IndependentMethod, LogInfoExplicit)
{
    testing::internal::CaptureStdout();
    FLog::Get().Log("Testing", FLog::Info);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Info] Testing\n") << "Log output format unexpected";
}

TEST(IndependentMethod, LogWarning)
{
    testing::internal::CaptureStdout();
    FLog::Get().Log("Testing", FLog::Warning);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Warning] Testing\n") << "Log output format unexpected";
}

TEST(IndependentMethod, LogError)
{
    testing::internal::CaptureStdout();
    FLog::Get().Log("Testing", FLog::Error);
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_EQ(output, "[Error] Testing\n") << "Log output format unexpected";
}