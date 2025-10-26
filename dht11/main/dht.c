/***************************************************************************
 * File name    :   dht11.c
 * Description  :   Driver implementation for the DHT11 sensor.
 *
 * This file contains the low-level GPIO manipulation and timing logic
 * required to communicate with the DHT11 sensor using its custom 1-wire
 * protocol. A FreeRTOS critical section is used in dht11_read() to
 * guarantee microsecond-level timing accuracy.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-26
 **************************************************************************/

#include "dht.h"
#include "freertos/task.h"  // taskENTER_CRITICAL/taskEXIT_CRITICAL
#include "rom/ets_sys.h"    // ets_delay_us()
#include "esp_log.h"
#include <string.h>         // for memset

static const char *TAG = "dht11_driver";

#define DHT11_START_SIGNAL_LOW_US 18000
#define DHT11_START_SIGNAL_HIGH_US 40
#define DHT11_RESPONSE_TIMEOUT_US 100
#define DHT11_BIT_READ_TIMEOUT_US 100
#define DHT11_BIT_1_THRESHOLD_US 40

/**
 *  Phase 1: MCU pulls signal from HIGH to LOW for at least 18 ms
 *  Phase 2: MCU pulls back HIGH and wait for 20-40 us for DHT11s response
 *  Phase 3: DHT11 sends 80 us long LOW signal for getting ready for data 
 *  Phase 4: DHT11 sends 40 bits of data starting from highest bit.
 *           Every bit starts with 50 us LOW signal and the next HIGH 
 *           tells if the bit is 0 or 1. (about 4 ms all in all)
 */


/**
 * @brief Helper function to busy-wait for the GPIO pin to change state.
 *
 * This function must be used within a critical section to prevent
 * preemption from corrupting the timing.
 *
 * @param pin The GPIO pin to check.
 * @param expected_state The state to wait for (0 for LOW, 1 for HIGH).
 * @param timeout_us Timeout in microseconds before abandoning the wait.
 * @return pdTRUE if the state was reached, pdFALSE on timeout.
 */
static BaseType_t dht11_wait_for_state(gpio_num_t pin, int expected_state, uint32_t timeout_us)
{
    uint32_t wait_time = 0;

    // Poll the pin level until matches expected state or timeout is reached
    while (gpio_get_level(pin) != expected_state) 
    {
        if (wait_time > timeout_us) 
        {
            return pdFALSE;
        }
        ets_delay_us(1);
        wait_time++;
    }
    return pdTRUE;
}


/**
 * @brief Reads the 40-bit data stream from the DHT11 after the response signal.
 *
 * This function is called within the critical section of dht11_read().
 * It is responsible for bit-by-bit reading and data assembly.
 *
 * @param pin The configured GPIO pin.
 * @param data Pointer to a 5-byte array to store the 40 bits of data (4 data bytes + 1 checksum byte).
 * @return pdTRUE if all 40 bits were read without timing out, pdFALSE otherwise.
 */
static BaseType_t dht11_read_data(gpio_num_t pin, uint8_t data[5])
{
    // Clear data buffer to ensure a clean start
    memset(data, 0, 5); 

    // ----- Acknowledge response phase -----
    // Wait for sensor's ~80us LOW response to end
    if (dht11_wait_for_state(pin, 1, DHT11_RESPONSE_TIMEOUT_US) == pdFALSE)
    {
        ESP_LOGE(TAG, "Read failed: Sensor response timed out");
        return pdFALSE;
    }

    // Wait for sensor's ~80us HIGH preface to end
    if (dht11_wait_for_state(pin, 0, DHT11_RESPONSE_TIMEOUT_US) == pdFALSE)
    {
        ESP_LOGE(TAG, "Read failed: Sensor response timed out");
        return pdFALSE;
    }

    // ----- Data reading phase (40 bits) -----
    for (int i = 0; i < 40; i++)
    {
        // Wait for 50us low bit-start pulse to end
        if (dht11_wait_for_state(pin, 1, 50) == pdFALSE)
        {
            ESP_LOGE(TAG, "Read failed: Bit %d timed out", i);
            return pdFALSE;
        }

        // Wait ~35 microseconds. This is the critical sampling point.
        // If the pin is HIGH after ~35us, it's a '1' bit (pulse is ~70us).
        // If the pin is LOW, it's a '0' bit (pulse is 26-28us).
        ets_delay_us(35);

        // Read the pin level and set the bit if it's HIGH.
        if (gpio_get_level(pin) == 1)
        {
            // Determine byte and bit index (MSB first)
            int byte_idx = i / 8;
            int bit_idx = 7 - (i % 8);
            data[byte_idx] |= (uint8_t)(1 << bit_idx);
        }
        // Wait for 1 bit's high pulse to end
        if (dht11_wait_for_state(pin, 0, DHT11_BIT_READ_TIMEOUT_US) == pdFALSE)
        {
            ESP_LOGE(TAG, "Read failed: Bit %d timed out", i);
            return pdFALSE;
        }

    }

    // 40 bits successfully read
    return pdTRUE;
}

/* ------------------------------------------------------------------------ */
/*                      PUBLIC FUNCTION IMPLEMENTATIONS                     */
/* ------------------------------------------------------------------------ */

esp_err_t dht11_init(gpio_num_t pin)
{
    // Configuration structure for the GPIO pin
    gpio_config_t io_conf = 
    {
        .intr_type = GPIO_INTR_DISABLE,         // Disable interrupts
        .mode = GPIO_MODE_OUTPUT_OD,            // Set to open-drain output mode
        .pin_bit_mask = (1ULL << pin),          // Select pin
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,       // Enable internal pull-up
    };
    return gpio_config(&io_conf);

}

BaseType_t dht11_read(gpio_num_t pin, dht11_data_t *p_data)
{
    uint8_t data[5] = {0};
    uint8_t checksum;
    BaseType_t read_success = pdFALSE;

    // ----- CRITICAL SECTION START -----
    // Disable interrupts to prevent the scheduler from preempting this task.
    // This is essential for maintaining accurate microsecond timing for the DHT11 protocol.
    taskENTER_CRITICAL();

    // Send start signal (MCU pulls line low for 18ms)
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
    ets_delay_us(DHT11_START_SIGNAL_LOW_US);

    // Release line (pull line HIGH for 40us)
    gpio_set_level(pin, 1);
    ets_delay_us(40);

    // Switch to input mode to lsiten for sensor response
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // Read 40 bits of data
    read_success = dht11_read_data(pin, data);

    // ----- CRITICAL SECTION END -----
    taskEXIT_CRITICAL();

    if (!read_success) 
    {
        return pdFALSE;
    }

    // Validate checksum and populate output structure
    checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum == data[4]) 
    {
        p_data->humidity = (int16_t)((data[0] * 10) | data[1]);
        p_data->temperature = (int16_t)((data[2] * 10) | data[3]);
        return pdTRUE;
    } else
    {
        return pdFALSE;
    }
}

