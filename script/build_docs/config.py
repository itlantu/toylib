"""文档构建的全部配置, build_docs.py 和 conf.py 共用

路径、模板占位符这些零散常量收在 BuildConfig 里, 别的文件一句
`from config import BuildConfig` 就够, 不用把十几个名字一个个列出来。

Sphinx 的配置项是小写变量名, Doxygen 的是大写键名, 两套规范不同,
所以再分成 SphinxConfig 和 DoxygenConfig 两个类, 而不是硬塞进 BuildConfig:
混在一个类里既不好读, 也没法共用同一套序列化规则。

conf.py 之所以还存在, 是因为 Sphinx 只认这个文件名, 它只负责把 SphinxConfig 展开成模块变量。
"""

from pathlib import Path


class BuildConfig:
    """构建流程自己用到的路径、占位符和依赖清单

    属性名保持大写, 用起来和模块级常量一样, 只是多一层 BuildConfig. 前缀:
    类体本身就是命名空间, 写成 BuildConfig.DOCS_DIR 比逐个 import 更能看出值从哪来
    """

    # ---------- 路径 ----------
    SCRIPT_PATH = Path(__file__).resolve()
    SCRIPT_DIR = SCRIPT_PATH.parent
    ROOT_DIR = SCRIPT_DIR.parents[1]
    # 生成物里标注"本文件由谁生成", 用相对路径, 换机器也不会写死绝对路径
    SCRIPT_REL = SCRIPT_PATH.relative_to(ROOT_DIR).as_posix()

    REQUIREMENTS = ROOT_DIR / "requirements.txt"
    INCLUDE_DIR = ROOT_DIR / "include"
    # 文档结构的根: 剥掉最外层的 toylib, 页面就按 utils/error 这种相对路径铺开
    PACKAGE_DIR = INCLUDE_DIR / "toylib" if (INCLUDE_DIR / "toylib").is_dir() else INCLUDE_DIR

    # 生成物统一落在这里, 整个目录不进版本库
    DOCS_DIR = ROOT_DIR / "docs"
    SOURCE_DIR = DOCS_DIR / "src"
    HTML_DIR = DOCS_DIR / "html"
    XML_DIR = DOCS_DIR / "xml"
    DOXYFILE = DOCS_DIR / "Doxyfile"
    # Sphinx 的中间态放发布目录外面, 否则 .doctrees 会被一起上传和发布
    DOCTREES_DIR = DOCS_DIR / "doctrees"

    # ---------- 首页模板 ----------
    # 正文模板, 里面用占位符标出由脚本填的位置
    INDEX_TEMPLATE = SCRIPT_DIR / "index.rst"
    MODULE_PLACEHOLDER = "@MODULES@"
    # 构建信息来源: 分支, 提交, 构建时间
    BRANCH_PLACEHOLDER = "@BRANCH@"
    COMMIT_PLACEHOLDER = "@COMMIT@"
    BUILT_AT_PLACEHOLDER = "@BUILT_AT@"

    # ---------- 依赖 ----------
    # Sphinx 侧需要的包, 缺了直接退出, 不要等到构建一半才发现
    REQUIRED_MODULES = ("sphinx", "breathe", "furo")


class SphinxConfig:
    """Sphinx 配置

    属性名就是 conf.py 里的变量名, to_dict() 的结果直接展开进 conf.py 的全局命名空间。
    """

    def __init__(self) -> None:
        # 项目信息
        self.project = "toylib"
        self.author = "toylib"
        self.copyright = "2026, toylib"

        # 界面和章节标题用中文
        self.language = "zh_CN"

        self.extensions = ["breathe"]

        # 头文件注释由 Doxygen 解析成 XML, 这里告诉 Breathe 去哪读
        self.breathe_projects = {"toylib": BuildConfig.XML_DIR.as_posix()}
        self.breathe_default_project = "toylib"
        # 头文件按 C 解析, 不明确指定的话 Breathe 有可能当成 C++ 处理
        self.breathe_domain_by_extension = {"h": "c"}

        self.html_theme = "furo"
        self.html_title = "toylib 数据结构库"
        # 源码里没有静态资源目录, 留空避免 Sphinx 警告
        self.html_static_path = []

        self.source_suffix = {".rst": "restructuredtext"}
        # 源目录是构建时自动生成的 docs/src, 里面只有 rst, 没有需要排除的东西
        self.exclude_patterns = []

        # dlist.h 是 typedef struct DListNode {...} DListNode 的写法, Breathe 会把 struct 和 typedef
        # 各声明一次, Sphinx 判定为重复声明。两处内容完全一致, 只影响链接目标, 这里静默掉
        self.suppress_warnings = ["duplicate_declaration.c"]

    def to_dict(self) -> dict[str, object]:
        """导出成可以 update 进 conf.py 的字典

        @return 变量名到值的映射
        """
        return dict(vars(self))


class DoxygenConfig:
    """Doxygen 配置

    属性名就是 Doxyfile 的键, 属性的定义顺序就是输出顺序, 由 to_doxyfile 序列化成 "键 = 值"。
    布尔值输出成 YES/NO, 路径统一用斜杠, 列表用反斜杠续行。

    输入输出目录直接取 BuildConfig, 和 SphinxConfig 一样不需要外部传路径进来。
    """

    def __init__(self, predefined: list[str]) -> None:
        # 项目信息
        self.PROJECT_NAME = "toylib"
        self.PROJECT_BRIEF = "用C实现的基础数据结构库"
        self.OUTPUT_DIRECTORY = BuildConfig.DOCS_DIR
        self.XML_OUTPUT = "xml"
        self.OUTPUT_LANGUAGE = "Chinese"

        # 只扫头文件: 公开接口都在include里, 实现细节不用进文档
        self.INPUT = BuildConfig.INCLUDE_DIR
        self.FILE_PATTERNS = "*.h"
        self.RECURSIVE = True
        self.INPUT_ENCODING = "UTF-8"

        # 不同目录下的同名头文件必须能区分: 让 XML 里的 location 带上相对路径。
        # 注意剥离点是 INCLUDE_DIR 而不是 PACKAGE_DIR, 这样连顶级头文件也带分隔符(toylib/queue.h),
        # Breathe 才会走"尾部精确匹配"而不是"只比文件名", 否则 adt/queue.h 和 queue.h 仍会撞。
        self.FULL_PATH_NAMES = True
        self.STRIP_FROM_PATH = BuildConfig.INCLUDE_DIR

        # 只输出有注释的成员, 漏注释会被 WARN_IF_UNDOCUMENTED 报出来
        self.EXTRACT_ALL = False
        self.EXTRACT_STATIC = False
        self.HIDE_UNDOC_MEMBERS = True

        # 把可配置的宏定义成头文件里的默认值, 否则函数签名里显示的是宏名而不是实际类型
        self.ENABLE_PREPROCESSING = True
        self.MACRO_EXPANSION = True
        self.PREDEFINED = predefined

        # 只要 XML, HTML 交给 Sphinx 生成
        self.GENERATE_HTML = False
        self.GENERATE_LATEX = False
        self.GENERATE_XML = True
        self.XML_PROGRAMLISTING = False

        # 不做调用图, 三个平台就都不用装 graphviz
        self.HAVE_DOT = False

        self.QUIET = True
        self.WARNINGS = True
        self.WARN_IF_UNDOCUMENTED = True
        self.WARN_IF_DOC_ERROR = True
        self.WARN_NO_PARAMDOC = True

    @staticmethod
    def _format_value(value: object) -> list[str]:
        """把属性值转成一行或多行文本

        @param value 属性值, 支持布尔/路径/列表/其它
        @return 值的文本行, 多于一行时由调用方补续行符
        """
        # bool 要排在 int 前面, 因为 bool 是 int 的子类
        if isinstance(value, bool):
            return ["YES" if value else "NO"]
        if isinstance(value, Path):
            return [value.as_posix()]
        if isinstance(value, (list, tuple)):
            # 空列表至少要产出一个空值行, 否则这个键会被整行丢掉
            return [str(item) for item in value] or [""]
        return [str(value)]

    def to_doxyfile(self) -> str:
        """序列化成 Doxyfile 文本

        @return 完整文件内容, 键按等宽对齐, 列表值用反斜杠续行
        """
        # 类约定"属性名即键", 所以实例属性直接就是条目, 不用另外维护键名列表
        items = list(vars(self).items())
        width = max(len(key) for key, _ in items)
        indent = " " * (width + 3)

        lines = [f"# 本文件由 {BuildConfig.SCRIPT_REL} 生成, 不要手改", ""]
        for key, value in items:
            values = self._format_value(value)
            for index, text in enumerate(values):
                head = f"{key:<{width}} = " if index == 0 else indent
                # 除了最后一项都要续行; 末尾不留空格
                tail = " \\" if index < len(values) - 1 else ""
                lines.append(f"{head}{text}{tail}".rstrip())

        return "\n".join(lines) + "\n"
