#include "initializers.h"

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

//*************************************************************************************** //


