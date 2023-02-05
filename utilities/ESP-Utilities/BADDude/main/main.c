#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"

#include "BDB_Foundation.h"
#include "BDB_Utilities/BDB_ESP_LED.h"

#include "BDB_BADDude.h"

int getCustomProgramCount();
const char* getCustomProgramTitle(int programID);
void runCustomProgram(int programID);

void app_main(void)
{
    esp_task_wdt_add(NULL);

    baddude_callback_t baddudeCallback;
    baddudeCallback.getCustomProgramCount = getCustomProgramCount;
    baddudeCallback.getCustomProgramTitle = getCustomProgramTitle;
    baddudeCallback.runCustomProgram = runCustomProgram;

    gpio_set_direction(BDB_ESP_LED_PIN,GPIO_MODE_OUTPUT);

    BADDude_start(&baddudeCallback);
}

int getCustomProgramCount(){
    return 2;
}
const char* getCustomProgramTitle(int programID){
    switch (programID){
        case 0:
            return "LED ON";
        case 1:
            return "LED OFF";
    }
    return "";
}
void runCustomProgram(int programID){
    switch (programID){
        case 0: // LED ON
            ESP_LED_setState(ESP_LED_ON);
            break;
        case 1: // LED OFF
            ESP_LED_setState(ESP_LED_OFF);
            break;
    }
}