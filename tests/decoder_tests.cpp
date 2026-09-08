// SPDX-FileCopyrightText: 2026 Roman Leonov
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <condition_variable>
#include <cstdint>
#include <gtest/gtest.h>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
namespace
{

void test_feed_input(protocol::Decoder& decoder, const std::string& input)
{
        std::uint8_t nibble = 0;
        std::size_t bit_count = 0;

        for (const char bit : input)
        {
                if (bit == ' ')
                {
                        continue;
                }

                nibble <<= 1U;

                if (bit == '1')
                {
                        nibble |= 1U;
                }

                ++bit_count;

                if (bit_count == 4U)
                {
                        decoder.process(nibble);

                        nibble = 0;
                        bit_count = 0;
                }
        }
}

} // namespace

TEST(DecoderTest, InitDoesNotInvokeCallback)
{
        bool called = false;

        protocol::Decoder decoder;

        decoder.init([&](const protocol::Decoder::CommandFrame&) { called = true; });

        EXPECT_FALSE(called);
}

TEST(DecoderTest, DecodesFramesFromWorkerThread)
{
        constexpr const char* input = "1111 1111 0101 1000 1101 0010 0001 0100 0010 0101 "
                                      "1111 1111 1111 1111 1111 1111 "
                                      "0101 1000 1101 1110 0001 0100 0010 1101 "
                                      "1111 1111 1111 "
                                      "0101 1000 1101 1110 0001 0100 0010 1001 "
                                      "1111 1111 1111 1111";

        std::mutex mutex;
        std::condition_variable condition;
        std::queue<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;

        decoder.init(
                [&](const protocol::Decoder::CommandFrame& frame)
                {
                        {
                                std::lock_guard lock(mutex);
                                frames.push(frame);
                        }

                        condition.notify_one();
                });

        std::thread worker([&]() { test_feed_input(decoder, input); });

        std::vector<protocol::Decoder::CommandFrame> received;

        {
                std::unique_lock lock(mutex);

                const bool complete = condition.wait_for(lock, std::chrono::seconds(1),
                                                         [&]() { return frames.size() >= 3; });

                ASSERT_TRUE(complete);

                while (!frames.empty())
                {
                        received.push_back(frames.front());
                        frames.pop();
                }
        }

        worker.join();

        ASSERT_EQ(received.size(), 3U);

        EXPECT_EQ(received[0].frame, 1U);
        EXPECT_EQ(received[0].id, 0x0D);
        EXPECT_EQ(received[0].sender_addr, 0x21);
        EXPECT_EQ(received[0].receiver_addr, 0x42);
        EXPECT_EQ(received[0].err, 0);

        EXPECT_EQ(received[1].frame, 2U);
        EXPECT_EQ(received[1].id, 0x0D);
        EXPECT_EQ(received[1].sender_addr, 0xE1);
        EXPECT_EQ(received[1].receiver_addr, 0x42);
        EXPECT_NE(received[1].err, 0);

        EXPECT_EQ(received[2].frame, 3U);
        EXPECT_EQ(received[2].id, 0x0D);
        EXPECT_EQ(received[2].sender_addr, 0xE1);
        EXPECT_EQ(received[2].receiver_addr, 0x42);
        EXPECT_EQ(received[2].err, 0);
}

TEST(DecoderTest, IgnoresIdleNibblesUntilSyncStart)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        decoder.process(0xF);
        decoder.process(0xF);
        decoder.process(0xA);
        decoder.process(0x0);

        EXPECT_TRUE(frames.empty());

        // Valid frame:
        // 5 ^ 8 ^ D ^ 2 ^ 1 ^ 4 ^ 2 = 5
        decoder.process(0x5);
        decoder.process(0x8);
        decoder.process(0xD);
        decoder.process(0x2);
        decoder.process(0x1);
        decoder.process(0x4);
        decoder.process(0x2);
        decoder.process(0x5);

        ASSERT_EQ(frames.size(), 1U);

        EXPECT_EQ(frames[0].id, 0xD);
        EXPECT_EQ(frames[0].sender_addr, 0x21);
        EXPECT_EQ(frames[0].receiver_addr, 0x42);
        EXPECT_EQ(frames[0].err, 0);
}

TEST(DecoderTest, ResetsAfterInvalidSecondSyncNibble)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        // Start sync, then corrupt it.
        decoder.process(0x5);
        decoder.process(0x7);

        // These must not be interpreted as frame data.
        decoder.process(0xD);
        decoder.process(0x2);
        decoder.process(0x1);
        decoder.process(0x4);
        decoder.process(0x2);
        decoder.process(0x5);

        EXPECT_TRUE(frames.empty());
}

TEST(DecoderTest, AcceptsRepeatedSyncStart)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        decoder.process(0x5);
        decoder.process(0x5);
        decoder.process(0x5);
        decoder.process(0x8);

        decoder.process(0xD);
        decoder.process(0x2);
        decoder.process(0x1);
        decoder.process(0x4);
        decoder.process(0x2);
        decoder.process(0x5);

        ASSERT_EQ(frames.size(), 1U);

        EXPECT_EQ(frames[0].id, 0xD);
        EXPECT_EQ(frames[0].sender_addr, 0x21);
        EXPECT_EQ(frames[0].receiver_addr, 0x42);
        EXPECT_EQ(frames[0].err, 0);
}

TEST(DecoderTest, TrimsUpperBitsOfInput)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        // Upper bits are intentionally set.
        decoder.process(0xF5);
        decoder.process(0xA8);
        decoder.process(0xBD);
        decoder.process(0xC2);
        decoder.process(0xE1);
        decoder.process(0x94);
        decoder.process(0x72);
        decoder.process(0x35);

        ASSERT_EQ(frames.size(), 1U);

        EXPECT_EQ(frames[0].id, 0xD);
        EXPECT_EQ(frames[0].sender_addr, 0x21);
        EXPECT_EQ(frames[0].receiver_addr, 0x42);
        EXPECT_EQ(frames[0].err, 0);
}

TEST(DecoderTest, DoesNotEmitIncompleteFrame)
{
        bool called = false;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame&) { called = true; });

        decoder.process(0x5);
        decoder.process(0x8);
        decoder.process(0xD);
        decoder.process(0x2);
        decoder.process(0x1);

        EXPECT_FALSE(called);
}

TEST(DecoderTest, EmitsFrameWithChecksumError)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        decoder.process(0x5);
        decoder.process(0x8);
        decoder.process(0xD);
        decoder.process(0xE);
        decoder.process(0x1);
        decoder.process(0x4);
        decoder.process(0x2);

        // Expected checksum is 0x9, deliberately provide 0xD.
        decoder.process(0xD);

        ASSERT_EQ(frames.size(), 1U);

        EXPECT_EQ(frames[0].id, 0xD);
        EXPECT_EQ(frames[0].sender_addr, 0xE1);
        EXPECT_EQ(frames[0].receiver_addr, 0x42);
        EXPECT_NE(frames[0].err, 0);
}

TEST(DecoderTest, DecodesBackToBackFrames)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        // Frame 1
        const std::uint8_t frame1[] = {0x5, 0x8, 0xD, 0x2, 0x1, 0x4, 0x2, 0x5};

        // Frame 2 immediately follows, no idle nibbles.
        const std::uint8_t frame2[] = {0x5, 0x8, 0xD, 0xE, 0x1, 0x4, 0x2, 0x9};

        for (const auto nibble : frame1)
        {
                decoder.process(nibble);
        }

        for (const auto nibble : frame2)
        {
                decoder.process(nibble);
        }

        ASSERT_EQ(frames.size(), 2U);

        EXPECT_EQ(frames[0].frame, 1U);
        EXPECT_EQ(frames[0].err, 0);

        EXPECT_EQ(frames[1].frame, 2U);
        EXPECT_EQ(frames[1].err, 0);
}

TEST(DecoderTest, InitFlushesPartialFrame)
{
        std::vector<protocol::Decoder::CommandFrame> frames;

        protocol::Decoder decoder;

        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        // Begin a frame but stop halfway.
        decoder.process(0x5);
        decoder.process(0x8);
        decoder.process(0xD);
        decoder.process(0x2);

        // Reinitialize. Partial frame should be discarded.
        decoder.init([&](const protocol::Decoder::CommandFrame& frame)
                     { frames.push_back(frame); });

        // Complete valid frame.
        const std::uint8_t frame[] = {0x5, 0x8, 0xD, 0x2, 0x1, 0x4, 0x2, 0x5};

        for (const auto nibble : frame)
        {
                decoder.process(nibble);
        }

        ASSERT_EQ(frames.size(), 1U);

        EXPECT_EQ(frames[0].id, 0xD);
        EXPECT_EQ(frames[0].sender_addr, 0x21);
        EXPECT_EQ(frames[0].receiver_addr, 0x42);
}
