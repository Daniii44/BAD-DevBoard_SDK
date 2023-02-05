#include "BDB_ATtiny_Reset.h"
#include "BDB_ATtiny_def.h"
#include "BDB_Foundation.h"
#include "driver/gpio.h"

void BDB_ATtiny_InitReset(){
    gpio_set_direction(BDB_ESP_ATTINY_RESET_PIN, GPIO_MODE_DEF_OUTPUT);
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, ATTINY_RESET_ACTIVE_LEVEL); // Stay at the level previously applied by the pull-up resistor
}
void BDB_ATtiny_DeinitReset(){
    gpio_set_direction(BDB_ESP_ATTINY_RESET_PIN, GPIO_MODE_DEF_DISABLE);
}

void BDB_ATtiny_ApplyReset(){
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, ATTINY_RESET_ACTIVE_LEVEL);
}
void BDB_ATtiny_ReleaseReset(){
    gpio_set_level(BDB_ESP_ATTINY_RESET_PIN, !ATTINY_RESET_ACTIVE_LEVEL);
}

void BDB_ATtiny_Reset(){
    BDB_ATtiny_ApplyReset();
    BDB_delayMicroseconds(t_RST_US);
    BDB_ATtiny_ReleaseReset();
    BDB_delayMicroseconds(t_TOUT_US);
}