/***************************************************************************
 * File name    :   main.c
 * Description  :   Main application entry point for the DHT11 sensor project.
 *
 * This file initializes the core ESP8266 system components, retrieves and
 * prints chip hardware information (cores, revision, flash size), and demonstrates
 * the use of a non-blocking FreeRTOS delay (vTaskDelay) within the main
 * application context for periodic execution.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-13
 **************************************************************************/
#include "dht.h"
#include "freertos/task.h"
#include "esp_log.h"

#define DHT11_GPIO_PIN GPIO_NUM_4
#define TASK_DELAY_MS 2000

static const char *TAG_MAIN = "APP_MAIN";


/**
 * @brief FreeRTOS task to periodically read the DHT11 sensor.
 *
 * This is the main application loop for the sensor reading. It is a dedicated
 * FreeRTOS task that runs at a scheduled interval.
 *
 * @param pvParameters Not used (NULL), but required by the FreeRTOS task prototype.
 */
static void dht11_reader_task(void *pvParameters)
{
    dht11_data_t sensor_data;

    while (1) 
    {
        if (dht11_read(DHT11_GPIO_PIN, &sensor_data) == pdTRUE)
        {
            ESP_LOGI(TAG_MAIN,  "Humidity: %d %% \t Temperature: %d C",
                    sensor_data.humidity / 10, sensor_data.temperature / 10);
        } else
        {
            ESP_LOGE(TAG_MAIN, "Failed to read from DHT11 sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    // Initialize DHT11 driver 
    if (dht11_init(DHT11_GPIO_PIN) != ESP_OK)
    {
        ESP_LOGE(TAG_MAIN, "DHT11 initialization failed.");
        return;
    }

    // Create task that will read from the sensor
    BaseType_t ret = xTaskCreate(
        dht11_reader_task,
        "dht11_task",
        2048,
        NULL,
        5,
        NULL
    );

    if (ret != pdPASS)
        ESP_LOGE(TAG_MAIN, "Failed to create DHT reader task");
}