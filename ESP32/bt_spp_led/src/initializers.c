#include "initializers.h"

ledc_timer_config_t ledc_timer_config_info(ledc_timer_t timer_id){
    
    ledc_timer_config_t pwm_timer = {

        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = timer_id,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK

    };

    return pwm_timer;
}

ledc_timer_config_t ledc_timer_config_info_f(ledc_timer_t timer_id,uint32_t frequency){
    
    ledc_timer_config_t pwm_timer = {

        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = timer_id,
        .freq_hz = frequency,
        .clk_cfg = LEDC_AUTO_CLK

    };

    return pwm_timer;
}


//
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