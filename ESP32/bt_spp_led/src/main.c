#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "driver/gpio.h"

#include "time.h"
#include "sys/time.h"

#include "initializers.h"
#include "freertos/queue.h"

#include <stdint.h>
#include <math.h>
//#include <stdio.h>

#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP32_BT"
#define SPP_SHOW_DATA 0b10
#define SPP_SHOW_SPEED 0b01
#define SPP_SHOW_MODE (SPP_SHOW_DATA)//(SPP_SHOW_DATA | SSP_SHOW_SPEED)/*Choose show mode: show data or speed*/

#define ledR 14
#define ledG 12
#define ledB 13 //2

QueueHandle_t msg_queue;
//#define ledpwm 18

const char *tag = "Bluetooth";

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;


void set_lightmode(int mode);

static void init_led(void)
{
    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);

    ESP_LOGI(tag, "Init led completed");  
}

static void print_speed(void)
{
    float time_old_s = time_old.tv_sec + time_old.tv_usec / 1000000.0;
    float time_new_s = time_new.tv_sec + time_new.tv_usec / 1000000.0;
    float time_interval = time_new_s - time_old_s;
    float speed = data_num * 8 / time_interval / 1024.0;
    ESP_LOGI(tag, "speed(%fs ~ %fs): %f kbit/s", time_old_s, time_new_s, speed);
    data_num = 0;
    time_old.tv_sec = time_new.tv_sec;
    time_old.tv_usec = time_new.tv_usec;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_INIT_EVT");
        esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(tag, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(tag, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(tag, "ESP_SPP_CLOSE_EVT");
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(tag, "ESP_SPP_START_EVT");
        esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
#if ((SPP_SHOW_MODE & 0b10) == SPP_SHOW_DATA)
        ESP_LOGI(tag, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);
        printf("Value received: ");
        char char_num[3];
        for (size_t i = 0; i < (param->data_ind.len) - 2; i++)
        {   
            char value = param->data_ind.data[i];
            //printf("%c", value);

            if( value == 'X'){
                
                char_num[0] = param->data_ind.data[i+1];
                char_num[1] = param->data_ind.data[i+2];
                char_num[2] = param->data_ind.data[i+3];

                int num = atoi(char_num); 
                if(num >= 1 && num <=3) xQueueSend(msg_queue, (void*)&num, pdMS_TO_TICKS(10));

                /*
                switch (num)
                {
                case 1 :
                    //if (GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(ledR)){
	                //    //pin is output - read the GPIO_OUT_REG register
	                //    gpio_set_level(ledR, 1-((GPIO_REG_READ(GPIO_OUT_REG)  >> ledR) & 1U));
                    //} 
                    ////gpio_set_level(ledR, 1);
                    //gpio_set_level(ledG, 0);
                    //gpio_set_level(ledB, 0);
                    //set_lightmode(0);

                    break;
                case 2 :
                    //gpio_set_level(ledR, 0);
                    //if (GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(ledG)){
	                //    //pin is output - read the GPIO_OUT_REG register
	                //    gpio_set_level(ledG, 1-((GPIO_REG_READ(GPIO_OUT_REG)  >> ledG) & 1U));
                    //}                 
                    ////gpio_set_level(ledG, 1);
                    //gpio_set_level(ledB, 0);
                    //set_lightmode(1);
                    break;
                case 3 :
                    //gpio_set_level(ledR, 0);
                    //gpio_set_level(ledG, 0);
                    //if (GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(ledB)){
	                //    //pin is output - read the GPIO_OUT_REG register
	                //    gpio_set_level(ledB, 1-((GPIO_REG_READ(GPIO_OUT_REG)  >> ledB) & 1U));
                    //} 
                    ////gpio_set_level(ledB, 1);
                    //set_lightmode(2);
                    break;
                default:
                    break;
                }
                */
            }
        }
        printf("\n");

        //char rgb_value[3] = {char_num[0],char_num[1],char_num[2]}; 
        //int num = atoi(rgb_value);

        //if(num >= 0 && num <=255) xQueueSend(msg_queue, (void*)&num, 0);

        esp_spp_write(param->data_ind.handle, param->data_ind.len, param->data_ind.data);
#endif

#if ((SPP_SHOW_MODE & 0b01) == SPP_SHOW_SPEED)

        gettimeofday(&time_new, NULL);
        data_num += param->data_ind.len;
        if (time_new.tv_sec - time_old.tv_sec >= 0.01)
        {
            print_speed();
        }
#endif
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(tag, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(tag, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(tag, "ESP_SPP_SRV_OPEN_EVT");
        gettimeofday(&time_old, NULL);
        break;
    case ESP_SPP_SRV_STOP_EVT:
        ESP_LOGI(tag, "ESP_SPP_SRV_STOP_EVT");
        break;
    case ESP_SPP_UNINIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_UNINIT_EVT");
        break;
    default:
        break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
    {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(tag, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(tag, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        }
        else
        {
            ESP_LOGE(tag, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:
    {
        ESP_LOGI(tag, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit)
        {
            ESP_LOGI(tag, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        }
        else
        {
            ESP_LOGI(tag, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
        break;

    default:
    {
        ESP_LOGI(tag, "event: %d", event);
        break;
    }
    }
    return;
}


/*
void hsv2rgb(int val)
{

    ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0 , val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
*/

void hsv_to_rgb(float h, float s, float v, uint8_t *rgb) {
    // Calculate chroma
    float c = v * s;
    
    // Normalize hue to the [0, 360) range and scale to [0,6)
    float hh = fmodf(h, 360.0f) / 60.0f;
    
    // Compute intermediate value x
    float x = c * (1 - fabsf(fmodf(hh, 2) - 1));
    
    // Adjustment to match the value (brightness)
    float m = v - c;
    
    // Temporary variables for r', g', and b'
    float r_temp, g_temp, b_temp;
    
    if (hh >= 0 && hh < 1) {
        r_temp = c;
        g_temp = x;
        b_temp = 0;
    } else if (hh >= 1 && hh < 2) {
        r_temp = x;
        g_temp = c;
        b_temp = 0;
    } else if (hh >= 2 && hh < 3) {
        r_temp = 0;
        g_temp = c;
        b_temp = x;
    } else if (hh >= 3 && hh < 4) {
        r_temp = 0;
        g_temp = x;
        b_temp = c;
    } else if (hh >= 4 && hh < 5) {
        r_temp = x;
        g_temp = 0;
        b_temp = c;
    } else {  // hh >= 5 && hh < 6
        r_temp = c;
        g_temp = 0;
        b_temp = x;
    }
    
    // Convert to RGB by adding m and scaling to 0–255 range
    rgb[0] = (uint8_t)((r_temp + m) * 255);
    rgb[1] = (uint8_t)((g_temp + m) * 255);
    rgb[2] = (uint8_t)((b_temp + m) * 255);
}

void set_led_pwm(void *params)
{
    int val_mode = 0;

    //initializing pwm for led R
    ledc_timer_config_t timer_config1 = ledc_timer_config_info(LEDC_TIMER_0);
    ledc_channel_config_t channel_config1 = ledc_channel_config_info(ledR,LEDC_CHANNEL_0,LEDC_TIMER_0);
    
    ledc_timer_config(&timer_config1);
    ledc_channel_config(&channel_config1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0 , 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

   //initializing pwm for led G
   ledc_timer_config_t timer_config2 = ledc_timer_config_info(LEDC_TIMER_1);
   ledc_channel_config_t channel_config2 = ledc_channel_config_info(ledR,LEDC_CHANNEL_1,LEDC_TIMER_1);
   
   ledc_timer_config(&timer_config2);
   ledc_channel_config(&channel_config2);
   
   ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
   ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

   //initializing pwm for led B
   ledc_timer_config_t timer_config3 = ledc_timer_config_info(LEDC_TIMER_2);
   ledc_channel_config_t channel_config3 = ledc_channel_config_info(ledR,LEDC_CHANNEL_2,LEDC_TIMER_2);
   
   ledc_timer_config(&timer_config3);
   ledc_channel_config(&channel_config3);
   
   ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
   ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

   float h_rainbow = 0.0f;
   float v_wave = 0.0f;
   //uint8_t v_blink = 0; 
   
   uint8_t color[3];

   float dir = 1.0f;
   float inc = 0.1f;

   while(true){
    xQueueReceive(msg_queue, (void*)&val_mode, 0);
    switch (val_mode)
    {
    case 1: 
        
        timer_config1 = ledc_timer_config_info_f(LEDC_TIMER_0,5);
        ledc_timer_config(&timer_config1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
 
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        //v_blink = (v_blink == 255) ? 0 : 255;
        val_mode = 0;
        break;
    case 2:

        if(timer_config1.freq_hz == 5){
            timer_config1 = ledc_timer_config_info(LEDC_TIMER_0);
            ledc_timer_config(&timer_config1);
        }

        hsv_to_rgb(h_rainbow,1.0,v_wave,color);

        timer_config1 = ledc_timer_config_info(LEDC_TIMER_0);
        ledc_timer_config(&timer_config1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, color[0]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, color[1]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
 
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, color[2]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        v_wave += dir*inc;

        if(v_wave > 1.0f){
            dir = -1.0f;
            v_wave = 1.0f;
        }
        if(v_wave < 0.0f){
            dir = 1.0f;
            v_wave = 0.0f;
        }

        break;
    case 3:

    if(timer_config1.freq_hz == 5){
        timer_config1 = ledc_timer_config_info(LEDC_TIMER_0);
        ledc_timer_config(&timer_config1);
    }

        hsv_to_rgb(h_rainbow,1.0,1.0,color);


        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, color[0]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, color[1]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, color[2]);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        h_rainbow += inc*360.0f;

        if(h_rainbow > 360.0f){
            h_rainbow = 0.0f;
        }

        break;
    }
    //set_lightpwm(val_mode);
    vTaskDelay(pdMS_TO_TICKS(100));
   }

}



/*
void light_task(void *params){

    int value = 0;
    int saturation = 255;
    int angle = 0;
    int sign = 1;
    int lightmode = 0;

    while(true){

        ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0 , value);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);


        value += sign*25;
        if(value > 255){
            sign = -1*sign;
            value = 255;
        } 
        if(value < 0){
            sign = -1*sign;
            value = 0;
        } 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
*/

void app_main(void)
{
    init_led();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(tag, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(tag, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    msg_queue = xQueueCreate(10, sizeof(int)); 

    xTaskCreatePinnedToCore(&set_led_pwm, "task que apled", 2048, NULL, 1, NULL, 1);

}
