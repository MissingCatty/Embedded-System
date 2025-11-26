#include "stm32f4xx.h"

typedef struct
{
    uint16_t id;
    uint16_t val;
    uint32_t timestamp;
} SensorData_t;

void random_sensor_data(SensorData_t *sensor_data);
