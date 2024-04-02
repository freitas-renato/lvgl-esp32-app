/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "lvgl.h"

#include "lvgl_helpers.h"

#include "homescreen.h"



#define LV_TICK_PERIOD_MS 1

static const char* TAG = "MAIN";

static void lv_tick_task(void* arg);
static void gui_task(void* arg);
static void lvgl_init_application(void);

void app_main(void) {
    ESP_LOGI(TAG, "Hello world!");
    // printf("Hello world!\n");

    // /* Print chip information */
    // esp_chip_info_t chip_info;
    // esp_chip_info(&chip_info);
    // ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s, ",
    //         CONFIG_IDF_TARGET,
    //         chip_info.cores,
    //         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    //         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    // ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    // ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
    //         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // ESP_LOGI(TAG, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    // Create LVGL task
    xTaskCreatePinnedToCore(gui_task, "GUI", 4096*2, NULL, 0, NULL, 1);
}


// If we want to call any lvgl function form another task, take a look at this semaphore first
SemaphoreHandle_t lvgl_gui_semaphore;


static void gui_task(void* arg) {
    (void)arg;  // Unused argument

    lvgl_gui_semaphore = xSemaphoreCreateMutex();

    // Initialize the display
    lv_init();

    // Init SPI bus
    lvgl_driver_init();

    // Allocate two framebuffers
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_draw_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

    // Initialize framebuffer using double buffering method
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    // Initialize the display
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;  // using lvgl esp32 display driver preset for ILI9341
    disp_drv.draw_buf = &disp_buf;

    lv_disp_drv_register(&disp_drv);



    // Create timer to call lv_tick_task on interrupt
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_lvgl_tick_timer"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));  // LV_TICK_PERIOD comes from menuconfig


    // Create a basic application
    lvgl_init_application();

    // Enter main loop
    for (;;) {
        if (xSemaphoreTake(lvgl_gui_semaphore, portMAX_DELAY) == pdTRUE) {
            lv_task_handler();
            xSemaphoreGive(lvgl_gui_semaphore);
            // vTaskDelay(LV_TICK_PERIOD_MS / portTICK_PERIOD_MS);
        }
    }

    //! We should never get here

    // Clean up
    free(buf1);
    free(buf2);

    vTaskDelete(NULL);
}






static void lvgl_init_application(void) {
    homescreen_init();
}

static void lv_tick_task(void* arg) {
    (void)arg;  // Unused argument

    // Call lv_tick_inc to increment the tick
    lv_tick_inc(LV_TICK_PERIOD_MS);
}
