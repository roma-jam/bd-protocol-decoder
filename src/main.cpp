// SPDX-FileCopyrightText: 2026 Roman Leonov
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <iostream>

protocol::Decoder decoder;

/**
 * @brief Command decode callback
 *
 * HINT: Suppose to be fast and non-blocking.
 */
void command_handle(const protocol::Decoder::CommandFrame& command)
{
        std::cout << "Frame: " << static_cast<unsigned int>(command.frame) << ", ";

        /*
         * Sync verification is done inside the decoder logic.
         * When we have the callback -> sync is OK.
         * Add the output to match the expected output according the task.
         */
        std::cout << "Sync: OK, ";

        std::cout << "Command: 0x" << std::hex << static_cast<unsigned int>(command.id) << ", ";

        std::cout << "Sender: 0x" << static_cast<unsigned int>(command.sender_addr) << ", ";

        std::cout << "Receiver: 0x" << static_cast<unsigned int>(command.receiver_addr) << ", ";

        std::cout << "Checksum: " << ((command.err == 0) ? "OK" : "NOK") << '\n';
}

/**
 * @brief Checks the format of an input string
 *
 * The input may contain binary digits ('0' and '1') and space characters.
 * Spaces are ignored by the parser and may be used to group bits for
 * readability.
 *
 * @param input Input string containing the bit representation of the data.
 *
 * @return true if the input contains at least one bit and consists only of
 *         '0', '1', and space characters.
 * @return false otherwise.
 */
static bool input_is_formatted(const std::string& input)
{
        bool pass = false;

        for (const char c : input)
        {
                if (c == '0' || c == '1')
                {
                        pass = true;
                        continue;
                }

                if (c == ' ')
                {
                        continue;
                }

                return false;
        }

        return pass;
}

/**
 * @brief Processes a binary input string nibble-by-nibble with decoder.
 *
 * Spaces are ignored. Every group of four bits is converted into a nibble
 * and passed to the decoder from left to right.
 *
 * @param input Input string containing '0', '1', and optional spaces.
 * @param decoder Decoder instance receiving parsed nibbles.
 *
 * @return true if the complete input was processed successfully.
 * @return false if the input contains an invalid character or an incomplete
 *         final nibble.
 */
bool input_decode(const std::string& input, protocol::Decoder& decoder)
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
                        /* Process nibble */
                        decoder.process(nibble);

                        nibble = 0;
                        bit_count = 0;
                }
        }

        return bit_count == 0U;
}

int main(int argc, char* argv[])
{
        /* Briefly check the arguments, we need string input */
        if (argc != 2)
        {
                std::cerr << "Usage: " << argv[0] << " <hex-string>\n"
                          << "Example: " << argv[0] << " \"1111 1111\"\n";

                return EXIT_FAILURE;
        }

        const std::string input = argv[1];

        if (!input_is_formatted(input))
        {
                std::cerr << "Invalid input. '0', '1', and spaces are allowed.\n";

                return EXIT_FAILURE;
        }

        /* Init decoder */
        decoder.init(command_handle);

        /* Decode the input */
        if (!input_decode(input, decoder))
        {
                std::cerr << "Invalid input bit string\n";
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}
