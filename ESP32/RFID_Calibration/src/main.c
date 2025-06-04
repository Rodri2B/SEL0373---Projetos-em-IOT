#include "types.h"
#include "initializers.h"
#include "rfid.h"

///////////////////////

#include "esp_system.h"
#include "esp_event.h"
#include "driver/twai.h"
///////////////////////


////////////////////////

#define TAG_TWAI "TWAI"
#define AVG_SAMPLES   10
#define GPIO_DATA   GPIO_NUM_15
#define GPIO_SCLK   GPIO_NUM_16

///////////////////////

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

//static tag_calib_handle calibration_handle = {
//    .scanner = &scanner,
//    .id_to_write = 0
//};


uint32_t id_block[2] = {0x00000001,
                        0x00000002};

uint32_t id_block_size = 2;

QueueHandle_t msg_queue_read_tag;
QueueHandle_t msg_queue_calibrate;
tag_calib_handle calibration_handle;

void app_main() {

    ///////////////////////////////////////
    
    // Configure TWAI driver
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // Set to 1 Mbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all messages

    // Install TWAI driver
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_TWAI, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG_TWAI, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Start TWAI driver
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG_TWAI, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG_TWAI, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    ///////////////////////////////////////

    msg_queue_read_tag = xQueueCreate(10, sizeof(cow_id_stamped));
    msg_queue_calibrate = xQueueCreate(10, sizeof(tag_uid_handle));




    //driver_config = rc522_spi_driver_config_info();
    
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };

    rc522_create(&scanner_config, &scanner);
    //rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, &scanner);

    calibration_handle.id_to_write = 0;

    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed_calibration, &scanner);
    rc522_start(scanner);

    //running calibration
    tag_calibration(id_block,id_block_size, &calibration_handle);



}
