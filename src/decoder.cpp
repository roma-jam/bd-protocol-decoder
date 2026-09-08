// SPDX-FileCopyrightText: 2026 Roman Leonov
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <utility>

namespace protocol {

void Decoder::init(CommandCallback callback) {
  _callback = std::move(callback);
  reset();
}

void Decoder::proceed(std::uint8_t nibble) {
  nibble &= 0x0FU;

  /*
   * Protocol decoding logic will be added here.
   *
   * Once a complete command is decoded:
   *
   * Command command;
   *
   * if (callback_) {
   *     callback_(command);
   * }
   */
}

void Decoder::reset() {
  /*
   * Reset internal decoder state here.
   */
}

} /* namespace protocol */
