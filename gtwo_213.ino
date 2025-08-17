#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "word_images.h"

// 屏幕初始化（适配212x104分辨率）
GxEPD2_BW<GxEPD2_213_flex, GxEPD2_213_flex::HEIGHT> display(GxEPD2_213_flex(/*CS=5*/ 15, /*DC=*/ 4, /*RST=*/ 2, /*BUSY=*/ 5));

// 按钮配置
const int BUTTON_PIN = 16; // D0对应GPIO16
int currentImageIndex = 0;
unsigned long lastDebounceTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("墨水屏单词机启动中...");
  
  // 初始化墨水屏
  display.init();
  display.setRotation(1); // 设置屏幕方向
  
  // 配置按钮引脚，使用上拉电阻
  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("初始化完成，显示第一张图片");
  displayCurrentImage();
}

void loop() {
  // 检测按钮按下（高电平触发）
  int reading = digitalRead(BUTTON_PIN);
  
  // 按钮防抖处理，确保稳定的高电平信号
  if (reading == HIGH && millis() - lastDebounceTime > 200) {
    lastDebounceTime = millis();
    
    // 切换到下一张图片
    currentImageIndex = (currentImageIndex + 1) % TOTAL_IMAGES;
    Serial.print("按钮触发，切换到图片：");
    Serial.println(currentImageIndex);
    
    // 显示新图片
    displayCurrentImage();
    
    // 等待按钮释放
    while(digitalRead(BUTTON_PIN) == HIGH) {
      delay(10);
    }
  }
  
  delay(10); // 短暂延时以减少CPU负载
}

void displayCurrentImage() {
  // 准备显示图片
  display.setFullWindow();
  display.firstPage();
  
  // 绘制当前索引对应的图片
  do {
    display.fillScreen(GxEPD_BLACK);  // 设置背景为黑色
    display.drawInvertedBitmap(0, 0, word_images[currentImageIndex], IMAGE_WIDTH, IMAGE_HEIGHT, GxEPD_WHITE);  // 图片内容显示为白色
  } while (display.nextPage());
  
  // 完成显示后进入深度睡眠模式以节省电量
  display.hibernate();
  
  Serial.print("显示图片完成：");
  Serial.println(currentImageIndex);
}
