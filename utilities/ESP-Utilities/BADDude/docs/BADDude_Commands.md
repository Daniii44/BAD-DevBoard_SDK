# Definitions
Max Chunk Size: 64 Bytes

# Keys
## Start Key
32 bits: 0x89ABCDEF

## Custom Prgram Completion Key
abbreviation: CPCPLT_KEY
32 bits: 0xC0C1C2C3

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
operands:
    1. progress percentage (0 = 0% / 255 = 100%)

## Flash Read
mnemonic: FRead
opcode: 0x2A

# IO
## ATTiny Reset
mnemonic: ATRESET
opcode: 0x30

# Custom Programs
## Custom Program Count
mnemonic: CPCNT
opcode: 0x40

## Custom Program Title
mnemonic: CPTITLE
opcode: 0x41
operands:
    1. programID
returns:
    1. ACK
    2. null terminated string upto a size of MAX Chunk Size

## Custom Program Run
mnemonic: CPRUN
opcode: 0x42
operands:
    1. programID