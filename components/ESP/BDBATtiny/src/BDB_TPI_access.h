#ifndef _TPI_ACCESS_H
#define _TPI_ACCESS_H

#include "esp_types.h"

typedef enum{
    NVM_CMD_NOP = 0x00,
    NVM_CMD_CHIP_ERASE = 0x10,
    NVM_CMD_SECTION_ERASE = 0x14,
    NVM_CMD_DWORD_WRITE = 0x1D
} nvm_cmd_t;


//SRAM
void TPI_readSRAM(uint8_t* buf, uint16_t startTPIPointer, uint16_t size);
void TPI_writeSRAM(uint8_t* buf, uint16_t startTPIPointer, uint16_t size);
void TPI_writeSRAMByte(uint16_t address, uint8_t data);

//Flash
void TPI_NVM_ChipErase();
void TPI_writeTPIPointer(uint16_t tpiPointer);
void TPI_writeFlash(uint32_t dWord);
void TPI_writeFlashNoPolling(uint32_t dWord);
void TPI_readFlash(uint8_t* buf, uint16_t startTPIPointer, uint16_t size);

//Non-Volatile Memory Controller
void TPI_enableNVMController();
void TPI_disableNVMController();
void TPI_NVM_SetCommand(nvm_cmd_t NVM_CMD);
uint8_t TPI_getNVMEN();
uint8_t TPI_getNVMBUSY();

//TPI Instruction Set
uint8_t TPI_SDL(bool postIncrement);                  // Serial LoaD from data space using indirect addressing
void TPI_SST(bool postIncrement,uint8_t data);        // Serial STore to data space using indirect addressing
void TPI_SSTPR(uint8_t pointerByte, uint8_t pointer); // Serial STore to Pointer Register using direct addressing
uint8_t TPI_SIN(uint8_t address);                     // Serial IN from data space
void TPI_SOUT(uint8_t address, uint8_t data);        // Serial OUT to data space
uint8_t TPI_SLDCS(uint8_t address);                   // Serial LoaD from Control and Status space using direct addressing
void TPI_SSTCS(uint8_t address, uint8_t data);        // Serial STore to Control and Status space using direct addressing
void TPI_SKEY(uint64_t key);                          // Serial KEY

#endif // _TPI_ACCESS_H