#include "types.h"
#include "tasks_definitions.h"
#include "initializers.h"
#include "rfid.h"

static rc522_driver_handle_t driver;
static rc522_handle_t scanner;
static rc522_spi_config_t driver_config = {

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


void app_main() {

    //driver_config = rc522_spi_driver_config_info();

    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };

    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, &scanner);
    rc522_start(scanner);

    //initializing tasks
    xTaskCreatePinnedToCore(&servo_actuate, "task que inicializa pwm, e faz controle do servo", 2048, NULL, 1, NULL, 1);
    //xTaskCreatePinnedToCore(&core1functions, "task que inicializa o i2c no core 1", 2048, NULL, 1, NULL, 1);
}

