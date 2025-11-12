#include "dht11.h"
#include "driver.h"
#include "FreeRTOS.h"
#include "task.h"

#define DHT11_PORT           GPIOA
#define DHT11_PIN            GPIO_Pin_6
#define DHT11_RCC            RCC_AHB1Periph_GPIOA
#define DHT11_TIMEOUT        100
#define MASTER_WRITE(status) GPIO_WriteBit(DHT11_PORT, DHT11_PIN, (BitAction)status)
#define MASTER_READ()        GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)

uint8_t dht11_humid_integer = 0;
uint8_t dht11_humid_decimal = 0;
uint8_t dht11_temp_integer  = 0;
uint8_t dht11_temp_decimal  = 0;
uint8_t dht11_checksum      = 0;

#define DHT11_TASK_PRIO 2             // 任务优先级
#define DHT11_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t DHT11Task_Handler;       // 任务句柄
void         dht11_task(void *p_arg); // 任务函数

void dht11_pin_init(void)
{
    RCC_AHB1PeriphClockCmd(DHT11_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);

    MASTER_WRITE(1); // 释放总线
}

void dht11_init(void)
{
    dht11_pin_init();
}

void _dht11_start(void)
{
    MASTER_WRITE(0);
    timer_delay_ms(18);
    MASTER_WRITE(1); // 释放总线
    timer_delay_us(30);
}

uint8_t _dht11_check_response(void)
{
    // 等待dht11拉低
    uint8_t timeout = DHT11_TIMEOUT;
    while (MASTER_READ())
    {
        timer_delay_us(1);
        timeout--;
        if (!timeout)
            return 0;
    }

    // dht11拉低响应（持续80us左右）
    timeout = DHT11_TIMEOUT;
    while (!MASTER_READ())
    {
        timer_delay_us(1);
        timeout--;
        if (!timeout)
            return 0;
    }

    // dht11拉高释放总线（持续80us左右）
    timeout = DHT11_TIMEOUT;
    while (MASTER_READ())
    {
        timer_delay_us(1);
        timeout--;
        if (!timeout)
            return 0;
    }
    return 1;
}

uint8_t dht11_receive_data(void)
{
    // 开启通信
    _dht11_start();

    // 接收响应
    if (!_dht11_check_response())
    {
        return 1; // 响应超时
    }

    // 接收数据
    uint8_t data[5] = {0x00};
    uint8_t timeout;

    for (uint8_t i = 0; i < 5; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            // 读取每一位之前，等待dht11拉低约50us
            timeout = DHT11_TIMEOUT;
            while (!MASTER_READ())
            {
                timer_delay_us(1);
                timeout--;
                if (!timeout)
                    return 1;
            }

            // 35us后读取数据，如果是高，说明是1；如果是低，说明是0
            timer_delay_us(35);
            if (MASTER_READ())
            {
                data[i] |= (1 << (7 - j));
                timeout = DHT11_TIMEOUT;
                while (MASTER_READ())
                {
                    timer_delay_us(1);
                    timeout--;
                    if (!timeout)
                        return 1;
                }
            }
        }
    }

    dht11_humid_integer = data[0];
    dht11_humid_decimal = data[1];
    dht11_temp_integer  = data[2];
    dht11_temp_decimal  = data[3];
    dht11_checksum      = data[4];

    if (dht11_checksum != (dht11_humid_integer + dht11_humid_decimal + dht11_temp_integer + dht11_temp_decimal))
    {
        return 1; // 校验和不正确
    }

    return 0; // 接收数据成功
}
