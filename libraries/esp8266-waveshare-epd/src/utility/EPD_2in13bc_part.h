#ifndef __EPD_2IN13BC_PART_H_
#define __EPD_2IN13BC_PART_H_

#include "EPD_2in13bc.h"

// 局部刷新初始化函数
void EPD_2IN13BC_Init_Part(void);

// 带超时的忙状态检测函数
void EPD_2IN13BC_ReadBusyWithTimeout(unsigned long timeout_ms);

// 局部刷新显示函数
void EPD_2IN13BC_DisplayPart(const UBYTE *blackimage, const UBYTE *ryimage, UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end);

#endif