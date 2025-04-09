#pragma once

#include "types.h"

//pwm initializers 

ledc_timer_config_t ledc_timer_config_info(ledc_timer_t timer_id);

ledc_timer_config_t ledc_timer_config_info_f(ledc_timer_t timer_id,uint32_t frequency);

ledc_channel_config_t ledc_channel_config_info(
    gpio_num_t pwm_pin,
    ledc_channel_t channel,
    ledc_timer_t timer_id);



