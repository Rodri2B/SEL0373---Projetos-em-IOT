#pragma once

// Incluir esta lib antes de timers.h
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/event_groups.h"
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "picc/rc522_mifare.h"
#include "driver/spi_master.h"
#include <string.h>
#include <math.h>
#include <inttypes.h>
//#include "rfid.h"
//***************************************************************************************//

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