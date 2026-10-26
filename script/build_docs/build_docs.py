"""构建 API 文档: Doxygen 解析头文件注释成 XML, 再由 Breathe 交给 Sphinx 渲染成 HTML

用法:
    python script/build_docs/build_docs.py
    python script/build_docs/build_docs.py --clean
    python script/build_docs/build_docs.py --allow-warnings

依赖:
    doxygen 在 PATH 里
    sphinx / breathe / furo 装在当前解释器里, 见仓库根目录的 requirements.txt

目录约定:
    script/build_docs/   脚本, 配置和首页源文件, 全部进版本库
      config.py          所有配置(路径, Sphinx, Doxygen)都在这
      conf.py            Sphinx 入口, 只把 config.py 的配置展开成模块变量
      index.rst          首页正文, 模块列表和构建信息由脚本填
    docs/                全是生成物, 整个不进版本库
      Doxyfile           按头文件里的宏自动生成
      src/               按头文件自动生成的 rst
      xml/               Doxygen 产出, 给 Breathe 读
      html/              最终页面

    头文件增删、宏增删都不用改这里: 页面列表和 PREDEFINED 都是从 include 下扫出来的。
    include 下的目录结构会原样映射成文档层级: utils/error.h 落在 api/utils/error,
    并自动生成 api/utils/index.rst 承担那一层目录, 所以新增子模块、模块里再嵌套都不用改配置。
"""

import argparse
import importlib.util
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

# 配置和路径全在 config.py, 这里只取需要用的; 直接运行本文件时 sys.path[0] 就是本目录
from config import (
    BRANCH_PLACEHOLDER,
    BUILT_AT_PLACEHOLDER,
    COMMIT_PLACEHOLDER,
    DOCS_DIR,
    DOCTREES_DIR,
    DOXYFILE,
    DoxygenConfig,
    HTML_DIR,
    INCLUDE_DIR,
    INDEX_TEMPLATE,
    MODULE_PLACEHOLDER,
    PACKAGE_DIR,
    REQUIREMENTS,
    REQUIRED_MODULES,
    ROOT_DIR,
    SCRIPT_DIR,
    SOURCE_DIR,
    XML_DIR,
)

# 文档层级的根(PACKAGE_DIR 本身), 用来区分"顶级"和"子目录"
TOP = Path(".")

# index.xml 里的一条文件复合项; 文件复合名默认只有纯文件名。
# 两个属性用前瞻匹配, 不依赖 doxygen 输出的属性顺序; 匹配不上时什么都不改, 后面会自动退回纯文件名
FILE_COMPOUND_PATTERN = re.compile(
    r'(<compound\b(?=[^>]*\bkind="file")(?=[^>]*\brefid="([^"]+)")[^>]*>\s*<name>)([^<]*)(</name>)'
)
# compound 文件里 compounddef 自身的 location(不带 line/column 的那种)
XML_LOCATION_PATTERN = re.compile(r'<location file="([^"]+)"\s*/>')

# 形如 "#ifndef X" 紧跟 "#define X <值>" 的块, 是可配置默认值的统一写法(元素类型、容量上限、哨兵等)。
# 只匹配对象式宏: 包含卫哨没有值, 函数式宏的名字后面直接跟括号, 两者都命中不了。
# 这是解析规则而不是配置, 所以留在本文件。
DEFAULT_MACRO_PATTERN = re.compile(
    r"^[ \t]*#[ \t]*ifndef[ \t]+(\w+)[ \t]*\r?\n[ \t]*#[ \t]*define[ \t]+\1[ \t]+([^\r\n]+)",
    re.MULTILINE,
)


def ensure_utf8_output() -> None:
    """把标准输出切成 UTF-8

    @note Windows 上 stdout 接管道或重定向时用的是系统 ANSI 代码页(cp1252 之类), 打印中文会直接抛
          UnicodeEncodeError; 控制台反而是 UTF-8 不受影响, 所以这里无条件设一次最省心
    """
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")


def preflight() -> str:
    """构建前的环境检查

    @return doxygen 可执行文件路径
    @note 缺东西时直接给出能照抄的处理办法, 而不是等构建到一半才报错
    """
    doxygen = shutil.which("doxygen")
    if doxygen is None:
        raise SystemExit("找不到 doxygen, 请先安装并确保它在 PATH 里")

    missing = [name for name in REQUIRED_MODULES if importlib.util.find_spec(name) is None]
    if missing:
        print(f"缺少 Python 依赖: {', '.join(missing)}", file=sys.stderr)
        print(f"安装命令: {sys.executable} -m pip install -r {REQUIREMENTS}", file=sys.stderr)
        raise SystemExit(1)

    return doxygen


def collect_headers() -> list[Path]:
    """收集需要进文档的头文件

    @return 头文件路径列表, 按相对路径排序, 同目录的排在一起
    @note 内容全部来自头文件里的注释, 所以只扫 include, 不看 src;
          按完整路径排而不是只按文件名, 保证不同机器上的顺序一致
    """
    headers = sorted(INCLUDE_DIR.rglob("*.h"), key=lambda path: path.as_posix())
    if not headers:
        raise SystemExit(f"{INCLUDE_DIR} 下没有找到头文件")
    return headers


def collect_predefined(headers: list[Path]) -> list[str]:
    """扫出头文件里带默认值的宏, 用来填 Doxyfile 的 PREDEFINED

    @param headers collect_headers 的结果
    @return 形如TOYLIB_QUEUE_VALUE_TYPE=int的列表, 按头文件顺序去重
    @note 新增一个带可配置宏的头文件时, 不需要回来改这里的任何东西
    """
    predefined: list[str] = []
    seen: set[str] = set()
    for header in headers:
        for macro, value in DEFAULT_MACRO_PATTERN.findall(header.read_text(encoding="utf-8", errors="replace")):
            if macro not in seen:
                seen.add(macro)
                predefined.append(f"{macro}={value.strip()}")
    return predefined


@dataclass(frozen=True)
class BuildInfo:
    """本次构建的来源信息, 会写进首页

    @note 三个字段都是可以直接填进模板的文本, 取值和格式化都交给 collect_build_info,
          渲染那边只负责替换
    """

    branch: str
    commit: str
    built_at: str


def git_output(*args: str) -> str:
    """读一条 git 信息

    @param args git 的子命令和参数
    @return 去掉首尾空白的输出, 取不到时返回空串
    @note 构建机上不一定有 git(比如只解压了源码包), 所以失败一律静默降级, 不影响出文档
    """
    try:
        result = subprocess.run(["git", *args], cwd=ROOT_DIR, capture_output=True, text=True, check=True)
    except (OSError, subprocess.CalledProcessError):
        return ""
    return result.stdout.strip()


def collect_build_info() -> BuildInfo:
    """收集分支, 提交和构建时间

    @return 写进首页的构建信息
    @note GitHub Actions 会提供 GITHUB_REF_NAME / GITHUB_SHA, 优先用它们:
          CI 里是浅克隆 + 游离 HEAD, git 命令拿不到分支名;
          本地则退回 git, 再取不到就写"未知", 不让这点信息把构建卡住
    """
    branch = os.environ.get("GITHUB_REF_NAME") or git_output("rev-parse", "--abbrev-ref", "HEAD")
    commit = os.environ.get("GITHUB_SHA") or git_output("rev-parse", "HEAD")

    return BuildInfo(
        branch=branch or "未知",
        commit=commit[:7] or "未知",
        # 带时区偏移, CI 是 UTC, 本地是 UTC+8, 都如实写出来
        built_at=datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %z"),
    )


def qualify_file_names() -> set[str]:
    """把 index.xml 里的文件复合名补成带目录的相对路径

    @return 改写之后可用的文件复合名, 拿不到路径的项不在里面
    @note Breathe 的 doxygenfile 只按 index.xml 里的 <name> 找文件, 而 doxygen 在那里只写纯文件名,
          于是两个目录下的同名头文件会被判成"多个匹配"。
          doxygen 自己其实给它们分了不同的 refid(adt_2queue_8h / queue_8h),
          把 refid 对应文件里的 location 补回 name, 名字就唯一了
    """
    index_file = XML_DIR / "index.xml"
    names: set[str] = set()

    def replace(match: re.Match[str]) -> str:
        head, refid, _old_name, tail = match.groups()
        compound = XML_DIR / f"{refid}.xml"
        location = XML_LOCATION_PATTERN.search(compound.read_text(encoding="utf-8")) if compound.is_file() else None
        if location is None:
            return match.group(0)

        names.add(location.group(1))
        return f"{head}{location.group(1)}{tail}"

    index_file.write_text(FILE_COMPOUND_PATTERN.sub(replace, index_file.read_text(encoding="utf-8")), encoding="utf-8")
    return names


def generate_doxyfile(predefined: list[str]) -> None:
    """生成 Doxyfile

    @param predefined PREDEFINED 的内容
    @note 输入输出都写绝对路径, 这样 doxygen 从哪个目录启动都能找到
    """
    config = DoxygenConfig(output_dir=DOCS_DIR, input_dir=INCLUDE_DIR, predefined=predefined)
    DOXYFILE.write_text(config.to_doxyfile(), encoding="utf-8")

    summary = ", ".join(predefined) if predefined else "无"
    print(f"识别到 {len(predefined)} 个可配置宏: {summary}")


def render_page(title: str, entries: list[str]) -> str:
    """生成一个只含 toctree 的中间页

    @param title 页面标题
    @param entries toctree 条目, 相对于本页所在目录
    @return 页面正文
    @note 子目录用它生成 index.rst, 这样嵌套结构在侧边栏里就是一层一层的
    """
    body = "".join(f"   {entry}\n" for entry in entries)
    return f"{title}\n{'=' * len(title)}\n\n.. toctree::\n   :maxdepth: 2\n\n{body}"


def render_index(entries: list[str], info: BuildInfo) -> str:
    """把首页模板里的占位符换成模块列表和构建信息

    @param entries toctree 条目, 相对于 docs/src
    @param info 本次构建的分支/提交/时间
    @return 替换好的首页正文
    @note 正文写在 script/build_docs/index.rst, 这里只做占位符替换;
          toctree 续行的缩进从占位符所在行读出来, 所以模板怎么缩进都对
    """
    template = INDEX_TEMPLATE.read_text(encoding="utf-8")
    match = re.search(rf"^([ \t]*){re.escape(MODULE_PLACEHOLDER)}", template, re.MULTILINE)
    if match is None:
        raise SystemExit(f"{INDEX_TEMPLATE} 里找不到占位符 {MODULE_PLACEHOLDER}")

    separator = "\n" + match.group(1)
    text = template.replace(MODULE_PLACEHOLDER, separator.join(entries))

    values = {
        BRANCH_PLACEHOLDER: info.branch,
        COMMIT_PLACEHOLDER: info.commit,
        BUILT_AT_PLACEHOLDER: info.built_at,
    }
    for placeholder, value in values.items():
        # 占位符被误删时直接报错, 别静默生成一份没有构建信息的首页
        if placeholder not in text:
            raise SystemExit(f"{INDEX_TEMPLATE} 里找不到占位符 {placeholder}")
        text = text.replace(placeholder, value)

    return text


def generate_sources(headers: list[Path], file_names: set[str], info: BuildInfo) -> None:
    """按头文件生成 Sphinx 需要的 rst, 目录结构与头文件一致

    @param headers collect_headers 的结果
    @param file_names qualify_file_names 返回的可用复合名
    @param info 本次构建的分支/提交/时间, 只用于首页
    @note 每个页面正文只有一句 doxygenfile 指令, 函数列表和文档在构建时由 Breathe 渲染,
          所以头文件里增删函数都不用动这里;
          utils/error.h 会落到 api/utils/error.rst, 并自动生成 api/utils/index.rst 承担那层目录,
          所以新增子模块、模块里再套模块都不需要改任何配置
    """
    api_dir = SOURCE_DIR / "api"
    shutil.rmtree(api_dir, ignore_errors=True)

    # 目录 -> 该目录下直接包含的模块名
    modules: dict[Path, list[str]] = defaultdict(list)

    for header in headers:
        rel = header.relative_to(PACKAGE_DIR)
        page = api_dir / rel.with_suffix(".rst")
        page.parent.mkdir(parents=True, exist_ok=True)

        # 优先用带目录的相对路径(唯一), 拿不到就退回纯文件名
        target = header.relative_to(INCLUDE_DIR).as_posix()
        if target not in file_names:
            target = header.name

        title = header.stem
        page.write_text(f"{title}\n{'=' * len(title)}\n\n.. doxygenfile:: {target}\n", encoding="utf-8")
        modules[rel.parent].append(title)

    # 出现过的所有目录(不含根), 连同它的每一级父目录
    folders = {folder for parent in modules for folder in (parent, *parent.parents)} - {TOP}

    # 子目录各自生成 index, 把嵌套结构铺开
    for folder in sorted(folders):
        (api_dir / folder).mkdir(parents=True, exist_ok=True)
        children = [f"{sub.name}/index" for sub in sorted(folders) if sub.parent == folder]
        (api_dir / folder / "index.rst").write_text(
            render_page(folder.name, modules[folder] + children), encoding="utf-8"
        )

    # 首页列出最外层的模块和子目录
    top = [f"api/{name}" for name in modules[TOP]]
    top += [f"api/{sub.name}/index" for sub in sorted(folders) if sub.parent == TOP]
    (SOURCE_DIR / "index.rst").write_text(render_index(top, info), encoding="utf-8")

    print(f"生成了 {len(headers)} 个模块页面, {len(folders)} 个目录页")


def run(command: list[str], title: str) -> None:
    """执行一条命令, 失败就往上抛

    @param command 要执行的命令, 第一项是可执行文件
    @param title 步骤名, 只用于输出提示
    @note 工作目录固定为仓库根目录; 生成的 Doxyfile 里都是绝对路径, 并不依赖这一点
    """
    print(f"\n==> {title}")
    print("    " + " ".join(command))
    subprocess.run(command, cwd=ROOT_DIR, check=True)


def clean_output() -> None:
    """删掉上次的生成物

    @note Windows 上文件被占用时 rmtree 会失败, 不让它把整个构建带崩, 但要如实说出来
    """
    print(f"清理 {DOCS_DIR}")
    shutil.rmtree(DOCS_DIR, ignore_errors=True)
    if DOCS_DIR.exists():
        print(f"警告: {DOCS_DIR} 没有完全删掉, 接着按增量构建")


def build(clean: bool, allow_warnings: bool) -> Path:
    """跑完整个文档构建流程

    @param clean 构建前是否删掉 docs 目录(里面全是生成物)
    @param allow_warnings True 则不把 Sphinx 警告当成错误
    @return 生成的 HTML 目录
    """
    doxygen = preflight()

    if clean:
        clean_output()

    DOCS_DIR.mkdir(parents=True, exist_ok=True)

    # 只扫一次头文件, 结果同时喂给 Doxyfile 和 rst
    headers = collect_headers()
    generate_doxyfile(collect_predefined(headers))

    # 构建信息只进首页那一小块, 先收集好, 后面写 rst 时直接用
    info = collect_build_info()
    print(f"构建来源: 分支 {info.branch}, 提交 {info.commit}, {info.built_at}")

    run([doxygen, str(DOXYFILE)], "Doxygen 生成 XML")

    # 得等 doxygen 跑完才知道每个文件在 XML 里叫什么, 所以 rst 放在它后面写
    generate_sources(headers, qualify_file_names(), info)

    # -c 指定配置目录(script/build_docs), -d 把中间态挪出发布目录, 源目录是自动生成的 docs/src
    sphinx = [
        sys.executable,
        "-m",
        "sphinx",
        "-b",
        "html",
        "--keep-going",
        "-c",
        str(SCRIPT_DIR),
        "-d",
        str(DOCTREES_DIR),
    ]
    if not allow_warnings:
        sphinx.append("-W")
    sphinx += [str(SOURCE_DIR), str(HTML_DIR)]
    run(sphinx, "Sphinx 生成 HTML")

    return HTML_DIR


def main() -> int:
    ensure_utf8_output()

    parser = argparse.ArgumentParser(description="构建 toylib 的 HTML API 文档")
    parser.add_argument("--clean", action="store_true", help="构建前删掉 docs 目录")
    parser.add_argument("--allow-warnings", action="store_true", help="不把 Sphinx 警告当成错误")
    args = parser.parse_args()

    try:
        html_dir = build(clean=args.clean, allow_warnings=args.allow_warnings)
    except subprocess.CalledProcessError as error:
        print(f"\n构建失败: 命令返回 {error.returncode}", file=sys.stderr)
        return error.returncode
    except KeyboardInterrupt:
        print("\n已中断", file=sys.stderr)
        return 130

    print(f"\n完成, 入口页面: {html_dir / 'index.html'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
