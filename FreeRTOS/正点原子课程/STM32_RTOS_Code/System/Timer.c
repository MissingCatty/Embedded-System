#include "Timer.h"

/* [定时器中断实验]
** (1) 实验描述：更新中断触发频率为1s一次，每次修改变量值，并显示在OLED屏幕上
** (2) 实验细节：使用TIM2计数
*/
#ifdef EXP_TIM_IT_UPDATE

uint8_t seconds = 0;

void Timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 开启内部时钟
    TIM_InternalClockConfig(TIM2);                       // 选择内部时钟作为时钟源

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;               // 自动重装寄存器
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;             // 预分频寄存器
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitTypeDef NVIC_InitStructre;
    NVIC_InitStructre.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructre.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructre.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructre.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructre);

    TIM_Cmd(TIM2, ENABLE);
}

uint8_t Timer_GetSeconds(void)
{
    return seconds;
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        seconds++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

#endif // DEBUG

/* [定时器输出比较实验]
** (1) 实验描述：输出固定占空比的PWM波
** (2) 实验细节：使用TIM2的通道2(PA1)输出波形
*/
#ifdef EXP_TIM_PWM

void Timer_Init(void)
{
    // 打开时钟，包括GPIO和TIM
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 配置GPIO功能复用
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 配置为复用推挽模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置定时器时基单元
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    // 配置输出比较单元
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出
    TIM_OCInitStructure.TIM_Pulse = 10000 / 2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平为有效电平
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    // 开启定时器
    TIM_Cmd(TIM2, ENABLE);
}

#endif // EXP_TIM_PWM

/* [定时器输入捕获实验]
** (1) 实验描述：测量PWM波的频率和占空比
** (2) 实验细节：使用TIM2的通道2(PA1)输出波形，使用TIM3的通道2(PA7)捕获PWM
*/
#ifdef EXP_TIM_IC
uint16_t cnt_t2 = 10000;      // TIM2计数器更新值
uint16_t prescaler_t2 = 7200; // TIM2预分频值

uint16_t cnt_t3 = 10000;     // TIM3计数器更新值
uint16_t prescaler_t3 = 720; // TIM3预分频值

uint8_t first_captured = 0;       // 第一个上升沿是否已捕获
uint16_t val_first_rising = 0;    // 第一个上升沿的捕获值
uint16_t update_times = 0;        // CNT溢出次数
uint8_t ready_to_cap_falling = 0; // 是否准备好捕获下降沿
uint32_t val_falling = 0;         // 下降沿的捕获值
uint32_t val_last_rising = 0;     // 第二个上升沿的捕获值

uint16_t frequency = 0; // 频率
float duty_ratio = 0;   // 占空比

void Timer_Init(void)
{
    // 打开时钟，包括GPIO和TIM
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // =============PWM配置================

    // 配置TIM2_CH2的GPIO功能复用
    GPIO_InitTypeDef GPIO_InitStructure1;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_AF_PP; // 配置为复用推挽模式
    GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_1;       // PA1
    GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure1);

    // 配置定时器时基单元
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure1;
    TIM_TimeBaseInitStructure1.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure1.TIM_Period = cnt_t2 - 1;
    TIM_TimeBaseInitStructure1.TIM_Prescaler = prescaler_t2 - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure1);

    // 配置输出比较单元
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出
    TIM_OCInitStructure.TIM_Pulse = cnt_t2 / 2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平为有效电平
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    // =============输入捕获配置================

    // 配置TIM3_CH2的GPIO
    GPIO_InitTypeDef GPIO_InitStructure2;
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_AF_PP; // 配置为复用推挽模式
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_7;       // PA6,PA7
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure2);

    // 配置TIM3的时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure2;
    TIM_TimeBaseInitStructure2.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure2.TIM_Prescaler = prescaler_t3 - 1;
    TIM_TimeBaseInitStructure2.TIM_Period = cnt_t3 - 1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure2);

    // 配置TIM3_CH2为输入捕获模式，初始捕获上升沿
    TIM_ICInitTypeDef TIM_ICInitStructure1;
    TIM_ICStructInit(&TIM_ICInitStructure1);
    TIM_ICInitStructure1.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure1.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM3, &TIM_ICInitStructure1);

    // =============中断配置================
    // 1. TIM3_CH1、TIM3_CH2每次捕获到值时，需要触发对应的中断来保存捕获的值
    // 2. 如果待测波形的频率非常低，则有可能造成CNT计数溢出，所以需要开启TIM3的更新中断，统计更新次数
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 开启定时器
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        update_times++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }

    if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        if (!first_captured)
        {
            val_first_rising = TIM_GetCapture2(TIM3);
            first_captured = 1;
            update_times = 0;

            // 捕获极性改为下降沿
            // 注意此处的TIM_OC2PolarityConfig名字是OC，但其实配置的是CCER寄存器
            // 该寄存器既可以配置输入通道的极性，也可以配置输出通道的极性(手册p292)，不会打架，因为同一时间通道x只能配置成输入或输出
            TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Falling);
            ready_to_cap_falling = 1;
        }
        else
        {
            if (ready_to_cap_falling)
            {
                uint16_t tmp = TIM_GetCapture2(TIM3);
                val_falling = tmp + update_times * cnt_t3;
                ready_to_cap_falling = 0;
                TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Rising);
            }
            else
            {
                uint16_t tmp = TIM_GetCapture2(TIM3);
                val_last_rising = tmp + update_times * cnt_t3; // 注意此处的val_last_rising的范围，uint16_t可能会溢出
                frequency = 72000000 / prescaler_t3 / (val_last_rising - val_first_rising);
                duty_ratio = (float)((val_falling - val_first_rising)) / (val_last_rising - val_first_rising);
                first_captured = 0;
            }
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
    }
}

uint16_t TIM_GetFrequency(void)
{
    return frequency;
}

float TIM_GetDutyRatio(void)
{
    return duty_ratio;
}

#endif

/* [ADC定时转换实验]
*/
#ifdef EXP_ADC_DMA
void Timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure); // 初始化TIM3，更新周期为1s

    // 配置TIM3的TRGO的触发源
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

    TIM_Cmd(TIM3, ENABLE);
}
#endif
