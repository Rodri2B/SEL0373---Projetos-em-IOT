#include "initializers.h"

ledc_timer_config_t ledc_timer_config_info(ledc_timer_t timer_id){
    
    ledc_timer_config_t pwm_timer = {

        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = timer_id,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK

    };

    return pwm_timer;
}


ledc_channel_config_t ledc_channel_config_info(
    gpio_num_t pwm_pin,
    ledc_channel_t channel,
    ledc_timer_t timer_id){

    ledc_channel_config_t pwm_channel = {
        .gpio_num = (int) pwm_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer_id,
        .duty = 0,
        .hpoint = 0,
        .flags = {.output_invert = 0}
    };

    return pwm_channel;
}


spi_bus_config_t bus_config_info(){

    
    spi_bus_config_t config = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        //.max_transfer_sz = 32,
    };

    return config;

}

spi_device_interface_config_t dev_config_info(){

    spi_device_interface_config_t config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128,
        .clock_speed_hz = SPI_CLK_FREQUENCY,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = QUEUE_SIZE

    };

    return config;
}

//RFID
/*
rc522_spi_config_t rc522_spi_driver_config_info(){

    rc522_spi_config_t config = {
        .host_id = ESP_SPI_HOST,
        .bus_config = &(spi_bus_config_t){
            .miso_io_num = PIN_NUM_MISO,
            .mosi_io_num = PIN_NUM_MOSI,
            .sclk_io_num = PIN_NUM_CLK,
        },
        .dev_config = {
            .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA_CS,
        },
        .rst_io_num = RC522_SCANNER_GPIO_RST,
    };

    return config;
}
*/

//it's not used on the code, it's a default setting for servo pwm
//**************************************************************************** //
ledc_timer_config_t ledc_servo_timer_config_info(ledc_timer_t timer_id){
    
    ledc_timer_config_t pwm_timer = {

        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = timer_id,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK

    };

    return pwm_timer;
}

servo_config_t servo_config_info(ledc_timer_t timer_id,ledc_channel_t channel,gpio_num_t servo_pwm_pin){

    servo_config_t servo_cfg = {
    .max_angle = 180,
    .min_width_us = 500,
    .max_width_us = 2500,
    .freq = 50,
    .timer_number = timer_id,
    .channels = {
        .servo_pin = {
            servo_pwm_pin
        },
        .ch = {
            channel
        },
    },
    .channel_number = 1,
} ;

    return servo_cfg;
}

//*************************************************************************************** //


