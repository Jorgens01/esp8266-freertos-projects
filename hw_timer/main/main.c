/***************************************************************************
 * File name    :   main.c
 * Description  :   Main application entry for timer demo
 *
 * This file contains the main entry point (app_main) for the application.
 * Its sole responsibility is to initialize the custom hardware timer module.
 * This demonstrates a modular approach where the main application simply
 * starts the necessary components without being concerned with their
 * internal implementation details.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-19
 **************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer.h"

void app_main(void)
{
    hardware_timer_init();
}
