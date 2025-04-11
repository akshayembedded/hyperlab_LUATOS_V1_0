#ifndef CONFIG_H
#define CONFIG_H

#include "debug.h"


#define DEFAULT_HOMING_OFFSET 69.0f

#define AUTOSHUTDOWN_TIME 10*60 // 10 minutes
#define BATTERY_LOWER_LIMIT 6.5
#define BATTERY_UPDATE_TIME 5 * 60 * 1000 // m5 minutes

// Pin Definitions
#define LASER_PIN 15
#define BUZZER_PIN 4

// NeoPixel Configuration
#define NUM_LEDS 3
#define PIXEL_PIN 21

#define LATCH_PIN 23
#define BUTTON_PIN 22

#define BQ25887_SCL 5
#define BQ25887_SDA 14
#define BQ25887_INT 35

#define CHARGER_DETECT_PIN 36
#define BATTERY_VOLTAGE_PIN 35

// Motor X Configuration
#define MOTOR_X_DIR_PIN  (uint8_t)25
#define MOTOR_X_STEP_PIN  (uint8_t)33
#define MOTOR_X_EN_PIN  (uint8_t)32
#define MOTOR_X_SENSOR_PIN  (uint8_t)34
#define MOTOR_X_SENSOR_DETECT (bool)1
#define DEFAULT_HOMINGX_OFFSET_DEGREE 66.69
// Motor Y Configuration
#define MOTOR_Y_DIR_PIN  (uint8_t)27
#define MOTOR_Y_STEP_PIN  (uint8_t)26
#define MOTOR_Y_EN_PIN  (uint8_t)13
#define MOTOR_Y_SENSOR_PIN  (uint8_t)39
#define MOTOR_Y_SENSOR_DETECT (bool)0

// TF Mini-Plus Lidar Configuration
#define TF_MINI_RX_PIN 16
#define TF_MINI_TX_PIN 17



// Device Constants
#define DEVICE_HEIGHT 264.00
#define DEVICE_HEIGHT_MM DEVICE_HEIGHT

// Battery Configuration
#define BATTERY_LOWER_LIMIT 6.5
#define BATTERY_UPPER_LIMIT 8.40

// Bluetooth Configuration
#define BLE_NAME_PREFIX "HELIOS"
#define BLE_NAME_LENGTH 20

// OTA Configuration
#define OTA_SERVICE_UUID "fb1e4001-54ae-4a28-9f74-dfccb248601d"
#define OTA_UUID_RX "fb1e4002-54ae-4a28-9f74-dfccb248601d"
#define OTA_UUID_TX "fb1e4003-54ae-4a28-9f74-dfccb248601d"

// Default Motor Settings
#define DEFAULT_MOTOR_X_STEPS_PER_REV 3200
#define DEFAULT_MOTOR_Y_STEPS_PER_REV 400
#define DEFAULT_MOTOR_X_GEAR_RATIO 0.267
#define DEFAULT_MOTOR_Y_GEAR_RATIO 0.4
#define DEFAULT_MOTOR_X_SPEED 400
#define DEFAULT_MOTOR_Y_SPEED 850
#define DEFAULT_MOTOR_X_ACCELERATION 0.4
#define DEFAULT_MOTOR_Y_ACCELERATION 0.4
#define DEFAULT_MOTOR_X_CURRENT_LIMIT 5000
#define DEFAULT_MOTOR_Y_CURRENT_LIMIT 4000

// Homing Configuration
#define DEFAULT_X_HOMING_STEP_LOWER_LIMIT 2000
#define DEFAULT_X_HOMING_STEP_UPPER_LIMIT 2500

// Other Constants
#define MTU_SIZE 512
#define MANUFACTURER_CODE 0xFFFF


// Firmware Version original
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 2
#define FIRMWARE_VERSION_PATCH 0

// Firmware Version new



// // Hardware Version
#define HARDWARE_VERSION_MAJOR uint8_t(1)
#define HARDWARE_VERSION_MINOR uint8_t(0)
#define HARDWARE_VERSION_PATCH uint8_t(0)



// // Function to combine version numbers into a single integer
// #define MAKE_VERSION(major, minor, patch) ((major << 16) | (minor << 8) | patch)

// // Combined version numbers
// #define FIRMWARE_VERSION MAKE_VERSION(FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH)
// #define HARDWARE_VERSION MAKE_VERSION(HARDWARE_VERSION_MAJOR, HARDWARE_VERSION_MINOR,  HARDWARE_VERSION_PATCH)



// #define HARDWARE_VERSION_IS(major, minor, patch) (HARDWARE_VERSION == MAKE_VERSION(major, minor, patch))

// //store hardware version
#define STORE_HARDWARE_VERSION() STORAGE.store("hardware_version", HARDWARE_VERSION)












#endif // CONFIG_H