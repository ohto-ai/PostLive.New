#include "WinDbgHelper.h"
#include <utils/dll_crt.h>
#include <mutex>
#include <DbgHelp.h>

static std::once_flag initFlag;

static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo) {
    if (IsDebuggerPresent()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    ohtoai::rt::lib dbg_help_dll(TEXT("dbghelp.dll"));

    // Create a minidump file
    HANDLE hFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = lpExceptionInfo;
        mdei.ClientPointers = FALSE;
        dbg_help_dll.call("MiniDumpWriteDump", GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, NULL, NULL);
        CloseHandle(hFile);
        MessageBox(NULL, L"An unexpected error occurred. A crash dump has been saved to crash.dmp.", L"Error", MB_OK | MB_ICONERROR);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

WinDbgHelper::WinDbgHelper() {
    std::call_once(initFlag, []() {
        SetUnhandledExceptionFilter(ExceptionFilter);
        });
}