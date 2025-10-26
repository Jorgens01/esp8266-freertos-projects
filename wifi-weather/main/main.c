#include <esp_err.h>
/**
 * @file        main.c
 * @brief       ESP8266 Wi-Fi Station (STA) Initialization and Connection Monitor.
 *
 * @details     This application initializes the ESP-IDF system, NVS (Non-Volatile Storage),
 * and the Wi-Fi stack. It configures the ESP8266 as a Wi-Fi Station, connects
 * to a predefined Access Point (AP), and uses a FreeRTOS Event Group for
 * synchronization and state management. A separate FreeRTOS task monitors
 * the connection status and logs system health (free heap).
 *
 * @note        Relies on configuration values (SSID, PASSWORD) defined
 * via the project's Kconfig file.
 *
 * @author      Jere Piirainen
 * @date        2025-10-26
 * @version     1.0.0
 */

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"


/* ============================================================================
 * CONFIGURATION - MODIFY THESE VALUES
 * ============================================================================ */

 // WiFi configuration
 #define WIFI_SSID          CONFIG_WIFI_SSID
 #define WIFI_PASS          CONFIG_WIFI_PASSWORD
 #define WIFI_MAX_RETRY     5

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

 #define MAX_HTTP_RECV_BUFFER   2048
 #define MAX_HTTP_OUTPUT_BUFFER 512

 static const char *TAG = "WEATHER";

 // FreeRTOS event group for WiFi events
 static EventGroupHandle_t s_wifi_event_group;

 #define WIFI_CONNECTED_BIT BIT0
 #define WIFI_FAIL_BIT      BIT1

 static int s_retry_num = 0;

/* ============================================================================
 * WIFI EVENT HANDLER
 * ============================================================================ */

 /**
 * @brief Handles asynchronous Wi-Fi and IP events from the ESP-IDF event loop.
 */
 static void event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WiFi started, attempting to connect...");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG, "Disconnected. Reason: %d", (int)event_id);
        if (s_retry_num < WIFI_MAX_RETRY)
        {
            // Reattempt connection
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to AP (%d/%d)", s_retry_num, WIFI_MAX_RETRY);
        }
        else 
        {
            // Max retries reached, signal failure to the waiting task.
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to AP after %d attempts", WIFI_MAX_RETRY);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "✓ SUCCESS! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "✓ Netmast; " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "✓ Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        s_retry_num = 0;

        // Signal successful connection to the waiting task.
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ============================================================================
 * WIFI INITIALIZATION - Blocking Wait for Connection
 * ============================================================================ */

 /**
 * @brief Initializes the Wi-Fi stack and blocks the calling task until 
 * connection succeeds or fails after max retries.
 * @return ESP_OK on successful connection, ESP_FAIL otherwise.
 */
 esp_err_t wifi_init_sta(void)
 {
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) 
    {
        ESP_LOGE(TAG, "Failed to create Wi-Fi Event Group. Out of memory?");
        return ESP_FAIL;
    }

    // Initialize the TCP/IP stack and default event loop.
    tcpip_adapter_init();
    esp_event_loop_create_default();

    // Initialize WiFi driver.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Register event handler for key WiFi and IP events
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                               &event_handler, NULL);

    // Set WiFi configuration and start driver.
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi initialization complete, waiting for connection...");

    // Wait for connection or failure
    EventBits_t bits = xEventGroupWaitBits(
                                        s_wifi_event_group,
                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                        pdFALSE,
                                        pdFALSE,
                                        portMAX_DELAY);

    // Check result and clean up
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "✓✓✓ STEP 1 PASSED: Connected to AP SSID: %s", WIFI_SSID);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "✗✗✗ STEP 1 FAILED: Could not connect to SSID: %s", WIFI_SSID);
        ESP_LOGE(TAG, "Check your SSID and password!");
        return ESP_FAIL;
    }

    // Should not even be reached with portMAX_DELAY, but for completeness.
    return ESP_FAIL;
}

/* ============================================================================
 * TEST TASK - SHOWS CONNECTION STATUS
 * ============================================================================ */

/**
 * @brief Periodically checks and logs the Wi-Fi connection status and free heap size.
 * @param pvParameters Not used.
 */
static void connection_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Connection monitor started");

    int count = 0;
    while (1)
    {   
        // Non-blocking read of the event group bits to check the current status.
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "[%d] WiFi Status: CONNECTED ✓", count++);
            // Logging free heap size crucial to monitor for memory leaks
            ESP_LOGI(TAG, "[%d] Free heap: %d bytes", count, esp_get_free_heap_size());
        } 
        else 
        {
            ESP_LOGW(TAG, "[%d] WiFi Status: DISCONNECTED ✗", count++);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/* ============================================================================
 * MAIN APPLICATION
 * ============================================================================ */

void app_main(void)
{
    ESP_LOGI(TAG, "=========================================");
    ESP_LOGI(TAG, "    STEP 1: WiFi Connection Test");
    ESP_LOGI(TAG, "=========================================");
    ESP_LOGI(TAG, "SDK Version: %s\n", esp_get_idf_version());
    ESP_LOGI(TAG, "What we're testing:");
    ESP_LOGI(TAG, "  1. WiFi initialization");
    ESP_LOGI(TAG, "  2. Connection to your AP");
    ESP_LOGI(TAG, "  3. Obtaining IP address");
    ESP_LOGI(TAG, "  4. Connection stability");
    ESP_LOGI(TAG, "=========================================");

    // Initialize NVS (Non-Volatile Storage) - required for Wi-Fi configuration storage.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // Handle NVS corruption or partition change by erasing and re-initializing.
        // This is a common solution but can be slow/wear flash memory.
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    // Check final result
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ NVS initialization failed!");
        return; // Halt if a critical component fails
    }
    ESP_LOGI(TAG, "✓ NVS initialized");

    // Initialize WiFi
    ESP_LOGI(TAG, "Connecting to SSID: %s", WIFI_SSID);
    if (wifi_init_sta() == ESP_OK) 
    {
        ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
        ESP_LOGI(TAG, "║   ✓✓✓ STEP 1 COMPLETE! ✓✓✓             ║");
        ESP_LOGI(TAG, "║   WiFi is working!                     ║");
        ESP_LOGI(TAG, "║   Ready for Step 2: DNS Test           ║");
        ESP_LOGI(TAG, "╚════════════════════════════════════════╝\n");

        xTaskCreate(connection_monitor_task, "monitor", 2048, NULL, 5, NULL);
    }
    else 
    {
        ESP_LOGE(TAG, "╔════════════════════════════════════════╗");
        ESP_LOGE(TAG, "║   ✗✗✗ STEP 1 FAILED! ✗✗✗               ║");
        ESP_LOGE(TAG, "║   Check these:                         ║");
        ESP_LOGE(TAG, "║   1. SSID is correct                   ║");
        ESP_LOGE(TAG, "║   2. Password is correct               ║");
        ESP_LOGE(TAG, "║   3. Router is 2.4GHz (not 5GHz)       ║");
        ESP_LOGE(TAG, "║   4. Router is powered on              ║");
        ESP_LOGE(TAG, "║   5. ESP8266 is in range               ║");
        ESP_LOGE(TAG, "╚════════════════════════════════════════╝\n");
    }
}
