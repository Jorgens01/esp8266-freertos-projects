/***************************************************************************
 * File name    :   main.c
 * Description  :   ESP8266 Project Template
 *
 * This file initializes the core ESP8266 system components, retrieves and
 * prints chip hardware information (cores, revision, flash size), and demonstrates
 * the use of a non-blocking FreeRTOS delay (vTaskDelay) within the main
 * application context for periodic execution.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-13
 **************************************************************************/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void app_main(void)
{
    printf("Hello from ESP8266 Project!\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("ESP8266 with %d CPU cores, WiFi\n", chip_info.cores);
    printf("Silicon revision: %d\n", chip_info.revision);
    printf("Flash: %dMB %s\n",
           spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // Your code here
    while (1)
    {
        printf("Running...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
