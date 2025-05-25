#include "rfid.h"

static const char *TAG = "rc522-read-write";


static void dump_block(uint8_t buffer[RC522_MIFARE_BLOCK_SIZE])
{
    for (uint8_t i = 0; i < RC522_MIFARE_BLOCK_SIZE; i++) {
        esp_log_write(ESP_LOG_INFO, TAG, "%02" RC522_X " ", buffer[i]);
    }

    esp_log_write(ESP_LOG_INFO, TAG, "\n");
}

static void dump_block_lim(uint8_t buffer[RC522_MIFARE_BLOCK_SIZE],uint8_t buffer_size)
{
    for (uint8_t i = 0; i < buffer_size; i++) {
        esp_log_write(ESP_LOG_INFO, TAG, "%02" RC522_X " ", buffer[i]);
    }

    esp_log_write(ESP_LOG_INFO, TAG, "\n");
}

/*
static esp_err_t card_read(rc522_handle_t scanner, rc522_picc_t *picc)
{

}

static esp_err_t card_write(rc522_handle_t scanner, rc522_picc_t *picc)
{

}
*/
static esp_err_t read_write(rc522_handle_t scanner, rc522_picc_t *picc, uint32_t id_to_send)
{
    const uint8_t block_address = 4;
    rc522_mifare_key_t key = {
        .value = { RC522_MIFARE_KEY_VALUE_DEFAULT },
    };

    if (sizeof(id_to_send) > 16) {
        ESP_LOGW(TAG, "Please make sure that data length is no more than 16 bytes");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(rc522_mifare_auth(scanner, picc, block_address, &key), TAG, "auth fail");

    uint8_t read_buffer[RC522_MIFARE_BLOCK_SIZE];
    uint8_t write_buffer[RC522_MIFARE_BLOCK_SIZE];

    // Read
    ESP_LOGI(TAG, "Reading data from the block %d", block_address);
    ESP_RETURN_ON_ERROR(rc522_mifare_read(scanner, picc, block_address, read_buffer), TAG, "read fail");
    ESP_LOGI(TAG, "Current data:");
    dump_block(read_buffer);
    // ~Read

    // Write
    memcpy(write_buffer, &id_to_send, sizeof(id_to_send));

    ESP_LOGI(TAG, "Writing data to the block %d:", block_address);
    dump_block(write_buffer);
    ESP_RETURN_ON_ERROR(rc522_mifare_write(scanner, picc, block_address, write_buffer), TAG, "write fail");
    // ~Write

    // Read again
    ESP_LOGI(TAG, "Write done. Verifying...");
    ESP_RETURN_ON_ERROR(rc522_mifare_read(scanner, picc, block_address, read_buffer), TAG, "read fail");
    ESP_LOGI(TAG, "New data in the block %d:", block_address);
    dump_block(read_buffer);
    // ~Read again

    // Validate
    bool rw_missmatch = false;
    uint8_t i;
    for (i = 0; i < RC522_MIFARE_BLOCK_SIZE; i++) {
        if (write_buffer[i] != read_buffer[i]) {
            rw_missmatch = true;
            break;
        }
    }
    // ~Validate

    // Feedback
    if (!rw_missmatch) {
        ESP_LOGI(TAG, "Verified.");
    }
    else {
        ESP_LOGE(TAG,
            "Write failed. RW missmatch on the byte %d (w:%02" RC522_X ", r:%02" RC522_X ")",
            i,
            write_buffer[i],
            read_buffer[i]);

        dump_block(write_buffer);
        dump_block(read_buffer);
    }
    // ~Feedback

    return ESP_OK;
}

static esp_err_t read_tag(rc522_handle_t scanner, rc522_picc_t *picc)
{
    const uint8_t block_address = 4;
    rc522_mifare_key_t key = {
        .value = { RC522_MIFARE_KEY_VALUE_DEFAULT },
    };

    ESP_RETURN_ON_ERROR(rc522_mifare_auth(scanner, picc, block_address, &key), TAG, "auth fail");

    uint8_t read_buffer[RC522_MIFARE_BLOCK_SIZE];

    // Read
    ESP_LOGI(TAG, "Reading data from the block %d", block_address);
    ESP_RETURN_ON_ERROR(rc522_mifare_read(scanner, picc, block_address, read_buffer), TAG, "read fail");
    ESP_LOGI(TAG, "Current data:");
    dump_block_lim(read_buffer,4);

    cow_id_stamped id_read;

    memcpy(&id_read.cow_id, read_buffer,4);
    id_read.time_stamp = esp_timer_get_time();


    xQueueSend(msg_queue_read_tag, (void*)&id_read, pdMS_TO_TICKS(100));
    // ~Read

    return ESP_OK;
}


void on_picc_state_changed(void *arg_scanner_addr, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state != RC522_PICC_STATE_ACTIVE) {
        return;
    }

    //rc522_picc_print(picc);

    if (!rc522_mifare_type_is_classic_compatible(picc->type)) {
        ESP_LOGW(TAG, "Card is not supported by this example");
        return;
    }

    if (read_tag(*((rc522_handle_t*) arg_scanner_addr), picc) == ESP_OK) {
        ESP_LOGI(TAG, "Read success");
    }
    else {
        ESP_LOGE(TAG, "Read failed");
    }

    if (rc522_mifare_deauth(*((rc522_handle_t*) arg_scanner_addr), picc) != ESP_OK) {
        ESP_LOGW(TAG, "Deauth failed");
    }
}

void on_picc_state_changed_calibration(void *arg_scanner_addr, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)(((tag_calib_handle *)data)->scanner);
    uint32_t id_to_send = ((tag_calib_handle *)data)->id_to_write;
    rc522_picc_t *picc = event->picc;

    if (picc->state != RC522_PICC_STATE_ACTIVE) {
        return;
    }

    rc522_picc_print(picc);

    if (!rc522_mifare_type_is_classic_compatible(picc->type)) {
        ESP_LOGW(TAG, "Card is not supported by this example");
        return;
    }

    if (read_write(*((rc522_handle_t*) arg_scanner_addr), picc, id_to_send) == ESP_OK) {
        ESP_LOGI(TAG, "Read/Write success");

        tag_uid_handle uid_data;
        memcpy(uid_data.uid_buf,picc->uid.value,picc->uid.length);
        uid_data.uid_buf_length = picc->uid.length;
        xQueueSend(msg_queue_calibrate, (void*)&uid_data, pdMS_TO_TICKS(100));
    }
    else {
        ESP_LOGE(TAG, "Read/Write failed");
    }

    if (rc522_mifare_deauth(*((rc522_handle_t*) arg_scanner_addr), picc) != ESP_OK) {
        ESP_LOGW(TAG, "Deauth failed");
    }
}


void tag_calibration(uint32_t* id_block, uint32_t block_size, tag_calib_handle* calibration_handle){

    tag_uid_handle uid_handle;
    uint8_t last_uid[RC522_PICC_UID_SIZE_MAX] = {0};
    uint8_t last_uid_buf_size = RC522_PICC_UID_SIZE_MAX + 1;

    ESP_LOGI(TAG, "Starting Calibration");

    for(uint32_t i = 0; i < block_size; i++){
        ESP_LOGI(TAG, "Calibrating for id number: %" PRIu32 ", value: %" PRIu32, i+1U, id_block[i]);
        calibration_handle->id_to_write =  id_block[i];

        xQueueReset(msg_queue_calibrate);

        ESP_LOGI(TAG, "Approach the tag");

        while(true){

            if(pdTRUE == xQueueReceive(msg_queue_calibrate, (void*)&uid_handle, pdMS_TO_TICKS(100))){
                if(!((uid_handle.uid_buf_length ==  last_uid_buf_size) && (0 == memcmp( uid_handle.uid_buf, last_uid, uid_handle.uid_buf_length)))){

                    last_uid_buf_size = uid_handle.uid_buf_length;
                    memcpy(last_uid, uid_handle.uid_buf, uid_handle.uid_buf_length);
                    break;
                } 

                ESP_LOGI(TAG, "The same tag was scanned, please approach a different tag");
            }

            vTaskDelay(pdMS_TO_TICKS(200));
        } 

        ESP_LOGI(TAG, "Tag number: %" PRIu32 " calibrated" PRIu32, i+1U);
    }
}

//void rfid_rc522_init(){
//
//}