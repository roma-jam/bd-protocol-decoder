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
void command_handle(const protocol::Decoder::Command &command) {
  std::cout << "Command ID: " << static_cast<int>(command.command_id) << '\n';
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
static bool is_formatted(const std::string &input) {
  bool pass = false;

  for (const char c : input) {
    if (c == '0' || c == '1') {
      pass = true;
      continue;
    }

    if (c == ' ') {
      continue;
    }

    return false;
  }

  return pass;
}

int main(int argc, char *argv[]) {
  /* Briefly check the arguments */
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <hex-string>\n"
              << "Example: " << argv[0] << " \"1111 1111 0101 1000\"\n";

    return EXIT_FAILURE;
  }

  const std::string input = argv[1];

  if (!is_formatted(input)) {
    std::cerr << "Invalid input. Only '0', '1', and spaces are allowed.\n";

    return EXIT_FAILURE;
  }

  /* Init decoder */
  decoder.init(command_handle);

  // TODO: Decode the string nimble by nimble

  /* Parse next incoming nimble */
  decoder.proceed(0x4);

  std::cout << "String decoding completed\n";

  return EXIT_SUCCESS;
}
