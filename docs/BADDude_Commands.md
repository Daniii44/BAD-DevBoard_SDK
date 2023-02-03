# Definitions
Max Chunk Size: 64 Bytes

# Start Key
32 bits: 0x89ABCDEF

# Commands
## No Operation
mnemonic: NOP
opcode: 0x00

## Acknowledge
mnemonic: ACK
opcode: 0x01

## No Acknowledge
mnemonic: NACK
opcode: 0x02

## Complete
mnemonic: CPLT
opcode: 0x03

## Next Chunk
mnemonic: NEXTCHUNK
opcode: 0x04

## Ping
mnemonic: PING
opcode: 0x05

# TPI
## TPI Exit
mnemonic: TPIENTER
opcode: 0x10

## TPI Enter
mnemonic: TPIEXIT
opcode: 0x11

# FLASH
## Flash Prime
mnemonic: FPRIME
opcode: 0x20

## Flash Erase
mnemonic: FERASE
opcode: 0x21

## Flash Write
mnemonic: FWRITE
opcode: 0x22
operands:
    1. size (16 bits) (LSB first)
    2. data

## Flash Write Progress
mnemonic: FWPROG
opcode: 0x23
    1. progress percentage (0 = 0% / 255 = 100%)