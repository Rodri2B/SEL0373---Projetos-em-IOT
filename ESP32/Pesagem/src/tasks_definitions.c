#include "tasks_definitions.h"
#include "initializers.h"
#include "pwm.h"

spi_device_handle_t spi_handle;

void spi_transmit (uint8_t *data, int bytes)
{
	spi_transaction_t trans;
	memset(&trans, 0, sizeof(spi_transaction_t));
	
	trans.tx_buffer = data;
	trans.length = bytes*8;
	
	if (spi_device_transmit(spi_handle, &trans) != ESP_OK)
	{
		printf("writing error\n");	
	}
}

//task used to initialize variables and timers and function on core 0 (it is done only a single time)
void servo_actuate(void *params){
    
    //init pwm
    pwm_motors_init();

    //esp_err_t err;

    //spi_bus_config_t bus_cfg  = bus_config_info();
    //spi_device_interface_config_t dev_config = dev_config_info();

    //Initialize the SPI bus
    //err = spi_bus_initialize(ESP_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    //assert(err == ESP_OK);

    //add SPI device
    //err = spi_bus_add_device(ESP_SPI_HOST, &dev_config, &spi_handle);
    //assert(err == ESP_OK);

    while (true)
    {
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_PWM_CHANNEL, 50+SERVO_OFFSET);
        vTaskDelay(pdMS_TO_TICKS(1000));
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_PWM_CHANNEL, 100+SERVO_OFFSET);
        vTaskDelay(pdMS_TO_TICKS(1000));
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_PWM_CHANNEL, 170+SERVO_OFFSET);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
    }
    

    vTaskSuspend(NULL);

}

