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

//TODO: move this to BDBFoundation
#define ATTINY_RESET_ACTIVE_LEVEL 1 // The actual level applied to the #RESET pin of the ATTiny is inverted by a mosfet
#define t_TOUT_US 256*1000           // Time-out after reset
#define t_RST_US 1                   // Minimum pulse width on RESET Pin
#define ATTINY_MEMORY_FLASH_ADDR 0x4000
#define ATTINY_MEMORY_FLASH_SIZE 0x0800

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
void BADDUE_pollNextChunk(){
    uint8_t nextChunk = 0;
    while(nextChunk != BADDUDE_CMD_NEXTCHUNK)
        usb_serial_jtag_read_bytes(&nextChunk,1,portMAX_DELAY);
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

    gpio_set_direction(BDB_ESP_ATTINY_RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN,ATTINY_RESET_ACTIVE_LEVEL);
    BDB_delayMicroseconds(t_RST_US);
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN,!ATTINY_RESET_ACTIVE_LEVEL);

    for(;;){
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
            gpio_set_level(BDB_ESP_ATTINY_RESET_PIN,ATTINY_RESET_ACTIVE_LEVEL);
            BDB_delayMicroseconds(t_RST_US);
            gpio_set_level(BDB_ESP_ATTINY_RESET_PIN,!ATTINY_RESET_ACTIVE_LEVEL);
            BDB_delayMicroseconds(t_TOUT_US);
            BADDUDE_acknowledge();
        }
    }
}