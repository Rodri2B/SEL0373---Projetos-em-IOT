#pragma once

// Incluir esta lib antes de timers.h
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include <string.h>

//start and stop interrupts gpio constants
#define START_MOTOR_INTERRUPT_PIN GPIO_NUM_34
#define STOP_MOTOR_INTERRUPT_PIN GPIO_NUM_35


//pwm gpio constants
#define ENABLE_A GPIO_NUM_32
#define INPUT_1 GPIO_NUM_33
#define INPUT_2 GPIO_NUM_25

#define ENABLE_DIR ENABLE_A

#define ENABLE_B GPIO_NUM_4
#define INPUT_3 GPIO_NUM_18
#define INPUT_4 GPIO_NUM_19

#define ENABLE_ESQ ENABLE_B


//servo pwm constants

#define SERVO_DUTY_PIN 13
#define SERVO_PWM_TIMER LEDC_TIMER_2//pwm channel for servo
#define SERVO_PWM_CHANNEL LEDC_CHANNEL_2
#define SERVO_INITIAL_ANGLE 113.12
#define SERVO_OFFSET -3.5//-3.5

//hardware definitions for pwm 
#define PWM_FREQ 2000
#define MAX_DUTY 8191

