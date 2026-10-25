"""Sphinx 的配置入口

Sphinx 只认 conf.py 这个文件名(-c 只能指定目录, 不能换文件名), 所以这个文件只做两件事:
把本目录放进 sys.path, 然后把 config.py 里的 SphinxConfig 展开成模块变量。
真正的配置都在 config.py。
"""

import sys
from pathlib import Path

# Sphinx 一般会把配置目录加进 sys.path, 这里显式加一次, 不依赖它的实现细节
sys.path.insert(0, str(Path(__file__).resolve().parent))

from config import SphinxConfig

globals().update(SphinxConfig().to_dict())
