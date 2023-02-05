#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "BDB_Foundation.h"
#include "BDB_Utilities/BDB_ESP_LED.h"

#include "BDB_BADDude.h"


void app_main(void)
{
    esp_task_wdt_add(NULL);

    BADDude_start(NULL);
}