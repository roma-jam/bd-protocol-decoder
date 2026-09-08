// SPDX-FileCopyrightText: 2026 Roman
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace protocol
{

/**
 * @brief Streaming command decoder.
 */
class Decoder
{
      public:
        /**
         * @brief Command data structure.
         */
        struct CommandFrame
        {
                std::uint32_t frame;
                std::uint8_t id;
                std::uint8_t sender_addr;
                std::uint8_t receiver_addr;
                int err;
        };

        using CommandCallback = std::function<void(const CommandFrame&)>;

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
        void process(std::uint8_t nibble);

        /**
         * @brief Resets the internal decoder state.
         */
        void reset_state();

      private:
        /**
         * @brief Decoder state
         */
        enum class State
        {
                idle,
                sync,
                data,
                checksum
        };

        struct CommandData
        {
                /* Data size description */
                static constexpr std::size_t id_bits = 4;
                static constexpr std::size_t sender_addr_bits = 8;
                static constexpr std::size_t receiver_addr_bits = 8;

                /* Data holders */
                std::uint8_t id;
                std::uint8_t sender_addr;
                std::uint8_t receiver_addr;

                /* Verification members */
                static constexpr std::size_t bit_count =
                        id_bits + sender_addr_bits + receiver_addr_bits;
                /* Amount of nibbles to fill the data holders */
                static constexpr std::size_t nibble_count = bit_count / 4;
        };

        void handle_data(std::uint8_t nibble);

        State state_ = State::idle;

        /* Command data holder */
        CommandData data_{};

        // TODO: Might be optimized, but keep them separate for now
        std::uint8_t checksum_ = 0;
        std::uint8_t nibble_index_ = 0;
        std::uint32_t frame_num_ = 0;

        CommandCallback callback_;
};

} // namespace protocol
