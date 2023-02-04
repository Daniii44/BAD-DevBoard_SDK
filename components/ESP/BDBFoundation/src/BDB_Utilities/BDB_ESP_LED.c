#include "BDB_ESP_LED.h"
#include "driver/gpio.h"

static esp_led_state_t currentState;

esp_led_state_t ESP_LED_getState(){
    return currentState;
}

void ESP_LED_toggle(){
    ESP_LED_setState(!currentState);
}

void ESP_LED_setState(esp_led_state_t nextState){
    gpio_set_level(BDB_ESP_LED_PIN, nextState);
    currentState = nextState;
}