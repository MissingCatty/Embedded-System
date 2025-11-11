#include "lcd.h"
#include "stdlib.h"
#include "font.h"
#include "driver.h"

#define delay_ms timer_delay_ms
#define delay_us timer_delay_us

// LCD的画笔颜色和背景色
u16 POINT_COLOR = 0x0000; // 画笔颜色
u16 BACK_COLOR  = 0xFFFF; // 背景色

// 管理LCD重要参数
_lcd_dev lcddev;

// 写寄存器函数
void LCD_WR_REG(vu16 regval)
{
    regval       = regval;
    LCD->LCD_REG = regval;
}

// 写LCD数据
void LCD_WR_DATA(vu16 data)
{
    data         = data;
    LCD->LCD_RAM = data;
}

// 读LCD数据
u16 LCD_RD_DATA(void)
{
    vu16 ram;
    ram = LCD->LCD_RAM;
    return ram;
}

// 写寄存器
void LCD_WriteReg(vu16 LCD_Reg, vu16 LCD_RegValue)
{
    LCD->LCD_REG = LCD_Reg;
    LCD->LCD_RAM = LCD_RegValue;
}

// 读寄存器
u16 LCD_ReadReg(vu16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);
    delay_us(5);
    return LCD_RD_DATA();
}

// 开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

// LCD写GRAM
void LCD_WriteRAM(u16 RGB_Code)
{
    LCD->LCD_RAM = RGB_Code;
}

// 从ILI93xx读出的数据为GBR格式，转换为RGB格式
u16 LCD_BGR2RGB(u16 c)
{
    u16 r, g, b, rgb;
    b   = (c >> 0) & 0x1f;
    g   = (c >> 5) & 0x3f;
    r   = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

// 延时
void opt_delay(u8 i)
{
    while (i--);
}

// 读取某点的颜色值（简化版，只支持ILI9341）
u16 LCD_ReadPoint(u16 x, u16 y)
{
    vu16 r = 0, g = 0, b = 0;
    if (x >= lcddev.width || y >= lcddev.height) return 0;

    LCD_SetCursor(x, y);
    LCD_WR_REG(0X2E);  // ILI9341读GRAM指令
    LCD_RD_DATA();     // dummy Read
    opt_delay(2);
    r = LCD_RD_DATA(); // 实际读取
    opt_delay(2);
    b = LCD_RD_DATA();
    g = r & 0XFF;      // RG值，R在前G在后
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));
}

// LCD开启显示
void LCD_DisplayOn(void)
{
    LCD_WR_REG(0X29);
}

// LCD关闭显示
void LCD_DisplayOff(void)
{
    LCD_WR_REG(0X28);
}

// 设置光标位置
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(Xpos >> 8);
    LCD_WR_DATA(Xpos & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos >> 8);
    LCD_WR_DATA(Ypos & 0XFF);
}

// 设置LCD扫描方向（简化版）
void LCD_Scan_Dir(u8 dir)
{
    u16 regval = 0;
    u16 dirreg = 0;

    // 横竖屏处理
    if (lcddev.dir == 1)
    {
        switch (dir)
        {
        case 0: dir = 6; break;
        case 1: dir = 7; break;
        case 2: dir = 4; break;
        case 3: dir = 5; break;
        case 4: dir = 1; break;
        case 5: dir = 0; break;
        case 6: dir = 3; break;
        case 7: dir = 2; break;
        }
    }

    // ILI9341的扫描方向设置
    switch (dir)
    {
    case L2R_U2D:
        regval |= (0 << 7) | (0 << 6) | (0 << 5);
        break;
    case L2R_D2U:
        regval |= (1 << 7) | (0 << 6) | (0 << 5);
        break;
    case R2L_U2D:
        regval |= (0 << 7) | (1 << 6) | (0 << 5);
        break;
    case R2L_D2U:
        regval |= (1 << 7) | (1 << 6) | (0 << 5);
        break;
    case U2D_L2R:
        regval |= (0 << 7) | (0 << 6) | (1 << 5);
        break;
    case U2D_R2L:
        regval |= (0 << 7) | (1 << 6) | (1 << 5);
        break;
    case D2U_L2R:
        regval |= (1 << 7) | (0 << 6) | (1 << 5);
        break;
    case D2U_R2L:
        regval |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }

    dirreg = 0X36;
    regval |= 0X08;  // ILI9341需要BGR=1
    LCD_WriteReg(dirreg, regval);

    // 设置窗口
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.width - 1) >> 8);
    LCD_WR_DATA((lcddev.width - 1) & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.height - 1) >> 8);
    LCD_WR_DATA((lcddev.height - 1) & 0XFF);
}

// 设置LCD显示方向（0:竖屏 1:横屏）
void LCD_Display_Dir(u8 dir)
{
    lcddev.dir = dir;
    if (dir == 0) // 竖屏
    {
        lcddev.width  = 240;
        lcddev.height = 320;
    }
    else  // 横屏
    {
        lcddev.width  = 320;
        lcddev.height = 240;
    }
    lcddev.wramcmd = 0X2C;
    lcddev.setxcmd = 0X2A;
    lcddev.setycmd = 0X2B;
    LCD_Scan_Dir(DFT_SCAN_DIR);
}

// 设置窗口
void LCD_Set_Window(u16 sx, u16 sy, u16 width, u16 height)
{
    u16 twidth, theight;
    twidth  = sx + width - 1;
    theight = sy + height - 1;

    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(sx >> 8);
    LCD_WR_DATA(sx & 0XFF);
    LCD_WR_DATA(twidth >> 8);
    LCD_WR_DATA(twidth & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(sy >> 8);
    LCD_WR_DATA(sy & 0XFF);
    LCD_WR_DATA(theight >> 8);
    LCD_WR_DATA(theight & 0XFF);
}

// 画点
void LCD_DrawPoint(u16 x, u16 y)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    LCD->LCD_RAM = POINT_COLOR;
}

// 快速画点
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(x >> 8);
    LCD_WR_DATA(x & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(y >> 8);
    LCD_WR_DATA(y & 0XFF);
    LCD->LCD_REG = lcddev.wramcmd;
    LCD->LCD_RAM = color;
}

// 清屏函数
void LCD_Clear(u16 color)
{
    u32 index      = 0;
    u32 totalpoint = lcddev.width;
    totalpoint *= lcddev.height;
    LCD_SetCursor(0x00, 0x0000);
    LCD_WriteRAM_Prepare();
    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
    }
}

// 填充区域
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color)
{
    u16 i, j;
    u16 xlen = 0;
    xlen     = ex - sx + 1;
    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i);
        LCD_WriteRAM_Prepare();
        for (j = 0; j < xlen; j++)
            LCD->LCD_RAM = color;
    }
}

// 填充指定颜色块
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color)
{
    u16 height, width;
    u16 i, j;
    width  = ex - sx + 1;
    height = ey - sy + 1;
    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i);
        LCD_WriteRAM_Prepare();
        for (j = 0; j < width; j++)
            LCD->LCD_RAM = color[i * width + j];
    }
}

// 画线
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow    = x1;
    uCol    = y1;
    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    for (t = 0; t <= distance + 1; t++)
    {
        LCD_DrawPoint(uRow, uCol);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 画矩形
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

// 画圆
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r)
{
    int a, b;
    int di;
    a  = 0;
    b  = r;
    di = 3 - (r << 1);
    while (a <= b)
    {
        LCD_DrawPoint(x0 + a, y0 - b);
        LCD_DrawPoint(x0 + b, y0 - a);
        LCD_DrawPoint(x0 + b, y0 + a);
        LCD_DrawPoint(x0 + a, y0 + b);
        LCD_DrawPoint(x0 - a, y0 + b);
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - b, y0 - a);
        LCD_DrawPoint(x0 - a, y0 - b);
        a++;
        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

// 显示一个字符
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode)
{
    u8  temp, t1, t;
    u16 y0 = y;
    u8  csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    num       = num - ' ';
    for (t = 0; t < csize; t++)
    {
        if (size == 12)
            temp = asc2_1206[num][t];
        else if (size == 16)
            temp = asc2_1608[num][t];
        else if (size == 24)
            temp = asc2_2412[num][t];
        else
            return;
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)
                LCD_Fast_DrawPoint(x, y, BACK_COLOR);
            temp <<= 1;
            y++;
            if (y >= lcddev.height)
                return;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                if (x >= lcddev.width)
                    return;
                break;
            }
        }
    }
}

u32 LCD_Pow(u8 m, u8 n)
{
    u32 result = 1;
    while (n--) result *= m;
    return result;
}


// 显示数字
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size)
{
    u8  t, temp;
    u8  enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
                continue;
            }
            else
                enshow = 1;
        }
        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
    }
}

// 显示数字(高级)
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{
    u8 t, temp;
    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (temp == 0 && t < (len - 1))
        {
            if (mode & 0X80)
                LCD_ShowChar(x + (size / 2) * t, y, '0', size, mode & 0X01);
            else
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size, mode & 0X01);
        }
        else
            LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode & 0X01);
    }
}

// 显示字符串
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
    u8  x0 = x;
    width += x;
    height += y;
    while ((*p <= '~') && (*p >= ' '))
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }
        if (y >= height)
            break;
        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}

// ILI9341初始化
void LCD_Init(void)
{
    GPIO_InitTypeDef              GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef       FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef readWriteTiming;
    FSMC_NORSRAMTimingInitTypeDef writeTiming;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    // PB1背光控制
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // PD口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8
                                  | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // PE口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12
                                  | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // GPIO复用配置
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

    // FSMC读时序配置
    readWriteTiming.FSMC_AddressSetupTime      = 0x0F;
    readWriteTiming.FSMC_AddressHoldTime       = 0x00;
    readWriteTiming.FSMC_DataSetupTime          = 0x46;
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision            = 0x00;
    readWriteTiming.FSMC_DataLatency            = 0x00;
    readWriteTiming.FSMC_AccessMode             = FSMC_AccessMode_A;

    // FSMC写时序配置（ILI9341用最快时序）
    writeTiming.FSMC_AddressSetupTime      = 0x00;
    writeTiming.FSMC_AddressHoldTime       = 0x00;
    writeTiming.FSMC_DataSetupTime          = 0x03;
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision            = 0x00;
    writeTiming.FSMC_DataLatency            = 0x00;
    writeTiming.FSMC_AccessMode             = FSMC_AccessMode_A;

    // FSMC配置
    FSMC_NORSRAMInitStructure.FSMC_Bank                  = FSMC_Bank1_NORSRAM4;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &writeTiming;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);

    // 背光开启
    GPIO_SetBits(GPIOB, GPIO_Pin_1);

    // 延时50ms
    delay_ms(50);

    // 固定设置为ILI9341
    lcddev.id = 0x9341;

    // ILI9341寄存器初始化
    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0X30);
    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0X12);
    LCD_WR_DATA(0X81);
    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x7A);
    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xC0);    // Power control
    LCD_WR_DATA(0x1B);   // VRH[5:0]
    LCD_WR_REG(0xC1);    // Power control
    LCD_WR_DATA(0x01);   // SAP[2:0];BT[3:0]
    LCD_WR_REG(0xC5);    // VCM control
    LCD_WR_DATA(0x30);   // 3F
    LCD_WR_DATA(0x30);   // 3C
    LCD_WR_REG(0xC7);    // VCM control2
    LCD_WR_DATA(0XB7);
    LCD_WR_REG(0x36);    // Memory Access Control
    LCD_WR_DATA(0x48);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1A);
    LCD_WR_REG(0xB6);    // Display Function Control
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0xA2);
    LCD_WR_REG(0xF2);    // 3Gamma Function Disable
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26);    // Gamma curve selected
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xE0);    // Set Gamma
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x2A);
    LCD_WR_DATA(0x28);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0XA9);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0XE1);    // Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x56);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x3f);
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xef);
    LCD_WR_REG(0x11);    // Exit Sleep
    delay_ms(120);
    LCD_WR_REG(0x29);    // Display on

    LCD_Display_Dir(0);  // 默认竖屏
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    LCD_Clear(WHITE);
}
