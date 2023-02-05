#include "BDB_TPI_access.h"
#include "BDB_TPI_physical.h"

//Control and Status Space
#define TPI_SR_ADDR 0x00
#define TPI_SR_NVMEN_MASK 0x02

//IO Space
#define NVMCMD_ADDR 0x33
#define NVMCSR_ADDR 0x32
#define NVMCSR_NVMBSY_MASK 0x80

void TPI_NVM_ChipErase(){
    TPI_NVM_SetCommand(NVM_CMD_CHIP_ERASE);
    TPI_writeTPIPointer(0x4001);

    for(int i = 0; i < 4; i++) // 4x dummy byte
        TPI_SST(1,0x64);

    while(TPI_getNVMBUSY());
}
void TPI_writeTPIPointer(uint16_t tpiPointer){
    TPI_SSTPR(0, tpiPointer & 0xFF);
    TPI_SSTPR(1, (tpiPointer >> 8) & 0xFF);
}
void TPI_writeFlash(uint32_t dWord){
    TPI_writeFlashNoPolling(dWord);
    while(TPI_getNVMBUSY());
}
void TPI_writeFlashNoPolling(uint32_t dWord){
    TPI_NVM_SetCommand(NVM_CMD_DWORD_WRITE);
    
    TPI_SST(1, (dWord >> 0) & 0xFF);
    TPI_SST(1, (dWord >> 8) & 0xFF);
    TPI_writeBit(1); //IDLE
    TPI_SST(1, (dWord >> 16) & 0xFF);
    TPI_SST(1, (dWord >> 24) & 0xFF);
}
void TPI_readFlash(uint8_t* buf, uint16_t startPointer, uint16_t size){
    TPI_writeTPIPointer(startPointer);

    for(int i = 0; i < size;i++)
        buf[i] = TPI_SDL(1);
}

//Non-Volatile Memory Controller
void TPI_enableNVMController(){
    uint64_t key = 0x1289AB45CDD888FF;
    TPI_SKEY(key);

    while(!TPI_getNVMEN());
}
void TPI_disableNVMController(){
    TPI_SSTCS(TPI_SR_ADDR,0);
}
void TPI_NVM_SetCommand(nvm_cmd_t NVM_CMD){
    TPI_SOUT(NVMCMD_ADDR,NVM_CMD);
}
uint8_t TPI_getNVMEN(){
    return (TPI_SLDCS(TPI_SR_ADDR) & TPI_SR_NVMEN_MASK) != 0;
}
uint8_t TPI_getNVMBUSY(){
    return (TPI_SIN(NVMCSR_ADDR) & NVMCSR_NVMBSY_MASK) != 0;
}

// TPI Instruction Set
uint8_t TPI_SDL(bool postIncrement){
    if(postIncrement)
        TPI_writeFrame(0x24);
    else
        TPI_writeFrame(0x20);
    return TPI_readFrame();
}
void TPI_SST(bool postIncrement,uint8_t data){
    if(postIncrement)
        TPI_writeFrame(0x64);
    else
        TPI_writeFrame(0x60);
    TPI_writeFrame(data);
}
void TPI_SSTPR(uint8_t pointerByte, uint8_t pointer){
    if(pointerByte == 0)
        TPI_writeFrame(0x68);
    else
        TPI_writeFrame(0x69);
    TPI_writeFrame(pointer);
}
uint8_t TPI_SIN(uint8_t address){
    uint8_t opcode = 0x10;
    opcode |= address & 0x0F;
    opcode |= (address<<1) & 0x60;
    TPI_writeFrame(opcode);
    
    return TPI_readFrame();
}
void TPI_SOUT(uint8_t address, uint8_t data){
    uint8_t opcode = 0x90;
    opcode |= address & 0x0F;
    opcode |= (address<<1) & 0x60;
    TPI_writeFrame(opcode);
    
    TPI_writeFrame(data);
}
uint8_t TPI_SLDCS(uint8_t address){
    uint8_t opcode = 0x80;
    opcode |= address & 0x0F;
    TPI_writeFrame(opcode);

    return TPI_readFrame();
}
void TPI_SSTCS(uint8_t address, uint8_t data){
    uint8_t opcode = 0xC0;
    opcode |= address & 0x0F;
    TPI_writeFrame(opcode);

    TPI_writeFrame(data);
}
void TPI_SKEY(uint64_t key){
    TPI_writeFrame(0xE0);

    for(int i = 0; i < 8; i++)
        TPI_writeFrame((uint8_t)(key >> i*8));
}