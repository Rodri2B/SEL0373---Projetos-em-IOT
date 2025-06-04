#pragma once

#define TAG_SERVO "SERVO"

void pwm_motors_init();
void pwm_actuate(int channel, float duty);