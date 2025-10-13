/***************************************************************************
 * File name    :   main.c
 * Description  :   ESP8266 FreeRTOS Concurrency Demonstration.
 *
 * This project utilizes two independent FreeRTOS tasks to drive two external LEDs
 * at distinct, non-blocking blink rates (500ms and 1000ms). It serves as a
 * fundamental example of safe GPIO operation, priority-based scheduling, and
 * Watchdog Timer (WDT) avoidance using vTaskDelay on the ESP8266 platform.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-13
 **************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// --- CONFIGURATION CONSTANTS ---
#define LED_PIN_1 GPIO_NUM_4
#define LED_PIN_2 GPIO_NUM_5

// Blinking periods in ms
#define BLINK_DELAY_MS_1 (500)
#define BLINK_DELAY_MS_2 (1000)

// Stack size
#define BLINK_STACK_SIZE (2048)

static const char *TAG = "ESP8266_BLINK";

/**
 * @brief FreeRTOS task to blink LED with 500ms period
 */
void led_blinker_task_1(void *pvParameters)
{
    int led_state = 0;
    gpio_set_level(LED_PIN_1, led_state);

    while (1)
    {
        led_state = !led_state;
        gpio_set_level(LED_PIN_1, led_state);

        // Non-blocking delay: yields CPU scheduler
        vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS_1));
    }
    vTaskDelete(NULL);
}

/**
 * @brief FreeRTOS task to blink LED with 1000ms period
 */
void led_blinker_task_2(void *pvParameters)
{
    int led_state = 1;
    gpio_set_level(LED_PIN_2, led_state);

    while (1)
    {
        led_state = !led_state;
        gpio_set_level(LED_PIN_2, led_state);

        vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS_2));
    }
    vTaskDelete(NULL);
}

/**
 * @brief Main application entry point. Initializes hardware and tasks
 */
void app_main(void)
{
    // --- GPIO Configuration ---
    gpio_config_t io_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = ((1ULL << LED_PIN_1) | (1ULL << LED_PIN_2)),
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // Check return value
    if (gpio_config(&io_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "ERROR: GPIO configuration failed!");
        return; // Halt if fails
    }
    ESP_LOGI(TAG, "GPIOs %d and %d initialized as outputs.", LED_PIN_1, LED_PIN_2);

    // --- FreeRTOS Task Creation ---
    BaseType_t result1, result2;

    // Create Task 1
    result1 = xTaskCreate(
        led_blinker_task_1,   // Task function pointer
        "LED_Blinker_500ms",  // Task name for debugging
        BLINK_STACK_SIZE,     // Stack depth in words
        NULL,                 // Parameter to pass to function
        tskIDLE_PRIORITY + 1, // Priority: low
        NULL);                // Task handle (not here)

    // Create Task 2
    result2 = xTaskCreate(
        led_blinker_task_2,   // Task function pointer
        "LED_Blinker_1000ms", // Task name for debugging
        BLINK_STACK_SIZE,     // Stack depth in words
        NULL,                 // Parameter to pass to function
        tskIDLE_PRIORITY + 1, // Priority: low
        NULL);                // Task handle (not here)

    // Check for successful task creation
    if ((result1 != pdPASS) || (result2 != pdPASS))
    {
        ESP_LOGE(TAG, "ERROR: Failed to create task(s). Check HEAP.");
    }
    else
    {
        ESP_LOGI(TAG, "Two blinker tasks created successfully.");
    }
}
