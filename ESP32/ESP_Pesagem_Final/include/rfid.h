#pragma once

#include "types.h"

//void dump_block(uint8_t buffer[RC522_MIFARE_BLOCK_SIZE]);
/*
static esp_err_t card_read(rc522_handle_t scanner, rc522_picc_t *picc);

static esp_err_t card_write(rc522_handle_t scanner, rc522_picc_t *picc);
*/
//esp_err_t read_write(rc522_handle_t scanner, rc522_picc_t *picc);

struct cow_id_stamped {
    uint32_t cow_id;
    int64_t time_stamp;
};

typedef struct cow_id_stamped cow_id_stamped;

struct tag_calib_handle {
    rc522_handle_t* scanner;
    uint32_t id_to_write;
};

typedef struct tag_calib_handle tag_calib_handle;

struct tag_uid_handle {
    uint8_t uid_buf[RC522_PICC_UID_SIZE_MAX];
    uint8_t uid_buf_length;
};

typedef struct tag_uid_handle tag_uid_handle;

extern QueueHandle_t msg_queue_read_tag;
extern QueueHandle_t msg_queue_servo;
extern QueueHandle_t msg_queue_calibrate;

void on_picc_state_changed(void *arg_scanner_addr, esp_event_base_t base, int32_t event_id, void *data);
void on_picc_state_changed_calibration(void *arg_scanner_addr, esp_event_base_t base, int32_t event_id, void *data);

void tag_calibration(uint32_t* id_block, uint32_t block_size, tag_calib_handle* calibration_handle);

//void rfid_rc522_init();