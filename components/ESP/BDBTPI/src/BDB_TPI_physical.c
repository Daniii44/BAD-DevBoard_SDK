#include "BDB_TPI_physical.h"
#include "driver/gpio.h"
#include "BDB_ESP_Pins.h"
#include "BDB_Timing.h"

#define T_HALF_CYCLE_PERIOD_US 1
#define t_TOUT_US 256*1000           // Time-out after reset
#define t_RST_US 1                   // Minimum pulse width on RESET Pin
#define TPI_FRAME_SIZE 12

#define ATTINY_RESET_ACTIVE_LEVEL 1 // The actual level applied to the #RESET pin of the ATTiny is inverted by a mosfet

void TPI_PHY_init(){
    gpio_set_direction(BDB_ESP_ATTINY_RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BDB_ESP_ATTINY_TPIDATA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BDB_ESP_ATTINY_TPICLK_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, ATTINY_RESET_ACTIVE_LEVEL);
    gpio_set_level(BDB_ESP_ATTINY_TPIDATA_PIN, 0);
    gpio_set_level(BDB_ESP_ATTINY_TPICLK_PIN, 0);
}
void TPI_enter(){
    //Firstly, enable the ATTiny quickly to securely put it into reset mode afterwards
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, !ATTINY_RESET_ACTIVE_LEVEL);
    BDB_delayMicroseconds(t_TOUT_US);
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, ATTINY_RESET_ACTIVE_LEVEL);
    BDB_delayMicroseconds(t_RST_US);

    for(int i = 0; i < 16;i++){
        TPI_writeBit(1);
    }
}
void TPI_exit(bool releaseReset){
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, !ATTINY_RESET_ACTIVE_LEVEL);
    BDB_delayMicroseconds(t_TOUT_US);
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, ATTINY_RESET_ACTIVE_LEVEL);
    BDB_delayMicroseconds(t_RST_US);
    if(releaseReset){
        gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, !ATTINY_RESET_ACTIVE_LEVEL);
        BDB_delayMicroseconds(t_TOUT_US);
    }
}
void TPI_PHY_deinit(bool deinitReset){
    if(deinitReset)
        gpio_set_direction(BDB_ESP_ATTINY_RESET_PIN, GPIO_MODE_DEF_DISABLE);
    gpio_set_direction(BDB_ESP_ATTINY_TPIDATA_PIN, GPIO_MODE_DEF_DISABLE);
    gpio_set_direction(BDB_ESP_ATTINY_TPICLK_PIN, GPIO_MODE_DEF_DISABLE);
}

void TPI_writeFrame(uint8_t data){
    gpio_set_direction(BDB_ESP_ATTINY_TPIDATA_PIN, GPIO_MODE_OUTPUT);

    uint8_t parity = data;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    parity &= 0x01;

    uint16_t send_buffer = 0;
    send_buffer |= 0; //Start bits
    send_buffer |= (uint16_t)data << 1;
    send_buffer |= (uint16_t)parity << 9;
    send_buffer |= 0x0C00; //Stop bits

    for(int i = 0; i < TPI_FRAME_SIZE; i++){
        TPI_writeBit((send_buffer >> i) & 0x01);
    }
}
uint8_t TPI_readFrame(){
    gpio_set_direction(BDB_ESP_ATTINY_TPIDATA_PIN, GPIO_MODE_INPUT);
    uint8_t data = 0;

    for(int i = 0; i < 256;i++)
        if(!TPI_readBit())
            break;

    for(int i = 0; i < 8;i++){
        data |= TPI_readBit() << i;
    }

    //TODO: parity and stop bit check
    for(int i = 0; i < 3;i++){
        TPI_readBit();
    }

    return data;
}

void TPI_writeBit(uint8_t bit){
    gpio_set_level(BDB_ESP_ATTINY_TPICLK_PIN, 0);
    gpio_set_level(BDB_ESP_ATTINY_TPIDATA_PIN,bit);
    BDB_delayMicroseconds(T_HALF_CYCLE_PERIOD_US);

    gpio_set_level(BDB_ESP_ATTINY_TPICLK_PIN, 1);
    BDB_delayMicroseconds(T_HALF_CYCLE_PERIOD_US);
}
uint8_t TPI_readBit(){
    gpio_set_level(BDB_ESP_ATTINY_TPICLK_PIN, 0);
    BDB_delayMicroseconds(T_HALF_CYCLE_PERIOD_US);

    gpio_set_level(BDB_ESP_ATTINY_TPICLK_PIN, 1);
    uint8_t bit = gpio_get_level(BDB_ESP_ATTINY_TPIDATA_PIN);
    BDB_delayMicroseconds(T_HALF_CYCLE_PERIOD_US);

    return bit;
}