#ifndef _BADDUDE_PROTOCOL_H
#define _BADDUDE_PROTOCOL_H

#define BADDUDE_MAX_CHUNK_SIZE 64
#define BADDUDE_START_KEY 0x89ABCDEF

typedef enum
{
    BADDUDE_CMD_NOP = 0x00,
    BADDUDE_CMD_ACK = 0x01,
    BADDUDE_CMD_NACK = 0x02,
    BADDUDE_CMD_CPLT = 0x03,
    BADDUDE_CMD_NEXTCHUNK = 0x04,
    BADDUDE_CMD_PING = 0x05,

    BADDUDE_CMD_TPIENTER = 0x10,
    BADDUDE_CMD_TPIEXIT = 0x11,

    BADDUDE_CMD_FPRIME = 0x20,
    BADDUDE_CMD_FERASE = 0x21,
    BADDUDE_CMD_FWRITE = 0x22,
    BADDUDE_CMD_FWPROG = 0x23,

    BADDUDE_CMD_FRead = 0x2A,

    BADDUDE_CMD_ATRESET = 0x30,
} baddude_cmd_t;

#endif // _BADDUDE_PROTOCOL_H