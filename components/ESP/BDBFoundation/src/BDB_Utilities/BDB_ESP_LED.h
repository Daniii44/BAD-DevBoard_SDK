#ifndef _BDB_ESP_LED_H
#define _BDB_ESP_LED_H

#include "BDB_Foundation.h"

typedef enum
{
    ESP_LED_OFF = 0,
    ESP_LED_ON = 1
} esp_led_state_t;

esp_led_state_t ESP_LED_getState();
void ESP_LED_setState(esp_led_state_t nextState);
void ESP_LED_toggle();

#endif // _BDB_ESP_LED_H