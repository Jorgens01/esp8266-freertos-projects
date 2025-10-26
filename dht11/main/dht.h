/***************************************************************************
 * File name    :   dht11.h
 * Description  :   Public interface for the DHT11 temperature and humidity
 *                  sensor driver.
 *
 * This file defines the data structure for holding sensor readings and
 * declares the functions used by application code (like main.c) to
 * initialize the sensor and read data from it.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-26
 **************************************************************************/

#ifndef DHT_H
#define DHT_H

#include "freertos/FreeRTOS.h"  // For BaseType_t
#include "driver/gpio.h"        // gpio_num_t
#include "esp_err.h"            // esp_err_t

#include <stdint.h>


/**
 * @brief Structure to hold the sensor data.
 *
 * Readings are stored as integer values multiplied by 10 to represent
 * one decimal place without using floating-point arithmetic.
 * This conserves memory and avoids complex floating-point libraries on a limited
 * microcontroller like the ESP8266.
 */
typedef struct
{
    int16_t temperature; // Temperature in Celsius * 10 (e.g., 25.5C is 255)
    int16_t humidity;    // Humidity in %RH * 10 (e.g., 60.5% is 605)
} dht11_data_t;

/**
 * @brief Initializes the GPIO pin used for the DHT11 sensor.
 *
 * Configures the specified GPIO pin as an Open-Drain Output with an
 * internal pull-up resistor, which is required for the 1-wire protocol.
 *
 * @param pin The GPIO number to use for the 1-wire communication (e.g., GPIO_NUM_4).
 * @return esp_err_t ESP_OK on successful configuration, or an ESP-IDF error code on failure.
 */
esp_err_t dht11_init(gpio_num_t pin);

/**
 * @brief Reads temperature and humidity data from the DHT11 sensor.
 *
 * This function is **time-critical**. It handles the entire 1-wire communication
 * sequence, including timing-critical start signals, data reception, and
 * final checksum validation. It must be called from a FreeRTOS task.
 *
 * @note The entire read sequence is protected by a FreeRTOS critical section
 * (`taskENTER_CRITICAL()`) to prevent task preemption and ensure accurate
 * microsecond timing using `ets_delay_us()`.
 *
 * @param pin The GPIO number configured by dht11_init().
 * @param p_data Pointer to a dht11_data_t struct where the results will be stored.
 * @return BaseType_t pdTRUE on successful read and checksum validation, pdFALSE on failure (timeout or bad checksum).
 */
BaseType_t dht11_read(gpio_num_t pin, dht11_data_t *p_data);




#endif // DHT_H