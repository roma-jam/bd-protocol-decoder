# BD - Protocol Decoder

A small C++20 application for decoding protocol frames from a stream of bits.

The decoder processes the input bit-by-bit from left to right, detects frame
synchronization, extracts the frame fields, and validates the checksum.

## Frame Structure

| Field | Sync | Command | Sender Addr. | Receiver Addr. | Checksum |
|---|---:|---:|---:|---:|---|
| Size | 8 bits | 4 bits | 8 bits | 8 bits | 4 bits |
| Description | `0101 1000` | Command | Sender address | Receiver address | XOR of all nibbles |

Protocol rules:

- Idle-state bits are `1`
- Sync pattern is `0101 1000`
- Input is processed bit-by-bit from left to right
- All frames in the stream are decoded
- The checksum is verified for each frame

## Requirements

- C++20 compatible compiler
- CMake 3.20+
- Bash, if using `build.sh`

## Build

The simplest way to build the application is:

```bash
./build.sh
```
