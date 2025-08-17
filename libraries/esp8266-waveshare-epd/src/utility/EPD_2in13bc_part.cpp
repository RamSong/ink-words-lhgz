#include "EPD_2in13bc.h"
#include "Debug.h"

// 局部刷新相关的LUT表
const unsigned char EPD_2IN13BC_lut_partial_update[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //LUT0: BB:     VS 0 ~7
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //LUT1: BW:     VS 0 ~7
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //LUT2: WB:     VS 0 ~7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //LUT3: WW:     VS 0 ~7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //LUT4: VCOM:   VS 0 ~7

    0x0A, 0x00, 0x00, 0x00, 0x00,                       // TP0 A~D RP0
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP1 A~D RP1
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP2 A~D RP2
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP3 A~D RP3
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP4 A~D RP4
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP5 A~D RP5
    0x00, 0x00, 0x00, 0x00, 0x00,                       // TP6 A~D RP6

    0x15, 0x41, 0xA8, 0x32, 0x30, 0x0A,
};

// 局部刷新初始化
void EPD_2IN13BC_Init_Part(void)
{
    EPD_2IN13BC_Reset();

    EPD_2IN13BC_SendCommand(0x06); // BOOSTER_SOFT_START
    EPD_2IN13BC_SendData(0x17);
    EPD_2IN13BC_SendData(0x17);
    EPD_2IN13BC_SendData(0x17);
    
    EPD_2IN13BC_SendCommand(0x04); // POWER_ON
    EPD_2IN13BC_ReadBusy();
    
    EPD_2IN13BC_SendCommand(0x00); // PANEL_SETTING
    EPD_2IN13BC_SendData(0x8F);
    
    EPD_2IN13BC_SendCommand(0x50); // VCOM_AND_DATA_INTERVAL_SETTING
    EPD_2IN13BC_SendData(0xF0);
    
    EPD_2IN13BC_SendCommand(0x61); // RESOLUTION_SETTING
    EPD_2IN13BC_SendData(EPD_2IN13BC_WIDTH); // width: 104
    EPD_2IN13BC_SendData(EPD_2IN13BC_HEIGHT >> 8); // height: 212
    EPD_2IN13BC_SendData(EPD_2IN13BC_HEIGHT & 0xFF);
    
    // 设置局部刷新模式
    EPD_2IN13BC_SendCommand(0x32); // 写LUT寄存器
    for (int i = 0; i < 70; i++) {
        EPD_2IN13BC_SendData(EPD_2IN13BC_lut_partial_update[i]);
    }
    
    EPD_2IN13BC_SendCommand(0x37); // 设置显示选项
    EPD_2IN13BC_SendData(0x00);
    EPD_2IN13BC_SendData(0x00);
    EPD_2IN13BC_SendData(0x00);
    EPD_2IN13BC_SendData(0x00);
    EPD_2IN13BC_SendData(0x40);
    EPD_2IN13BC_SendData(0x00);
    EPD_2IN13BC_SendData(0x00);
    
    EPD_2IN13BC_SendCommand(0x3C); // BorderWavefrom
    EPD_2IN13BC_SendData(0x01);
}

// 增加超时处理的忙状态检测函数
void EPD_2IN13BC_ReadBusyWithTimeout(unsigned long timeout_ms = 5000) {
    unsigned long start_time = millis();
    while(DEV_Digital_Read(EPD_BUSY_PIN) == 0) {      // 0: busy, 1: idle
        DEV_Delay_ms(10);
        if (millis() - start_time > timeout_ms) {
            printf("EPD busy timeout after %lu ms\r\n", timeout_ms);
            break; // 超时退出
        }
    }
    printf("EPD busy release\r\n");
}

// 局部刷新显示函数
void EPD_2IN13BC_DisplayPart(const UBYTE *blackimage, const UBYTE *ryimage, UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end)
{
    // 严格验证坐标范围，确保在有效范围内
    if (x_start >= EPD_2IN13BC_WIDTH || y_start >= EPD_2IN13BC_HEIGHT || 
        x_end > EPD_2IN13BC_WIDTH || y_end > EPD_2IN13BC_HEIGHT ||
        x_start >= x_end || y_start >= y_end) {
        printf("刷新区域超出范围: x_start=%d, y_start=%d, x_end=%d, y_end=%d\r\n", 
               x_start, y_start, x_end, y_end);
        return;
    }
    
    // 确保x_end和y_end不超过屏幕边界
    if (x_end > EPD_2IN13BC_WIDTH) x_end = EPD_2IN13BC_WIDTH;
    if (y_end > EPD_2IN13BC_HEIGHT) y_end = EPD_2IN13BC_HEIGHT;
    
    UWORD Width, Height;
    Width = (EPD_2IN13BC_WIDTH % 8 == 0) ? (EPD_2IN13BC_WIDTH / 8) : (EPD_2IN13BC_WIDTH / 8 + 1);
    Height = EPD_2IN13BC_HEIGHT;
    
    // 等待上一次刷新完成，使用带超时的函数
    EPD_2IN13BC_ReadBusyWithTimeout();
    
    // 设置局部刷新区域
    EPD_2IN13BC_SendCommand(0x91); // 进入局部刷新模式
    
    EPD_2IN13BC_SendCommand(0x90); // 设置局部刷新区域
    EPD_2IN13BC_SendData(x_start);
    EPD_2IN13BC_SendData(x_end - 1);
    EPD_2IN13BC_SendData(y_start / 256);
    EPD_2IN13BC_SendData(y_start % 256);
    EPD_2IN13BC_SendData(y_end / 256);
    EPD_2IN13BC_SendData(y_end % 256);
    EPD_2IN13BC_SendData(0x01); // 不关心
    
    // 发送黑色图像数据 - 只发送局部区域数据
    EPD_2IN13BC_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        // 只处理在刷新区域内的行
        if (j >= y_start && j < y_end) {
            for (UWORD i = 0; i < Width; i++) {
                // 计算当前像素的字节位置
                UWORD byte_index = i + j * Width;
                EPD_2IN13BC_SendData(blackimage[byte_index]);
            }
        } else {
            // 对于不在刷新区域内的行，发送空白数据
            for (UWORD i = 0; i < Width; i++) {
                EPD_2IN13BC_SendData(0xFF); // 白色
            }
        }
    }
    EPD_2IN13BC_SendCommand(0x92);
    DEV_Delay_ms(2); // 短暂延时确保命令被处理
    
    // 发送红色图像数据 - 只发送局部区域数据
    EPD_2IN13BC_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        // 只处理在刷新区域内的行
        if (j >= y_start && j < y_end) {
            for (UWORD i = 0; i < Width; i++) {
                // 计算当前像素的字节位置
                UWORD byte_index = i + j * Width;
                EPD_2IN13BC_SendData(ryimage[byte_index]);
            }
        } else {
            // 对于不在刷新区域内的行，发送空白数据
            for (UWORD i = 0; i < Width; i++) {
                EPD_2IN13BC_SendData(0xFF); // 白色
            }
        }
    }
    EPD_2IN13BC_SendCommand(0x92);
    DEV_Delay_ms(2); // 短暂延时确保命令被处理
    DEV_Delay_ms(2); // 短暂延时确保命令被处理
    
    // 刷新显示
    EPD_2IN13BC_SendCommand(0x12); // DISPLAY REFRESH
    DEV_Delay_ms(10);
    EPD_2IN13BC_ReadBusyWithTimeout(); // 等待刷新完成，使用带超时的函数
    
    EPD_2IN13BC_SendCommand(0x92); // 退出局部刷新模式
    DEV_Delay_ms(200); // 给屏幕一些恢复时间
}