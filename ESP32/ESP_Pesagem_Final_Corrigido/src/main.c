#include "types.h"
#include "tasks_definitions.h"
#include "initializers.h"
#include "rfid.h"

///////////////////////

#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/twai.h"
#include "HX711.h"
///////////////////////


////////////////////////

#define TAG_TWAI "TWAI"
#define AVG_SAMPLES   10
#define GPIO_DATA   GPIO_NUM_15
#define GPIO_SCLK   GPIO_NUM_16


//unsigned int weight =101;

//static void weight_reading_task(void* arg);

#define TAG_WEIGHT "WEIGHT_SENSOR"

static void initialise_weight_sensor(void);
void send_task(void *arg);
//void receive_task(void *arg);
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

static tag_calib_handle calibration_handle = {
    .scanner = &scanner,
    .id_to_write = 0
};


uint32_t id_block[2] = {0x00000001,
                        0x00000002};

uint32_t id_block_size = 2;

QueueHandle_t msg_queue_read_tag;
QueueHandle_t msg_queue_servo;
QueueHandle_t msg_queue_calibrate;

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
    
    // HX711
    nvs_flash_init();
    initialise_weight_sensor();

    xTaskCreatePinnedToCore(&send_task, "send_task", 4096, NULL, 1, NULL,1);
    //xTaskCreatePinnedToCore(&receive_task, "receive_task", 2048, NULL, 1, NULL,0);

    ///////////////////////////////////////

    msg_queue_read_tag = xQueueCreate(10, sizeof(cow_id_stamped));
    msg_queue_servo = xQueueCreate(10, sizeof(int));
    msg_queue_calibrate = xQueueCreate(10, sizeof(tag_uid_handle));


    //driver_config = rc522_spi_driver_config_info();
    
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };

    rc522_create(&scanner_config, &scanner);
    //rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed_calibration, &calibration_handle);
    rc522_start(scanner);

    //initializing tasks
    xTaskCreatePinnedToCore(&servo_actuate, "task que inicializa pwm, e faz controle do servo", 2048, NULL, 1, NULL, 1);
    //xTaskCreatePinnedToCore(&core1functions, "task que inicializa o i2c no core 1", 2048, NULL, 1, NULL, 1);
}

//static void weight_reading_task(void* arg)
//{
//}


static void initialise_weight_sensor(void)
{   
    HX711_init(GPIO_DATA,GPIO_SCLK,eGAIN_128);
    HX711_tare();
    ESP_LOGI(TAG_WEIGHT, "****************** Initialing weight sensor **********");
}

void send_task(void *arg) {

    cow_id_stamped id_read;
    int64_t last_time_stamp = 0;
    

    while (1) {



        if(pdTRUE == xQueueReceive(msg_queue_read_tag, (void*)&id_read, pdMS_TO_TICKS(100))){

            if((id_read.time_stamp - last_time_stamp) > 4000000){

    	        uint8_t buf[8];

                buf[0] = (id_read.cow_id >> 0)  & 0xFF;  // byte menos significativo
                buf[1] = (id_read.cow_id >> 8)  & 0xFF;
                buf[2] = (id_read.cow_id >> 16) & 0xFF;
                buf[3] = (id_read.cow_id >> 24) & 0xFF;  // byte mais significativo

                //ESP_LOGI(TAG, "Message sent: Data=[%02X %02X %02X %02X]",
                //         buf[0], buf[1], buf[2], buf[3]);

                
                ESP_LOGI(TAG_WEIGHT, "Waiting 2 seconds!");
                ESP_LOGI(TAG_WEIGHT, "Making weight measures!");
                vTaskDelay(pdMS_TO_TICKS(2000));

                int32_t weight = (int32_t)(HX711_get_units(AVG_SAMPLES)*1000.0);

                while( weight < 100000){
                    weight = (int32_t)(HX711_get_units(AVG_SAMPLES)*1000.0);
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                //ESP_LOGI(TAG, "******* weight = %d *********\n ", weight);

                buf[4] = (weight >> 0)  & 0xFF;  // byte menos significativo
                buf[5] = (weight >> 8)  & 0xFF;
                buf[6] = (weight >> 16) & 0xFF;
                buf[7] = (weight >> 24) & 0xFF;  // byte mais significativo

                twai_message_t message = {
                    .identifier = 0x123,  // Set message ID
                    .data_length_code = 8,  // Set data length
                    .data = {buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]}  // Data to send
                };

                //clear rx tx queues
                twai_clear_transmit_queue();
                //twai_clear_receive_queue();

                esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(500));
                if (err == ESP_OK) {

                    ESP_LOGI(TAG_TWAI, "Message sent successfully");

                    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(2000));
                    //ESP_LOGI(TAG,"Erro %d",err);
                    if (err == ESP_OK) {
                        //ESP_LOGI(TAG, "Message received: Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                         //        message.data[0], message.data[1], message.data[2], message.data[3],
                         //        message.data[4], message.data[5], message.data[6], message.data[7]);
                        int confirmation =  ((uint32_t)message.data[3] << 24) |
                                            ((uint32_t)message.data[2] << 16) |
                                            ((uint32_t)message.data[1] << 8)  |
                                            ((uint32_t)message.data[0]);

                    	ESP_LOGI(TAG_TWAI,"Confirmation: %d",confirmation);

                        xQueueSend(msg_queue_servo, (void*)&confirmation, pdMS_TO_TICKS(50));

                    }
                    else if (err == ESP_ERR_TIMEOUT) {
                        ESP_LOGW(TAG_TWAI, "Reception timed out");
                    } else {
                        ESP_LOGE(TAG_TWAI, "Message reception failed: %s", esp_err_to_name(err));
                    }
                
                } else {
                    ESP_LOGE(TAG_TWAI, "Message transmission failed: %s", esp_err_to_name(err));
                }

            }

            last_time_stamp = id_read.time_stamp;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // Send message every second
    }
}

/*
void receive_task(void *arg) {
    while (1) {
        twai_message_t message;
        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(50));
        //ESP_LOGI(TAG,"Erro %d",err);
        if (err == ESP_OK) {
            //ESP_LOGI(TAG, "Message received: Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
             //        message.data[0], message.data[1], message.data[2], message.data[3],
             //        message.data[4], message.data[5], message.data[6], message.data[7]);
            int weight =     ((uint32_t)message.data[3] << 24) |
                             ((uint32_t)message.data[2] << 16) |
                             ((uint32_t)message.data[1] << 8)  |
                             ((uint32_t)message.data[0]);
        	ESP_LOGI(TAG,"Valor: %d",weight);
        }
        else if (err == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Reception timed out");
        } else {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
        }
        

        vTaskDelay(pdMS_TO_TICKS(100));  // Send message every second
    }
}
*/