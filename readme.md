# 墨水屏单词机项目

## 包含
1. 硬件上的电路PCB设计（实际无用 因展示需求做得很大）
2. 软件上 arduino 用到的 所有文件

## 注意

1. 由于时间原因，声音播放功能未调试完好，具体废案在压缩包内。
2. 由于过度信任社，pcb中核心板子尺寸错误，目前未修改。
3. PCB文件应使用 LcEDA ，具体文件在releases。


## 项目整体功能
这是一个可以在墨水屏上显示英语单词卡片的设备，通过按钮切换不同的单词卡片，用于英语学习和记忆。

## 各文件功能详解

1. **[readme.md](file:///Users/ramsong/Documents/Arduino/gtwo_213/readme.md)**
   - 项目说明文档，介绍如何将BMP图片转换为C数组格式，以便在ESP8266墨水屏项目中使用。

2. **[bmp_to_c_array.py](file:///Users/ramsong/Documents/Arduino/gtwo_213/bmp_to_c_array.py)**
   - BMP转C数组工具，将[images](file:///Users/ramsong/Documents/Arduino/gtwo_213/images/)目录下的BMP图片转换为适合墨水屏显示的C数组格式。
   - 专门处理212×104分辨率的1位深度BMP图片，适配2.13寸墨水屏。
   - 生成可以直接替换到[word_images.cpp](file:///Users/ramsong/Documents/Arduino/gtwo_213/word_images.cpp)文件中的C代码。

3. **[generate_word_cards.py](file:///Users/ramsong/Documents/Arduino/gtwo_213/generate_word_cards.py)**
   - 单词卡片生成工具，使用PIL库自动生成包含英语单词、音标、词性和中文释义的图片。
   - 生成的图片保存在[images](file:///Users/ramsong/Documents/Arduino/gtwo_213/images/)目录下，文件名为word_0.bmp到word_10.bmp。
   - 每张图片尺寸为212×104像素，适配2.13寸墨水屏。

4. **[word_images.h](file:///Users/ramsong/Documents/Arduino/gtwo_213/word_images.h)**
   - 头文件，定义了图片尺寸常量和外部变量声明。
   - 声明了[word_images](file:///Users/ramsong/Documents/Arduino/gtwo_213/word_images.cpp#L1987-L1987)数组指针，用于引用所有单词卡片图片数据。

5. **[word_images.cpp](file:///Users/ramsong/Documents/Arduino/gtwo_213/word_images.cpp)**
   - 包含所有单词卡片图片的C数组数据。
   - 每张图片都被转换为C数组格式，可以直接在墨水屏上显示。
   - 这些数据是由[bmp_to_c_array.py](file:///Users/ramsong/Documents/Arduino/gtwo_213/bmp_to_c_array.py)工具自动生成的。

6. **[gtwo_213.ino](file:///Users/ramsong/Documents/Arduino/gtwo_213/gtwo_213.ino)**
   - 主程序文件，实现墨水屏单词机的核心功能。
   - 初始化墨水屏和按钮。
   - 通过按钮切换显示不同的单词卡片。
   - 实现了防抖处理和深度睡眠节能功能。

## 工作流程
1. 使用[generate_word_cards.py](file:///Users/ramsong/Documents/Arduino/gtwo_213/generate_word_cards.py)生成单词卡片图片（BMP格式）
2. 使用[bmp_to_c_array.py](file:///Users/ramsong/Documents/Arduino/gtwo_213/bmp_to_c_array.py)将BMP图片转换为C数组并更新[word_images.cpp](file:///Users/ramsong/Documents/Arduino/gtwo_213/word_images.cpp)
3. 编译并上传[gtwo_213.ino](file:///Users/ramsong/Documents/Arduino/gtwo_213/gtwo_213.ino)到ESP8266开发板
4. 设备启动后在墨水屏上显示单词卡片，按下按钮切换到下一张

