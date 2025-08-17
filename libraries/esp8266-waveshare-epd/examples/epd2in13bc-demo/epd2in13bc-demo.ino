/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "imagedata.h"
#include <stdlib.h>
#include <TimeLib.h> // 时间库
#include <ESP8266WiFi.h> // WiFi库
#include <WiFiUdp.h> // UDP库，用于NTP

// WiFi配置
const char* ssid = "TP-Song"; // 请在此处输入您的WiFi名称
const char* password = "song18567132008"; // 请在此处输入您的WiFi密码

// NTP服务器配置（阿里云NTP服务器）
const char* ntpServerName = "ntp.aliyun.com";
const int timeZone = 8; // 中国时区 (UTC+8)

// NTP相关变量
WiFiUDP Udp;
unsigned int localPort = 8888; // 本地端口用于UDP通信
byte packetBuffer[48]; // NTP时间戳是前48字节

// 不再使用分区显示，改为单行显示

// 全局变量
UBYTE *BlackImage, *RYImage;
UWORD Imagesize;
unsigned long lastMinuteUpdate = 0;
unsigned long lastFullRefresh = 0;
bool timeInitialized = false;
int lastMinute = -1; // 记录上次更新的分钟值

// 刷新间隔设置
#define REFRESH_INTERVAL 60000 // 刷新间隔(毫秒)，设为1分钟
#define BUSY_TIMEOUT 5000 // busy信号超时时间(毫秒)

/* 初始化函数 */
void initDisplay() {
  Serial.println("e-Paper Init and Clear...");
  EPD_2IN13BC_Init();
  EPD_2IN13BC_Clear();
  DEV_Delay_ms(500);

  // 创建图像缓存
  Imagesize = ((EPD_2IN13BC_WIDTH % 8 == 0) ? (EPD_2IN13BC_WIDTH / 8) : (EPD_2IN13BC_WIDTH / 8 + 1)) * EPD_2IN13BC_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    Serial.println("Failed to apply for black memory...");
    while (1);
  }
  if ((RYImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    Serial.println("Failed to apply for red memory...");
    while (1);
  }
  
  // 初始化图像
  Paint_NewImage(BlackImage, EPD_2IN13BC_WIDTH, EPD_2IN13BC_HEIGHT, 270, BLACK);
  Paint_NewImage(RYImage, EPD_2IN13BC_WIDTH, EPD_2IN13BC_HEIGHT, 270, WHITE);
  
  // 清空图像
  Paint_SelectImage(BlackImage);
  Paint_Clear(BLACK);
  Paint_SelectImage(RYImage);
  Paint_Clear(WHITE);
}

/* 绘制日期时间 - 单行显示年月日时分 */
void drawDateTime() {
  // 获取当前时间
  int currentYear = year();
  int currentMonth = month();
  int currentDay = day();
  int currentHour = hour();
  int currentMinute = minute();
  
  // 创建日期时间字符串 (格式: YYYY-MM-DD HH:MM)
  char dateTimeStr[20];
  sprintf(dateTimeStr, "%04d-%02d-%02d %02d:%02d", currentYear, currentMonth, currentDay, currentHour, currentMinute);
  
  // 清空黑色图层
  Paint_SelectImage(BlackImage);
  Paint_Clear(BLACK);
  
  // 清空红色图层
  Paint_SelectImage(RYImage);
  Paint_Clear(WHITE);
  
  // 在黑色图层上绘制日期时间字符串 (居中显示)
  Paint_SelectImage(BlackImage);
  
  // 计算水平居中位置 (使用Font8字体，每个字符宽度约为5像素)
  int x_pos = (EPD_2IN13BC_WIDTH - strlen(dateTimeStr) * 5) / 2;
  if (x_pos < 0) x_pos = 0;
  
  // 计算垂直居中位置
  int y_pos = (EPD_2IN13BC_HEIGHT - 8) / 2; // Font8高度为8像素
  if (y_pos < 0) y_pos = 0;
  
  // 绘制日期时间字符串
  Paint_DrawString_EN(x_pos, y_pos, dateTimeStr, &Font8, BLACK, WHITE);
  
  // 在底部添加一个小标识 (使用红色图层)
  Paint_SelectImage(RYImage);
  Paint_DrawString_EN(10, EPD_2IN13BC_HEIGHT - 15, "ESP8266 E-Ink Clock", &Font12, BLACK, WHITE);
  
  // 全屏刷新
  EPD_2IN13BC_Display(RYImage, BlackImage);
  
  // 更新上次刷新的分钟值
  lastMinute = currentMinute;
}

// 原drawTime函数已被移除，功能已合并到drawDateTime函数中

/* 连接WiFi */
void connectWiFi() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("正在连接到WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // 等待连接，最多尝试20秒
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi连接成功!");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi连接失败，将使用默认时间");
  }
}

/* 从NTP服务器获取时间 */
time_t getNtpTime() {
  // 如果WiFi未连接，返回0
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  
  Serial.println("正在从NTP服务器获取时间...");
  
  // 初始化UDP
  Udp.begin(localPort);
  
  // 发送NTP请求
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  
  // 发送请求到NTP服务器
  Udp.beginPacket(ntpServerName, 123); // NTP请求端口为123
  Udp.write(packetBuffer, 48);
  Udp.endPacket();
  
  // 等待响应，最多等待1秒
  unsigned long startWait = millis();
  while (millis() - startWait < 1000) {
    if (Udp.parsePacket()) {
      // 读取响应
      Udp.read(packetBuffer, 48);
      
      // NTP时间在响应的前48字节中
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // 将两个字组合成一个32位数字
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      
      // NTP时间是从1900年开始的，而Unix时间是从1970年开始的
      // 两者相差70年，即2208988800秒
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      
      // 调整时区
      epoch += timeZone * 3600;
      
      Serial.println("NTP时间同步成功!");
      return epoch;
    }
    delay(10);
  }
  
  Serial.println("NTP时间同步失败，将使用默认时间");
  return 0; // 如果无法获取时间，返回0
}

/* Entry point ----------------------------------------------------------------*/
void setup() {
  Serial.begin(115200);
  Serial.println("EPD_2IN13BC Clock Demo");
  DEV_Module_Init();
  
  // 连接WiFi
  connectWiFi();
  
  // 从NTP服务器获取时间
  time_t ntpTime = getNtpTime();
  if (ntpTime > 0) {
    // 设置系统时间
    setTime(ntpTime);
    timeInitialized = true;
    Serial.println("时间已同步: " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
  } else {
    // 如果无法获取NTP时间，设置默认时间
    setTime(12, 0, 0, 1, 1, 2023);
    Serial.println("使用默认时间: 12:00:00 2023-01-01");
  }
  
  // 初始化显示
  initDisplay();
  
  // 绘制日期时间（单行显示）
  drawDateTime();
}

/* The main loop -------------------------------------------------------------*/
void loop() {
  unsigned long currentMillis = millis();
  int currentMinute = minute();
  
  // 检查是否需要更新时间显示（每分钟更新一次）
  if (currentMinute != lastMinute) {
    Serial.println("检测到分钟变化，更新显示...");
    
    // 全屏刷新前重新初始化显示
    Serial.println("执行全屏刷新并重新初始化显示...");
    EPD_2IN13BC_Init();
    DEV_Delay_ms(100);
    
    // 更新日期时间显示（单行显示年月日时分）
    Serial.println("更新日期时间显示: " + String(year()) + "-" + String(month()) + "-" + String(day()) + " " + String(hour()) + ":" + String(minute()));
    drawDateTime();
    
    // 更新最后刷新时间
    lastMinuteUpdate = currentMillis;
  }
  
  // 每隔5分钟检查WiFi连接状态
  static unsigned long lastWiFiCheck = 0;
  if (currentMillis - lastWiFiCheck >= 300000) { // 5分钟检查一次
    lastWiFiCheck = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi连接已断开，尝试重新连接...");
      WiFi.reconnect();
    }
    
    // 每天凌晨重新同步一次NTP时间
    static int lastSyncDay = -1;
    if (day() != lastSyncDay && hour() == 0 && minute() < 5) {
      Serial.println("执行每日时间同步...");
      time_t ntpTime = getNtpTime();
      if (ntpTime > 0) {
        setTime(ntpTime);
        Serial.println("时间已重新同步");
      }
      lastSyncDay = day();
    }
  }
  
  // 短暂延时，避免CPU占用过高
  delay(100);
}
