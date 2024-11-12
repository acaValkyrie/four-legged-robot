#include <stdio.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_hidh_api.h"
#include "esp_gap_bt_api.h"
#include "esp_log.h"
#include "esp_bt_defs.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "HIDH";
uint8_t joycon_address[6] = {0x98, 0xB6, 0xE9, 0x89, 0x8A, 0x34};

void print_binary(int num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        // 上位ビットから順番に出力
        int bit = (num >> i) & 1;  // numのi番目ビットを抽出
        printf("%d", bit);
    }
    printf(" ");  // 各数字の後にスペースを追加
}

// Bluetooth GAPコールバック関数
static void hid_host_callback(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param){
    switch (event) {
        case ESP_HIDH_INIT_EVT:  // HIDホストの初期化が完了したとき
            ESP_LOGI(TAG, "HID Host initialized successfully");
            break;
        case ESP_HIDH_OPEN_EVT:  // HIDデバイスが接続されたとき
            ESP_LOGI(TAG, "HID OPEN EVT STATUS %d, CONN STATUS %d", param->open.status, param->open.conn_status);
            if (param->open.status != ESP_HIDH_OK || param->open.conn_status != ESP_HIDH_CONN_STATE_CONNECTED) {
                ESP_LOGE(TAG, "HID device connection failed, error code = %x", param->open.status);
                return;
            }
            ESP_LOGI(TAG, "HID device connected: %02X:%02X:%02X:%02X:%02X:%02X", 
                param->open.bd_addr[0], param->open.bd_addr[1], param->open.bd_addr[2], 
                param->open.bd_addr[3], param->open.bd_addr[4], param->open.bd_addr[5]);
            break;
        case ESP_HIDH_CLOSE_EVT:  // HIDデバイスが切断されたとき
            ESP_LOGI(TAG, "HID device disconnected");
            break;
        case ESP_HIDH_DATA_EVT:  // HIDデバイスからデータを受信したとき
            ESP_LOGI(TAG, "Received HID data, length: %d", param->data_ind.len);
            break;
        case ESP_HIDH_DATA_IND_EVT:  // HIDデバイスからデータを受信したとき
            for(int i = 0; i < 5; i++){
                print_binary(param->data_ind.data[i]);
            }
            printf("\n");
            break;
        default:
            ESP_LOGI(TAG, "Unhandled event: %d", event);
            break;
    }
}

void app_main()
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    // Enable the Bluetooth controller
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize the Bluedroid stack
    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid stack initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    // Enable the Bluedroid stack
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid stack enable failed: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize the HID host
    if ((ret = esp_bt_hid_host_init()) != ESP_OK) {
        ESP_LOGE(TAG, "esp_hidh_init failed, error code = %x", ret);
        return;
    }

    ESP_LOGI(TAG, "HID Host initialized successfully");

    if((ret = esp_bt_hid_host_register_callback(hid_host_callback)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_bt_hid_host_register_callback failed, error code = %x", ret);
        return;
    }

    if((ret = esp_bt_hid_host_connect(joycon_address)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_bt_hid_host_connect failed, error code = %x", ret);
        return;
    }

    ESP_LOGI(TAG, "HID device connection initiaited");
}