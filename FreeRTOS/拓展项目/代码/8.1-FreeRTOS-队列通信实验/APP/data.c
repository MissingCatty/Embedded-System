#include "data.h"
#include <stdlib.h> // 用于 rand()

uint16_t counter = 0;

void random_sensor_data(SensorData_t *sensor_data)
{
    if (sensor_data == NULL)
        return;
    sensor_data->id        = ++counter;
    sensor_data->val       = rand() % 65536;
    sensor_data->timestamp = (uint32_t)rand();
}
