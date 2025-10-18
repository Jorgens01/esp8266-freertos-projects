/***************************************************************************
 * File name    :   timer.c
 * Description  :   Hardware timer module implementation
 *
 * This file implements the logic for a hardware timer on the ESP8266 using
 * the FreeRTOS SDK. It configures a periodic hardware timer interrupt and
 * uses a binary semaphore to synchronize the ISR with a dedicated FreeRTOS
 * task. This approach follows the deferred interrupt processing pattern,
 * keeping the ISR short and offloading the main work to a task context.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-19
 **************************************************************************/

#include "timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/hw_timer.h"
#include "esp_log.h"

static const char *TAG = "TIMER_MODULE";

// Semaphore to synchronize the ISR and and timer task
static SemaphoreHandle_t s_timer_semaphore;

/**
 * @brief The Interrupt Service Routine (ISR) for the hardware timer.
 *
 * This function is called automatically when the hardware timer alarm triggers.
 * Its only job is to "give" a semaphore, which unblocks the handler task.
 * This keeps the ISR very short and fast, which is best practice.
 */
static void IRAM_ATTR timer_isr(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Give semaphore to unblock handler task
    xSemaphoreGiveFromISR(s_timer_semaphore, &xHigherPriorityTaskWoken);

    // If xHigherPriorityTaskWoken is now set to pdTRUE, a context switch
    // should be performed to allow the unblocked task to run immediately.
    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief FreeRTOS task to handle the timer event.
 *
 * This task waits indefinitely to "take" the semaphore. When the ISR gives
 * the semaphore, this task unblocks and executes its logic, in this case,
 * printing a message. This is known as "deferred interrupt processing".
 */
static void timer_event_task(void *arg)
{
    while (1)
    {
        // Wait for semaphore to be given by ISR
        if (xSemaphoreTake(s_timer_semaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Timer event occurred. Performing action");
        }
    }
}

/**
 * @brief Public function to initialize the timer module.
 */
void hardware_timer_init(void)
{
    s_timer_semaphore = xSemaphoreCreateBinary();
    if (s_timer_semaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create timer semaphore");
        return;
    }

    // Initialize hardware timer
    esp_err_t err = hw_timer_init(timer_isr, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialzie hardware timer");
        return;
    }

    // Configure timer alarm
    err = hw_timer_alarm_us(1000000, true); // 1_000_000 us = 1 s
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set timer alarm");
    }

    // Create FreeRTOS task that will handle timer events
    BaseType_t task_created = xTaskCreate(
        timer_event_task,
        "timer_event_task",
        2048,
        NULL,
        5,
        NULL
    );

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create timer event task");
    } else 
    {
        ESP_LOGI(TAG, "Hardware timer and handler task initialized successfully");    
    }

}