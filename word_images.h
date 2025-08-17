#ifndef WORD_IMAGES_H
#define WORD_IMAGES_H

#include <Arduino.h>

// 图片尺寸定义（适配212×104分辨率的2.13寸墨水屏）
#define IMAGE_WIDTH 212
#define IMAGE_HEIGHT 104
#define TOTAL_IMAGES 11  // 总共11张图片（word_0.bmp到word_10.bmp）

// 声明图片数组
extern const unsigned char* word_images[TOTAL_IMAGES];

// 图片数据将在word_images.cpp中定义

#endif // WORD_IMAGES_H