
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueue;
SemaphoreHandle_t xSemaphoreLed_r;
SemaphoreHandle_t xSemaphoreLed_y;

void btn_callback(uint gpio, uint32_t events){
    uint32_t btn_pin = gpio;
    xQueueSendFromISR(xQueue, &btn_pin,0);
}

void btn_handler_task(void *p){
    uint32_t btn_pin;
    while (true){
        if(xQueueReceive(xQueue, &btn_pin, 0)){
            if(btn_pin == BTN_PIN_R){
                xSemaphoreGive(xSemaphoreLed_r);
            } else if (btn_pin == BTN_PIN_Y){
                xSemaphoreGive(xSemaphoreLed_y);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void led_1_task(void *p){
    int delay = 250;
    bool ta_piscando_v = false;

    while (true){
        if(xSemaphoreTake(xSemaphoreLed_r, pdMS_TO_TICKS(100)) == pdTRUE){
            ta_piscando_v = !ta_piscando_v;
        }

        if(ta_piscando_v){
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        } else{
            gpio_put(LED_PIN_R,0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
  
}

void led_2_task(void *p){
    int delay = 250;
    bool ta_piscando_a = false;

    while (true){
        if(xSemaphoreTake(xSemaphoreLed_y, pdMS_TO_TICKS(100)) == pdTRUE){
            ta_piscando_a = !ta_piscando_a;
        }

        if(ta_piscando_a){
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        } else{
            gpio_put(LED_PIN_Y,0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
  
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0); 
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0); 

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    xQueue = xQueueCreate(10, sizeof(uint32_t));
    xSemaphoreLed_r = xSemaphoreCreateBinary();
    xSemaphoreLed_y = xSemaphoreCreateBinary();

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_handler_task, "BTN_Handler",256,NULL,1,NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}
