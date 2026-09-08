// SPDX-FileCopyrightText: 2026 Roman
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace protocol {

/**
 * @brief Streaming command decoder.
 */
class Decoder {
public:
  /**
   * @brief Represents a successfully decoded command.
   */
  struct Command {
    std::uint8_t command_id{};
    std::vector<std::uint8_t> payload;
  };

  using CommandCallback = std::function<void(const Command &)>;

  /**
   * @brief Initializes the decoder.
   *
   * Registers the callback and resets the internal decoder state.
   *
   * @param callback Callback invoked when a command is decoded successfully.
   */
  void init(CommandCallback callback);

  /**
   * @brief Processes the next input nibble.
   *
   * @param nibble Next 4-bit value from the input stream.
   */
  void proceed(std::uint8_t nibble);

private:
  /**
   * @brief Resets the internal decoder state.
   */
  void reset();

  CommandCallback _callback;
};

} // namespace protocol
