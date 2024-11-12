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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

static const char *TAG = "HIDH";
uint8_t joycon_address[6] = {0x98, 0xB6, 0xE9, 0x89, 0x8A, 0x34};

void print_binary(uint8_t num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        // 上位ビットから順番に出力
        int bit = (num >> i) & 1;  // numのi番目ビットを抽出
        printf("%d", bit);
    }
    printf(" ");  // 各数字の後にスペースを追加
}

int servo_state = 0;
#define BLUE_LED GPIO_NUM_17

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
                gpio_set_level(BLUE_LED, 0);
                return;
            }
            ESP_LOGI(TAG, "HID device connected: %02X:%02X:%02X:%02X:%02X:%02X", 
                param->open.bd_addr[0], param->open.bd_addr[1], param->open.bd_addr[2], 
                param->open.bd_addr[3], param->open.bd_addr[4], param->open.bd_addr[5]);
            gpio_set_level(BLUE_LED, 1);
            break;
        case ESP_HIDH_CLOSE_EVT:  // HIDデバイスが切断されたとき
            ESP_LOGI(TAG, "HID device disconnected");
            gpio_set_level(BLUE_LED, 0);
            break;
        case ESP_HIDH_DATA_EVT:  // HIDデバイスからデータを受信したとき
            ESP_LOGI(TAG, "Received HID data, length: %d", param->data_ind.len);
            break;
        case ESP_HIDH_DATA_IND_EVT:  // HIDデバイスからデータを受信したとき
            // for(int i = 0; i < 5; i++){
            // print_binary(param->data_ind.data[3]);
            // }
            printf("\n");
            uint8_t up   = 0b00000010;
            uint8_t down = 0b00000110;
            uint8_t stop = 0b00001000;
            if(param->data_ind.data[3] == up){
                ESP_LOGI(TAG, "UP");
                servo_state = 1;
            }else if(param->data_ind.data[3] == down){
                ESP_LOGI(TAG, "DOWN");
                servo_state = -1;
            }else if(param->data_ind.data[3] == stop){
                ESP_LOGI(TAG, "STOP");
                servo_state = 0;
            }
            break;
        default:
            ESP_LOGI(TAG, "Unhandled event: %d", event);
            break;
    }
}

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_FREQ 50
#define servoPinNum 5

int to_duty(int angle){
    const int PULTH_WIDTH_0_US = 853;
    const int PULTH_WIDTH_180_US = 2090;
    int duty_us = (PULTH_WIDTH_0_US + (PULTH_WIDTH_180_US - PULTH_WIDTH_0_US) * angle / 180);
    int duty = (duty_us * (1<<LEDC_TIMER_13_BIT) -1) / 20000;
    return duty;
}

void servo_task(void *pvParameters){
    const int servoPins[servoPinNum] = {
        GPIO_NUM_5, GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22};

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    for(int i = 0; i < servoPinNum; i++){
        ledc_channel_config_t ledc_channel = {
            .gpio_num = servoPins[i],
            .speed_mode = LEDC_MODE,
            .channel = i,
            .timer_sel = LEDC_TIMER,
            .duty = 1000,
            .hpoint = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }
    int servo_angle = 90;
    while(1){
        vTaskDelay(10 / portTICK_PERIOD_MS);
        servo_angle += servo_state*1;
        const int max_angle = 120;
        const int min_angle = 60;
        if(servo_angle > max_angle) servo_angle = max_angle;
        if(servo_angle < min_angle) servo_angle = min_angle;
        // ESP_LOGI(TAG, "servo_angle: %d", servo_angle);

        for(int i = 0; i < servoPinNum; i++){
            int angle = servo_angle;
            if (i == 2 || i == 3) {
                angle = 180 - servo_angle;
            }
            if(i == 4){
                angle = (angle - 90)*0.5 + 90 -15;
            }
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, i, to_duty(angle)));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, i));
        }
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

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;    // 割り込みを無効
    io_conf.mode = GPIO_MODE_OUTPUT;          // 出力モードに設定
    io_conf.pin_bit_mask = (1ULL << BLUE_LED); // GPIOピンを指定
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    if((ret = gpio_config(&io_conf)) != ESP_OK){
        ESP_LOGE(TAG, "gpio_config failed, error code = %x", ret);
        return;

    }

    if((ret = esp_bt_hid_host_register_callback(hid_host_callback)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_bt_hid_host_register_callback failed, error code = %x", ret);
        return;
    }

    if((ret = esp_bt_hid_host_connect(joycon_address)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_bt_hid_host_connect failed, error code = %x", ret);
        return;
    }

    ESP_LOGI(TAG, "HID device connection initiaited");

    xTaskCreate(servo_task, "servo_task", 4096, NULL, 5, NULL);
}