
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <Preferences.h>
#include "ble_controller.h"
#include "ble_handlers.h"
#include "NimBLEDevice.h"
void lua_loop(String script);
String luaStopScript = "";
bool isClosed = false;
extern BLEController bleController;

extern "C"
{
#include "luat_base.h"
#include "luat_timer.h"
#include "luat_uart.h"

#include <string.h>
#include <stdlib.h>
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"


#include "luat_rtos.h"

#include <time.h>
#include <sys/time.h>
extern lua_State *L;
#define LUAT_LOG_TAG "main"
#include "luat_log.h"

#ifdef LUAT_USE_LVGL
#include "lvgl.h"
#include "luat_lvgl.h"
#endif
}

extern "C"
{
extern int luat_main (void);
extern void bootloader_random_enable(void);
extern void luat_heap_init(void);
extern int luat_main_call(void);

#ifdef LUAT_USE_LVGL
static void luat_lvgl_callback(TimerHandle_t xTimer){
    lv_tick_inc(10);
    rtos_msg_t msg = {0};
    msg.handler = lv_task_handler;
    luat_msgbus_put(&msg, 0);
}
#endif

#ifdef LUAT_USE_NETWORK
#include "luat_network_adapter.h"
#endif
void app_main(void);
void luat_mcu_us_timer_init();
extern void luat_shell_poweron(int _drv);
}
#include "Arduino.h"

#define LED_PIN 13
void ble_task(void *pvParameter) {
    
    // pinMode(LED_PIN, OUTPUT);

    while (1) {
        processBLEMessages();
        vTaskDelay(10); // FreeRTOS delay (1000ms)
    }
}

void app_main(void){
    LLOGE("Started");
    NimBLEDevice::init("");
   vTaskPrioritySet( NULL, 10);
 
    luat_mcu_us_timer_init();
   
  
// #if defined(LUAT_USE_SHELL) || defined(LUAT_USE_REPL)
  
	// luat_shell_poweron(0);
// #endif



    bootloader_random_enable();
    esp_err_t r = nvs_flash_init();//28KB for both 4m and 8m
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        //ESP_LOGI("no free pages or nvs version mismatch, erase...");
        nvs_flash_erase();
        r = nvs_flash_init();
    }

    setenv("TZ", "IST+5:30", 1);
    tzset();

    luat_heap_init();
    esp_event_loop_create_default();
#ifdef LUAT_USE_LVGL
    lv_init();
    TimerHandle_t os_timer = xTimerCreate("lvgl_timer", 10 / portTICK_PERIOD_MS, true, NULL, luat_lvgl_callback);
    xTimerStart(os_timer, 0);
#endif


initializeBLEController("HyperLab");

    luat_main();
   
    
    xTaskCreatePinnedToCore(
        ble_task,    // Task function
        "ble_task",  // Task name
        8192,          // Stack size
        NULL,          // Parameters
        1,             // Priority
        NULL,           // Task handle
        1
    );
  
}


void luaClose(String closeScript)
{
    isClosed = true;
    closeScript.replace('\x01', ' ');
    luaStopScript = closeScript;
    // Serial.println("luaStopScript");
    // Serial.println(luaStopScript);
    // luaobj.stop();
    // TEST_PRINTLN("luaClose");
}

void lua_loop(String script)
{
    char charArray[2000]={0};
    script.toCharArray(charArray, script.length());

// Serial.println("lua_loop-----------------------------------------------------");
    // Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());
    // Serial.printf("Setup() - Free Stack Space: %d\n", uxTaskGetStackHighWaterMark(NULL));
    // Serial.printf("Total heap: %u\n", ESP.getHeapSize());
    // Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
 
     
LLOGI("lua_loop-----------------------------------------------------");
        // Use a try-catch block to handle Lua errors
       
            luaL_dostring(L,charArray);
           

        // luaobj.stop();
  
    // Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());
    // Serial.printf("Setup() - Free Stack Space: %d\n", uxTaskGetStackHighWaterMark(NULL));
    // Serial.printf("Total heap: %u\n", ESP.getHeapSize());
    // Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
  
    // Serial.println("-----------------------------------------------------");
}


/**
 *  NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE server library.
 *
 *  Created: on March 22 2020
 *      Author: H2zero
 */
