#ifndef ESP_AT_H
#define ESP_AT_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    ESP_AT_ACK_NONE,
    ESP_AT_ACK_OK,
    ESP_AT_ACK_ERROR,
    ESP_AT_ACK_BUSY,
    ESP_AT_ACK_READY
} esp_at_ack_t;

typedef struct
{
    esp_at_ack_t ack;
    char         str[64];
} esp_at_ack_match_t;

typedef struct
{
    uint8_t state;
    char    ssid[64];
    char    bssid[64];
    int     channel;
    int     rssi;
} esp_wifi_state_t;

typedef struct
{
    uint8_t city[32];
    uint8_t weather[32];
    uint8_t weather_code;
    float   temperature;
} esp_weather_info_t;

void esp_at_init(void);
void esp_wifi_init(void);
bool esp_wifi_connected(void);
bool esp_at_wifi_connect(const char ssid[], const char pwd[]);
bool esp_send_weather_request(char key[], char location[], uint16_t timeout);
bool esp_sntp_init(void);
bool esp_sntp_sync(void);

extern esp_wifi_state_t esp_wifi_state;

#endif
