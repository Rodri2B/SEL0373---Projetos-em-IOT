#pragma once

#include "types.h"

//void dump_block(uint8_t buffer[RC522_MIFARE_BLOCK_SIZE]);
/*
static esp_err_t card_read(rc522_handle_t scanner, rc522_picc_t *picc);

static esp_err_t card_write(rc522_handle_t scanner, rc522_picc_t *picc);
*/
//esp_err_t read_write(rc522_handle_t scanner, rc522_picc_t *picc);

void on_picc_state_changed(void *arg_scanner_addr, esp_event_base_t base, int32_t event_id, void *data);

//void rfid_rc522_init();