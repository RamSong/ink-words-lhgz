from PIL import Image, ImageDraw, ImageFont
import os

# 定义单词列表
words = [
    {"english": "Algorithm", "phonetic": "/ˈælɡərɪðəm/", "chinese": "算法", "partOfSpeech": "n."},
    {"english": "Framework", "phonetic": "/ˈfreɪmˌwɜːrk/", "chinese": "框架", "partOfSpeech": "n."},
    {"english": "Interface", "phonetic": "/ˈɪntərˌfeɪs/", "chinese": "接口", "partOfSpeech": "n."},
    {"english": "Optimization", "phonetic": "/ˌɒptɪmaɪˈzeɪʃən/", "chinese": "优化", "partOfSpeech": "n."},
    {"english": "Prototype", "phonetic": "/ˈproʊtəˌtaɪp/", "chinese": "原型", "partOfSpeech": "n."},
    {"english": "Compile", "phonetic": "/kəmˈpaɪl/", "chinese": "编译", "partOfSpeech": "v."},
    {"english": "Optimize", "phonetic": "/ˈɒptɪmaɪz/", "chinese": "优化", "partOfSpeech": "v."},
    {"english": "Scalable", "phonetic": "/ˈskeɪləbl/", "chinese": "可扩展的", "partOfSpeech": "adj."},
    {"english": "Robust", "phonetic": "/roʊˈbʌst/", "chinese": "强健的", "partOfSpeech": "adj."},
    {"english": "Modular", "phonetic": "/ˈmɒdjʊlər/", "chinese": "模块化的", "partOfSpeech": "adj."},
    {"english": "Intelligent", "phonetic": "/ɪnˈtelɪdʒənt/", "chinese": "智能的", "partOfSpeech": "adj."}
]

# 创建images目录（如果不存在）
if not os.path.exists('images'):
    os.makedirs('images')

# 设置图片尺寸（2.13寸墨水屏分辨率为212x104）
WIDTH = 212
HEIGHT = 104

# 加载字体（请确保这些字体文件存在，或替换为系统中可用的字体）
try:
    english_font = ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial Bold.ttf', 36)  # 英文单词字体
    phonetic_font = ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf', 20)  # 音标字体
    chinese_font = ImageFont.truetype('/System/Library/Fonts/PingFang.ttc', 24)  # 中文字体
except:
    print("警告：无法加载指定字体，将使用默认字体")
    english_font = ImageFont.load_default()
    phonetic_font = ImageFont.load_default()
    chinese_font = ImageFont.load_default()

# 为每个单词生成图片
for i, word in enumerate(words):
    # 创建新图片，使用1位色彩模式（黑白）
    img = Image.new('1', (WIDTH, HEIGHT), 255)  # 255为白色背景
    draw = ImageDraw.Draw(img)
    
    # 绘制英文单词
    english_text = word['english']
    english_bbox = draw.textbbox((0, 0), english_text, font=english_font)
    english_width = english_bbox[2] - english_bbox[0]
    english_x = (WIDTH - english_width) // 2
    draw.text((english_x, 5), english_text, font=english_font, fill=0)
    
    # 绘制音标
    phonetic_text = word['phonetic']
    phonetic_bbox = draw.textbbox((0, 0), phonetic_text, font=phonetic_font)
    phonetic_width = phonetic_bbox[2] - phonetic_bbox[0]
    phonetic_x = (WIDTH - phonetic_width) // 2
    draw.text((phonetic_x, 35), phonetic_text, font=phonetic_font, fill=0)
    
    # 绘制分隔线
    draw.line([(20, 60), (WIDTH-20, 60)], fill=0, width=1)
    
    # 绘制词性和中文释义
    pos_cn_text = f"{word['partOfSpeech']} {word['chinese']}"
    pos_cn_bbox = draw.textbbox((0, 0), pos_cn_text, font=chinese_font)
    pos_cn_width = pos_cn_bbox[2] - pos_cn_bbox[0]
    pos_cn_x = (WIDTH - pos_cn_width) // 2
    draw.text((pos_cn_x, 65), pos_cn_text, font=chinese_font, fill=0)
    
    # 绘制例句
    
    # 保存图片前进行水平镜像处理
    img = img.transpose(Image.FLIP_LEFT_RIGHT)
    # 保存图片，使用1位色深、无压缩的格式
    img.save(f'images/word_{i}.bmp', 'BMP', bits=1)

print("所有单词卡片图片已生成完成！")