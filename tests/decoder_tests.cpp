// SPDX-FileCopyrightText: 2026 Roman Leonov
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <gtest/gtest.h>

TEST(DecoderTest, InitDoesNotInvokeCallback) {
  bool called = false;

  protocol::Decoder d;

  d.init([&](const protocol::Decoder::Command &) { called = true; });

  EXPECT_FALSE(called);
}

TEST(DecoderTest, CanAcceptNibble) {
  protocol::Decoder d;

  d.init([](const protocol::Decoder::Command &) {});

  EXPECT_NO_THROW(d.proceed(0x0A));
}

TEST(DecoderTest, InitResetsDecoderState) {
  protocol::Decoder d;

  d.init([](const protocol::Decoder::Command &) {});

  d.proceed(0x0A);
  d.proceed(0x0B);

  EXPECT_NO_THROW(d.init([](const protocol::Decoder::Command &) {}));
}
