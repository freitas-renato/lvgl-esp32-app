/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "esp_log.h"

#include "esp_freertos_hooks.h"
#include "driver/gpio.h"

#include "lvgl.h"
// #include "lvgl/lvgl.h"
#include "lvgl_helpers.h"



#define LV_TICK_PERIOD_MS 1

static const char* TAG = "MAIN";

static void lv_tick_task(void* arg);
static void gui_task(void* arg);
static void create_hello_world_app(void);

void app_main(void) {
    ESP_LOGI(TAG, "Hello world!");
    // printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());


    // Create LVGL task
    xTaskCreatePinnedToCore(gui_task, "GUI", 4096*2, NULL, 0, NULL, 1);

    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();
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
    // // Initialize the touchpad
    // lv_indev_drv_t indev_drv;
    // lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_POINTER;
    // indev_drv.read = lv_touchpad_read;
    // lv_indev_drv_register(&indev_drv);

    // Allocate two framebuffers
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

    // Initialize framebuffer using double buffering method
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    // Initialize the display
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;  // using lvgl esp32 display driver preset for ILI9341
    disp_drv.buffer = &disp_buf;

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
    create_hello_world_app();

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






static void create_hello_world_app(void) {
    static lv_style_t screen_style;
    lv_style_init(&screen_style);
    lv_style_set_bg_color(&screen_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_style_set_border_width(&screen_style, LV_STATE_DEFAULT, 0);
    lv_style_set_border_color(&screen_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_style_set_radius(&screen_style, LV_STATE_DEFAULT, 0);

    // Create background rectangle
    lv_obj_t* scr = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(scr, 240, 320);
    lv_obj_add_style(scr, LV_OBJ_PART_MAIN, &screen_style);
    lv_scr_load(scr);


    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_align_mid(btn, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label = lv_label_create(btn, NULL);   // label on the button
    lv_label_set_text(label, "Hello world!");   // Set label text
    lv_obj_align_mid(label, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_center(label);  // Center horizontally and vertically
}

static void lv_tick_task(void* arg) {
    (void)arg;  // Unused argument

    // Call lv_tick_inc to increment the tick
    lv_tick_inc(LV_TICK_PERIOD_MS);
}
