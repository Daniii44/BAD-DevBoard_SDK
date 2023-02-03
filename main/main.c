#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "BDB_Foundation.h"
#include "BDB_Utilities/BDB_ESP_LED.h"

#include "BDB_TPI_physical.h"
#include "BDB_TPI_access.h"
#include "driver/usb_serial_jtag.h"

#include "BADDude_protocol.h"

void BADDUDE_acknowledge(){
    uint8_t ack = BADDUDE_CMD_ACK;
    usb_serial_jtag_write_bytes(&ack,1,portMAX_DELAY);
}
void BADDUDE_fwprog(uint8_t progress){
    uint8_t fwprog = BADDUDE_CMD_FWPROG;
    usb_serial_jtag_write_bytes(&fwprog,1,portMAX_DELAY);
    usb_serial_jtag_write_bytes(&progress,1,portMAX_DELAY);
}
void BADDUDE_nextChunk(){
    uint8_t nextChunk = BADDUDE_CMD_NEXTCHUNK;
    usb_serial_jtag_write_bytes(&nextChunk,1,portMAX_DELAY);
}

void app_main(void)
{
    esp_task_wdt_add(NULL);

    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.rx_buffer_size = 1024;
    usb_serial_jtag_config.tx_buffer_size = 1024;

    usb_serial_jtag_driver_install(&usb_serial_jtag_config);

    uint32_t startKey = BADDUDE_START_KEY;
    usb_serial_jtag_write_bytes(&startKey,4,portMAX_DELAY);

    for(;;){
        uint8_t opcode;
        usb_serial_jtag_read_bytes(&opcode,1,portMAX_DELAY);

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

            uint8_t* buf = malloc(2048);
            memset(buf, 0xFF, 2048);
            for(int i = 0; i < size; i += BADDUDE_MAX_CHUNK_SIZE){
                if((size - i) < BADDUDE_MAX_CHUNK_SIZE)
                    usb_serial_jtag_read_bytes(buf + i,(size - i),portMAX_DELAY);
                else
                    usb_serial_jtag_read_bytes(buf + i,BADDUDE_MAX_CHUNK_SIZE,portMAX_DELAY);
                BADDUDE_nextChunk();
            }
            BADDUDE_acknowledge();

            TPI_writeTPIPointer(0x4000);

            for(uint32_t i = 0; i < size/4;i++){
                TPI_writeFlashNoPolling(*(uint32_t*)&(buf[i*4]));
                BADDUDE_fwprog((uint8_t)((i*4*0xFF)/size));
                while(TPI_getNVMBUSY());
            }
            if(size % 4 != 0)
                TPI_writeFlash(*(uint32_t*)&(buf[size/4+1]));
            BADDUDE_fwprog(0xFF);
            
            free(buf);
        }
    }
}