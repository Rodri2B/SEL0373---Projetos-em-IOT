#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "HX711.h"

#define TAG "TWAI"
#define AVG_SAMPLES   10
#define GPIO_DATA   GPIO_NUM_18
#define GPIO_SCLK   GPIO_NUM_15

//static const char* TAG = "HX711_TEST";

//int id[4] = {0, 0, 0, 0};

unsigned int weight =0;

static void weight_reading_task(void* arg);
static void initialise_weight_sensor(void);

static void weight_reading_task(void* arg)
{
    HX711_init(GPIO_DATA,GPIO_SCLK,eGAIN_128);
    HX711_tare();

    //unsigned long weight =0;
    //unsigned int weight =0;
    while(1)
    {
        weight = HX711_get_units(AVG_SAMPLES);
        ESP_LOGI(TAG, "******* weight = %d *********\n ", weight);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void initialise_weight_sensor(void)
{
    ESP_LOGI(TAG, "****************** Initialing weight sensor **********");
    xTaskCreatePinnedToCore(weight_reading_task, "weight_reading_task", 4096, NULL, 1, NULL,0);
}

void send_task(void *arg) {
    while (1) {
    	uint8_t buf[8];
        buf[0] = (weight >> 0)  & 0xFF;  // byte menos significativo
        buf[1] = (weight >> 8)  & 0xFF;
        buf[2] = (weight >> 16) & 0xFF;
        buf[3] = (weight >> 24) & 0xFF;  // byte mais significativo
        ESP_LOGI(TAG, "Message sent: Data=[%02X %02X %02X %02X]",
                 buf[0], buf[1], buf[2], buf[3]);
        twai_message_t message = {
            .identifier = 0x123,  // Set message ID
            .data_length_code = 8,  // Set data length
            .data = {buf[0], buf[1], buf[2], buf[3], 0x00, 0x00, 0x00, 0x00}  // Data to send
        };

        esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(50));
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Message sent successfully");
        } else {
            ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Send message every second
    }
}

void receive_task(void *arg) {
    while (1) {
        twai_message_t message;
        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(50));
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
        /*else if (err == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Reception timed out");
        } else {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
        }
        */
    }
}

void app_main() {
    // Configure TWAI driver
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // Set to 1 Mbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all messages

    // Install TWAI driver
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Start TWAI driver
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // HX711
    //nvs_flash_init();
    //initialise_weight_sensor();

    // Create tasks
   xTaskCreate(send_task, "send_task", 2048, NULL, 5, NULL);
   xTaskCreate(receive_task, "receive_task", 2048, NULL, 5, NULL);
}
