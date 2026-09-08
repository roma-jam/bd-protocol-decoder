// SPDX-FileCopyrightText: 2026 Roman Leonov
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <cassert>
#include <iostream>
#include <utility>

namespace protocol
{
namespace
{

constexpr std::uint8_t SYNC_START = 0x5;
constexpr std::uint8_t SYNC_DONE = 0x8;

} // namespace

void Decoder::init(CommandCallback callback)
{
        callback_ = std::move(callback);
        frame_num_ = 0;
        reset_state();
}

void Decoder::handle_data(std::uint8_t nibble)
{
        checksum_ ^= nibble;

        // TODO: Might be optimized and adopt variable data len
        // but keep it simple for now
        switch (nibble_index_)
        {
        case 0:
                data_.id = nibble;
                break;

        case 1:
                data_.sender_addr = static_cast<std::uint8_t>(nibble << 4U);
                break;

        case 2:
                data_.sender_addr |= nibble;
                break;

        case 3:
                data_.receiver_addr = static_cast<std::uint8_t>(nibble << 4U);
                break;

        case 4:
                data_.receiver_addr |= nibble;
                break;

        default:
                reset_state();
                return;
        }

        ++nibble_index_;

        if (nibble_index_ == CommandData::nibble_count)
        { /* Enough, let's wait for checksum */
                state_ = State::checksum;
        }
}

void Decoder::process(std::uint8_t nibble)
{
        /* Trim the nibble */
        nibble &= 0x0FU;

        switch (state_)
        {
        case State::idle:
                /* Idle data is 1s, therefore normally we receive 0xF */
                if (nibble != SYNC_START)
                {
                        /* If not SYNC_START nibble */
                        return;
                }
                checksum_ = nibble;
                state_ = State::sync;
                break;

        case State::sync:
                if (nibble == SYNC_DONE)
                {
                        checksum_ ^= nibble;
                        nibble_index_ = 0;
                        // TODO: frame overflow?
                        frame_num_++;
                        state_ = State::data;
                        return;
                }

                // TODO: Might be sync start once again?
                if (nibble == SYNC_START)
                {
                        checksum_ = nibble;
                        return;
                }

                /* Everything else - reset the state and wait next nibble */
                reset_state();
                break;

        case State::data:
                handle_data(nibble);
                break;

        case State::checksum:
                assert(callback_);

                CommandFrame command;

                command.frame = frame_num_;
                command.id = data_.id;
                command.sender_addr = data_.sender_addr;
                command.receiver_addr = data_.receiver_addr;
                // TODO: Refine error logic
                command.err = checksum_ ^ nibble;

                callback_(command);

                reset_state();
                break;
        }
}

void Decoder::reset_state()
{
        state_ = State::idle;

        data_ = {};
        checksum_ = 0;
        nibble_index_ = 0;
}

} /* namespace protocol */
