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
} esp_wifi_state_t;

void         esp_at_init(void);
void         esp_wifi_init(void);
void         esp_wifi_query_state(esp_wifi_state_t *state);
esp_at_ack_t esp_at_send_cmd(const char cmd[], uint16_t timeout);

extern esp_wifi_state_t esp_wifi_state;


#endif
