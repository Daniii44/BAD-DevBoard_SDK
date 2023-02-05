#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/usb_serial_jtag.h"

#include "BDB_Foundation.h"

#include "BDB_BADDude.h"
#include "BDB_BADDude_protocol.h"

#include "BDB_TPI_physical.h"
#include "BDB_TPI_access.h"
#include "BDB_ATtiny_def.h"
#include "BDB_ATtiny_Reset.h"


static baddude_callback_t *baddudeCallback;

static void BADDude_setup();
static void BADDude_loop();

static void BADDUDE_acknowledge();
static void BADDUDE_notAcknowledge();
static void BADDUDE_fwprog(uint8_t progress);
static void BADDUDE_nextChunk();
static void BADDUE_pollNextChunk();


void BADDude_start(baddude_callback_t* baddude_callback){
    baddudeCallback = baddude_callback;

    BADDude_setup();
    for(;;){
        BADDude_loop();
    }
}

void BADDude_setup(){
    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.rx_buffer_size = 1024;
    usb_serial_jtag_config.tx_buffer_size = 1024;
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);

    uint32_t startKey = BADDUDE_START_KEY;
    usb_serial_jtag_write_bytes(&startKey,4,portMAX_DELAY);

    BDB_ATtiny_InitReset();
    BDB_delayMicroseconds(t_RST_US);
    BDB_ATtiny_ReleaseReset();
}
void BADDude_loop(){
    uint8_t opcode;
    usb_serial_jtag_read_bytes(&opcode,1,portMAX_DELAY);

    //TODO: use a switch case statement
    if (opcode == BADDUDE_CMD_PING){
        BADDUDE_acknowledge();
    }
    else if (opcode == BADDUDE_CMD_TPIENTER){
        TPI_PHY_init();
        TPI_enter();
        BADDUDE_acknowledge();
    }
    else if (opcode == BADDUDE_CMD_TPIEXIT){
        TPI_disableNVMController();
        TPI_exit(true);
        TPI_PHY_deinit(false);
        BADDUDE_acknowledge();
    }   
    else if (opcode == BADDUDE_CMD_FPRIME){
        TPI_enableNVMController();
        BADDUDE_acknowledge();
    }
    else if(opcode == BADDUDE_CMD_FERASE){
        TPI_NVM_ChipErase();
        BADDUDE_acknowledge();
    }
    else if (opcode == BADDUDE_CMD_FWRITE){
        uint32_t size = 0;
        usb_serial_jtag_read_bytes(&size,2,portMAX_DELAY);

        //Receive Data
        uint8_t* buf = malloc(ATTINY_MEMORY_FLASH_SIZE);
        memset(buf, 0xFF, ATTINY_MEMORY_FLASH_SIZE);
        for(int i = 0; i < size; i += BADDUDE_MAX_CHUNK_SIZE){
            if((size - i) < BADDUDE_MAX_CHUNK_SIZE)
                usb_serial_jtag_read_bytes(buf + i,(size - i),portMAX_DELAY);
            else
                usb_serial_jtag_read_bytes(buf + i,BADDUDE_MAX_CHUNK_SIZE,portMAX_DELAY);
            BADDUDE_nextChunk();
        }
        BADDUDE_acknowledge();

        //Write Data
        TPI_writeTPIPointer(ATTINY_MEMORY_FLASH_ADDR);
        for(uint32_t i = 0; i < size;i+=4){
            TPI_writeFlashNoPolling(*(uint32_t*)&(buf[i]));
            BADDUDE_fwprog((uint8_t)(((i*0xFF)/size)));
            while(TPI_getNVMBUSY());
        }
        BADDUDE_fwprog(0xFF);
        
        free(buf);
    }
    else if (opcode == BADDUDE_CMD_FRead){
        BADDUDE_acknowledge();
        uint8_t* buf = malloc(BADDUDE_MAX_CHUNK_SIZE);

        for(int i = 0; i < ATTINY_MEMORY_FLASH_SIZE; i += BADDUDE_MAX_CHUNK_SIZE){
            TPI_readFlash(buf,ATTINY_MEMORY_FLASH_ADDR+i,BADDUDE_MAX_CHUNK_SIZE);
            if (i != 0)
                BADDUE_pollNextChunk();
            usb_serial_jtag_write_bytes(buf,BADDUDE_MAX_CHUNK_SIZE,portMAX_DELAY);
        }

        free(buf);
    }
    else if (opcode == BADDUDE_CMD_ATRESET){
        BDB_ATtiny_Reset();
        BADDUDE_acknowledge();
    }
    else if (opcode == BADDUDE_CMD_CPCNT){
        BADDUDE_acknowledge();

        uint8_t count = 0;
        if(baddudeCallback != NULL)
            count = baddudeCallback->getCustomProgramCount();
        
        usb_serial_jtag_write_bytes(&count,1,portMAX_DELAY);
    }
    else if (opcode == BADDUDE_CMD_CPTITLE){
        uint8_t programID;
        usb_serial_jtag_read_bytes(&programID,1,portMAX_DELAY);

        if(baddudeCallback == NULL){
            BADDUDE_notAcknowledge();
            return; //TODO: use a switch statement and use break
        }
        BADDUDE_acknowledge();

        const char* title = baddudeCallback->getCustomProgramTitle(programID);
        uint8_t nullTermination = 0;
        uint8_t length = strlen(title);
        if(length > BADDUDE_MAX_CHUNK_SIZE-1)
            length = BADDUDE_MAX_CHUNK_SIZE-1;
        
        usb_serial_jtag_write_bytes(title,length,portMAX_DELAY);
        usb_serial_jtag_write_bytes(&nullTermination,1,portMAX_DELAY);
    }
    else if (opcode == BADDUDE_CMD_CPRUN){
        uint8_t programID;
        usb_serial_jtag_read_bytes(&programID,1,portMAX_DELAY);

        if(baddudeCallback == NULL){
            BADDUDE_notAcknowledge();
            return; //TODO: use a switch statement and use break
        }
        BADDUDE_acknowledge();

        baddudeCallback->runCustomProgram(programID);

        uint32_t cpcpltKey = BADDUDE_CPCPLT_KEY;
        usb_serial_jtag_write_bytes(&cpcpltKey,4,portMAX_DELAY);
    }
}

static void BADDUDE_acknowledge(){
    uint8_t ack = BADDUDE_CMD_ACK;
    usb_serial_jtag_write_bytes(&ack,1,portMAX_DELAY);
}
static void BADDUDE_notAcknowledge(){
    uint8_t nack = BADDUDE_CMD_NACK;
    usb_serial_jtag_write_bytes(&nack,1,portMAX_DELAY);
}
static void BADDUDE_fwprog(uint8_t progress){
    uint8_t fwprog = BADDUDE_CMD_FWPROG;
    usb_serial_jtag_write_bytes(&fwprog,1,portMAX_DELAY);
    usb_serial_jtag_write_bytes(&progress,1,portMAX_DELAY);
}
static void BADDUDE_nextChunk(){
    uint8_t nextChunk = BADDUDE_CMD_NEXTCHUNK;
    usb_serial_jtag_write_bytes(&nextChunk,1,portMAX_DELAY);
}
static void BADDUE_pollNextChunk(){
    uint8_t nextChunk = 0;
    while(nextChunk != BADDUDE_CMD_NEXTCHUNK)
        usb_serial_jtag_read_bytes(&nextChunk,1,portMAX_DELAY);
}