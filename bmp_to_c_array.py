#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
BMP转C数组工具

这个脚本用于将BMP图片转换为C数组格式，以便在ESP8266墨水屏项目中使用。
它会读取images目录下的word_0.bmp到word_10.bmp图片文件，然后将它们转换为
适合墨水屏显示的位图数组，并生成可以直接替换到word_images.cpp文件中的C代码。

适用于212×104分辨率的2.13寸墨水屏。
"""

import os
import struct
import sys
from pathlib import Path

# 配置参数
IMAGE_WIDTH = 212  # 图片宽度
IMAGE_HEIGHT = 104  # 图片高度
TOTAL_IMAGES = 11  # 总图片数量
IMAGE_PREFIX = "word_"  # 图片文件名前缀
IMAGE_DIR = "images"  # 图片目录
OUTPUT_FILE = "word_images_generated.cpp"  # 输出文件名

# 计算每行字节数（向上取整到8的倍数）
BYTES_PER_ROW = (IMAGE_WIDTH + 7) // 8
# 计算图片总字节数
IMAGE_BYTES = BYTES_PER_ROW * IMAGE_HEIGHT

def read_bmp_file(file_path):
    """
    读取BMP文件并转换为适合墨水屏显示的位图数组
    
    墨水屏使用1位深度的位图，每个字节表示8个像素，1表示黑色，0表示白色
    BMP文件的像素排列是从下到上、从左到右，需要转换为从上到下、从左到右
    """
    try:
        with open(file_path, 'rb') as f:
            # 读取BMP文件头
            header = f.read(54)
            
            # 检查是否是有效的BMP文件
            if header[0:2] != b'BM':
                raise ValueError(f"{file_path} 不是有效的BMP文件")
            
            # 获取位图数据偏移量
            offset = struct.unpack('<I', header[10:14])[0]
            
            # 获取位图宽度和高度
            width = struct.unpack('<i', header[18:22])[0]
            height = struct.unpack('<i', header[22:26])[0]
            
            # 获取位深度
            bit_depth = struct.unpack('<H', header[28:30])[0]
            
            # 检查图片尺寸
            if width != IMAGE_WIDTH or abs(height) != IMAGE_HEIGHT:
                print(f"警告: {file_path} 尺寸不是 {IMAGE_WIDTH}x{IMAGE_HEIGHT} (实际: {width}x{abs(height)})")
            
            # 检查位深度
            if bit_depth != 1:
                raise ValueError(f"{file_path} 不是1位深度的BMP文件 (实际: {bit_depth}位)")
            
            # 移动到位图数据区
            f.seek(offset)
            
            # 计算每行的字节数（包括填充到4字节的边界）
            row_size = ((width + 31) // 32) * 4
            
            # 读取位图数据
            raw_data = bytearray()
            
            # BMP文件的像素排列是从下到上，需要反转
            if height > 0:  # 正高度表示从下到上
                for i in range(height-1, -1, -1):
                    row_data = f.read(row_size)
                    raw_data.extend(row_data[:BYTES_PER_ROW])  # 去除填充字节
            else:  # 负高度表示从上到下
                for i in range(abs(height)):
                    row_data = f.read(row_size)
                    raw_data.extend(row_data[:BYTES_PER_ROW])  # 去除填充字节
            
            # 墨水屏的像素排列是1表示黑色，0表示白色
            # 而BMP文件中通常是0表示黑色，1表示白色，需要反转位
            for i in range(len(raw_data)):
                raw_data[i] = ~raw_data[i] & 0xFF
            
            return raw_data
            
    except Exception as e:
        print(f"处理 {file_path} 时出错: {e}")
        return None

def generate_c_array(image_data, image_name):
    """
    将图片数据转换为C数组格式的字符串
    """
    if not image_data:
        return f"// 错误: 无法处理 {image_name}\nconst unsigned char {image_name}[1] = {{0x00}};\n"
    
    # 生成数组声明
    c_array = f"const unsigned char {image_name}[{len(image_data)}] = {{\n  "
    
    # 每行16个字节
    bytes_per_line = 16
    for i, byte in enumerate(image_data):
        c_array += f"0x{byte:02X}"
        
        # 添加分隔符
        if i < len(image_data) - 1:
            c_array += ", "
        
        # 换行
        if (i + 1) % bytes_per_line == 0 and i < len(image_data) - 1:
            c_array += "\n  "
    
    c_array += "\n};\n"
    return c_array

def generate_cpp_file():
    """
    生成word_images.cpp文件
    """
    script_dir = Path(__file__).parent
    image_dir = script_dir / IMAGE_DIR
    output_path = script_dir / OUTPUT_FILE
    
    # 检查图片目录是否存在
    if not image_dir.exists() or not image_dir.is_dir():
        print(f"错误: 图片目录 {image_dir} 不存在")
        return False
    
    # 处理所有图片
    image_arrays = []
    image_names = []
    
    for i in range(TOTAL_IMAGES):
        image_file = f"{IMAGE_PREFIX}{i}.bmp"
        image_path = image_dir / image_file
        
        if not image_path.exists():
            print(f"错误: 图片文件 {image_path} 不存在")
            return False
        
        print(f"处理图片: {image_file}")
        image_data = read_bmp_file(str(image_path))
        
        if image_data is None:
            print(f"错误: 无法处理图片 {image_file}")
            return False
        
        image_name = f"WORD_{i}_IMAGE"
        image_names.append(image_name)
        image_arrays.append(generate_c_array(image_data, image_name))
    
    # 生成C文件内容
    cpp_content = """#include "word_images.h"

// 图片数据声明 - 由bmp_to_c_array.py自动生成
// 适用于212×104分辨率的2.13寸墨水屏
// 每个字节表示8个像素，1表示黑色，0表示白色

"""
    
    # 添加所有图片数组
    for array in image_arrays:
        cpp_content += array + "\n"
    
    # 添加图片数组指针数组
    cpp_content += "// 图片数组初始化\n"
    cpp_content += "const unsigned char* word_images[TOTAL_IMAGES] = {\n"
    
    for i, name in enumerate(image_names):
        cpp_content += f"  {name}"  # 图片数组名称
        
        # 添加注释和分隔符
        cpp_content += f",  // word_{i}.bmp\n"
    
    # 移除最后一个逗号并关闭数组
    cpp_content = cpp_content.rstrip("\n") + "\n};\n"
    
    # 写入文件
    with open(output_path, 'w') as f:
        f.write(cpp_content)
    
    print(f"\n成功生成C数组文件: {output_path}")
    print(f"请将此文件内容复制到word_images.cpp中，或直接重命名替换原文件。")
    return True

def main():
    print("BMP转C数组工具 - 适用于ESP8266墨水屏项目")
    print(f"将转换 {IMAGE_DIR} 目录下的 {TOTAL_IMAGES} 张BMP图片为C数组\n")
    
    if generate_cpp_file():
        print("\n转换完成!")
    else:
        print("\n转换失败!")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())