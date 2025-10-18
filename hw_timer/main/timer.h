/***************************************************************************
 * File name    :   timer.h
 * Description  :   Hardware timer module interface
 *
 * This header file defines the public interface for the ESP8266 hardware
 * timer module. It provides the initialization function required to set up
 * and start the timer, which uses a FreeRTOS task for deferred interrupt
 * processing.
 *
 * Author       :   Jere Piirainen
 * Date         :   2025-10-19
 **************************************************************************/

#ifndef TIMER_H
#define TIMER_H

/**
 * @brief Initializes the hardware timer and related FreeRTOS components.
 *
 * This function sets up a hardware timer to generate a periodic interrupt,
 * creates a binary semaphore for synchronization between the ISR and a task,
 * and creates the task that will be unblocked by the ISR to handle the timer event.
 */
void hardware_timer_init(void);

#endif // TIMER_H