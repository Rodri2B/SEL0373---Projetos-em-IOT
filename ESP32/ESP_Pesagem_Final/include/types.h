#pragma once

// Incluir esta lib antes de timers.h
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/ledc.h"
#include "freertos/event_groups.h"
#include "iot_servo.h"
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "picc/rc522_mifare.h"
#include "driver/spi_master.h"
#include <string.h>
#include <math.h>
#include <inttypes.h>
//#include "rfid.h"
//***************************************************************************************//

//not used on te code

//servo pwm constants

#define SERVO_DUTY_PIN 13
#define SERVO_PWM_TIMER LEDC_TIMER_2//pwm channel for servo
#define SERVO_PWM_CHANNEL LEDC_CHANNEL_2
#define SERVO_INITIAL_ANGLE 0.0
#define SERVO_OFFSET 0.0//-3.5

//SPI constants

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS  -1 //not used
#define RC522_SPI_SCANNER_GPIO_SDA_CS 14
#define QUEUE_SIZE 3 //not used
#define SPI_CLK_FREQUENCY 2000000

#define ESP_SPI_HOST SPI3_HOST//VSPI_HOST

#define RC522_SCANNER_GPIO_RST     (-1) // soft-reset
//***************************************************************************************//