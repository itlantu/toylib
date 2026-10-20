/*
 * 测试程序入口
 */

#if defined(_MSC_VER) && defined(_DEBUG)

/*
 * Debug版MSVC CRT检测到堆破坏时默认弹出对话框, 导致CI卡住
 * 把断言和错误的报告方式改成写stderr, 冲突时立即中止
 */

#include <crtdbg.h>

namespace {
struct CrtReportToStderr {
    CrtReportToStderr() {
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    }
} g_crt_report_to_stderr;
}  // namespace
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
