#pragma once

#include "types.h"

//pwm initializers 

ledc_timer_config_t ledc_timer_config_info(ledc_timer_t timer_id);

ledc_channel_config_t ledc_channel_config_info(
    gpio_num_t pwm_pin,
    ledc_channel_t channel,
    ledc_timer_t timer_id);

//SPI initializers

spi_bus_config_t bus_config_info();

spi_device_interface_config_t dev_config_info();

//RFID

rc522_spi_config_t rc522_spi_driver_config_info();

//************************************************************************************************************//

//not used on the code

ledc_timer_config_t ledc_servo_timer_config_info(ledc_timer_t timer_id);

servo_config_t servo_config_info(ledc_timer_t timer_id,ledc_channel_t channel,gpio_num_t servo_pin);

//**********************************************************************************************************//


//gpio pin default config
extern gpio_config_t gpio_default_config; //not used



