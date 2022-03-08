#include "WinInterop.h"
#include "Main.h"
#include "Settings.h"

#include "Utility.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj_core.h>
#include <string>
#include <thread>

File::File(char const* filename, File::Mode fileMode, bool updateFile)
{
    std::string sFileName = std::string(filename);
    Init(sFileName, fileMode, updateFile);
}

File::File(const std::string& filename, File::Mode fileMode, bool updateFile)
{
    Init(filename, fileMode, updateFile);
}

void File::GetHandle()
{
    //MultiByteToWideChar(CP_UTF8, );
    //TCHAR* test = TEXT("TEST");
    m_handle = CreateFile(m_filename.c_str(), m_accessType, m_shareType,
        NULL, m_openType, FILE_ATTRIBUTE_NORMAL, NULL);
}

void File::Init(const std::string& filename, File::Mode fileMode, bool createIfNotFound)
{
    m_filename = std::string(filename);
    m_accessType = GENERIC_READ;
    m_shareType  = FILE_SHARE_READ;
    m_openType   = OPEN_EXISTING;
    //m_fileAttribute = FILE_ATTRIBUTE_NORMAL;

    switch (fileMode)
    {
    case File::Mode::Read:
        m_accessType = GENERIC_READ;
        m_shareType  = FILE_SHARE_READ;
        //m_fileAttribute = FILE_ATTRIBUTE_READONLY;
        break;
    case File::Mode::Write:
        m_openType = TRUNCATE_EXISTING;
        //[[fallthrough]];
    //case File::Mode::Append:
        m_accessType = GENERIC_WRITE;
        m_shareType = FILE_SHARE_WRITE;
        break;
    default:
        break;
    }
    GetHandle();

    if (createIfNotFound && m_handle == INVALID_HANDLE_VALUE)
    {
        m_openType = CREATE_NEW;
        GetHandle();
    }
    m_handleIsValid = (m_handle != INVALID_HANDLE_VALUE);
    //assert(m_handleIsValid);
    auto filePointerLocation = FILE_END;

    if (m_handleIsValid)
    {
        switch (fileMode)
        {
        case File::Mode::Read:
            filePointerLocation = FILE_BEGIN;
            [[fallthrough]];
        //case File::Mode::Append:
        //    break;
        default:
            break;
        }
    }

    DWORD newFilePointer = SetFilePointer(m_handle, 0, NULL, filePointerLocation);
}

bool File::FileDestructor()
{
    return CloseHandle(m_handle);
}

File::~File()
{
    if (m_handleIsValid)
    {
        FileDestructor();
    }
}


void File::GetText()
{
    if (!m_handleIsValid)
        return;

    uint32 bytesRead;
    static_assert(sizeof(DWORD) == sizeof(uint32));
    static_assert(sizeof(LPVOID) == sizeof(void*));

    const uint32 fileSize = GetFileSize(m_handle, NULL);
    m_dataString.resize(fileSize, 0);
    m_textIsValid = true;
    if (!ReadFile(m_handle, (LPVOID)m_dataString.c_str(), (DWORD)fileSize, reinterpret_cast<LPDWORD>(&bytesRead), NULL))
    {
        //assert(false);
        m_textIsValid = false;
    }
}

void File::GetData()
{
    if (!m_handleIsValid)
        return;

    uint32 bytesRead;
    static_assert(sizeof(DWORD) == sizeof(uint32));
    static_assert(sizeof(LPVOID) == sizeof(void*));

    const uint32 fileSize = GetFileSize(m_handle, NULL);
    m_dataBinary.resize(fileSize, 0);
    m_binaryDataIsValid = true;
    if (ReadFile(m_handle, (LPVOID)m_dataBinary.data(), (DWORD)fileSize, reinterpret_cast<LPDWORD>(&bytesRead), NULL) == 0)
    {
        m_binaryDataIsValid = false;
        DWORD error = GetLastError();
    }
}
bool File::Write(void* data, size_t sizeInBytes)
{
    DWORD bytesWritten = {};
    BOOL result = WriteFile(m_handle, data, (DWORD)sizeInBytes, &bytesWritten, NULL);
    return result != 0;
}

bool File::Write(const void* data, size_t sizeInBytes)
{
    DWORD bytesWritten = {};
    BOOL result = WriteFile(m_handle, data, (DWORD)sizeInBytes, &bytesWritten, NULL);
    return result != 0;
}

bool File::Write(const std::string& text)
{
    DWORD bytesWritten = {};
    BOOL result = WriteFile(m_handle, text.c_str(), (DWORD)text.size(), &bytesWritten, NULL);
    return result != 0;
    //int32 result = fputs(text.c_str(), m_handle);
    //return (!(result == EOF));
    //return false;
}

void File::GetTime()
{
    FILETIME creationTime;
    FILETIME lastAccessTime;
    FILETIME lastWriteTime;
    if (!GetFileTime(m_handle, &creationTime, &lastAccessTime, &lastWriteTime))
    {
        DebugPrint("GetFileTime failed with %d\n", GetLastError());
        m_timeIsValid = false;
    }
    else
    {
        ULARGE_INTEGER actualResult;
        actualResult.LowPart = lastWriteTime.dwLowDateTime;
        actualResult.HighPart = lastWriteTime.dwHighDateTime;
        m_time = actualResult.QuadPart;
        m_timeIsValid = true;
    }
}

bool File::Delete()
{
    if (m_handleIsValid)
    {
        FileDestructor();
        //std::wstring fuckingWide(m_filename.begin(), m_filename.end());
        bool result = DeleteFile(m_filename.c_str());
        m_handleIsValid = false;
        return result;
    }
    return false;
}

bool CreateFolder(const std::string& folderLocation)
{
    BOOL result = CreateDirectoryA(folderLocation.c_str(), NULL);
    return !(result == 0);
}

void DebugPrint(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, list);
    OutputDebugStringA(buffer);
    va_end(list);
}

static const char* s_consoleColors[ConsoleColor_Count] = {
    "\033[0;30m",
    "\033[0;31m",
    "\033[0;32m",
    "\033[0;33m",
    "\033[0;34m",
    "\033[0;35m",
    "\033[0;36m",
    "\033[0;37m",
};

void ConsoleOutput(const std::string& text, ConsoleColor color)
{
    printf("%s%s%s\n", s_consoleColors[color], text.c_str(), s_consoleColors[ConsoleColor_White]);
}

std::string ToString(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    int32 i = vsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);
    return buffer;
}

//
// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2017
// Usage: SetThreadName ((DWORD)-1, "MainThread");
//
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
 } THREADNAME_INFO;
#pragma pack(pop)
void SetThreadName(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try{
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
    }
#pragma warning(pop)
}

void SetThreadName(std::thread::native_handle_type threadID, std::string name)
{
    SetThreadName(GetThreadId(threadID), name.c_str());
}

int32 CreateMessageWindow(const char* message, const char* title, uint32 type)
{
    return MessageBox(NULL, (LPCTSTR)message, (LPCTSTR)title, type);
}

void CreateErrorWindow(const char* message)
{
    assert(false);
    CreateMessageWindow(message, "MinerManager ERROR", MB_OK);
}

#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

void GetFullProcessList(std::vector<ProcessInfo>& output)
{
    output.clear();
    // Get processes
    DWORD processIDs[PROCESS_COUNT] = {};
    DWORD bytesUsedInProcessIDs;
    if (!EnumProcesses(processIDs, sizeof(processIDs), &bytesUsedInProcessIDs))
        return;


    const int32 processCount = bytesUsedInProcessIDs / sizeof(DWORD);
    for (int32 i = 0; i < processCount; i++)
    {
        if (processIDs[i] != 0)
        {
            DWORD& processID = processIDs[i];

            // Get a handle to the process.
            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

            // Get the process name.
            TCHAR processName[MAX_PATH] = TEXT("<unknown>");
            if (NULL != processHandle)
            {
                HMODULE moduleHandles;
                DWORD bytesUsedInModuleHandles;

                if (EnumProcessModules(processHandle, &moduleHandles, sizeof(moduleHandles), &bytesUsedInModuleHandles))
                {
                    GetModuleBaseName(processHandle, moduleHandles, processName, sizeof(processName) / sizeof(TCHAR));
                    //process is valid
                    {

                        ProcessInfo p;
                        p.name = processName;
                        p.id = processID;
                        output.push_back(p);
#if _DEBUG
                        // Print the process name and identifier.
                        _tprintf(TEXT("%s  (PID: %u)\n"), processName, processID);
                        //DebugPrint(TEXT("%s  (PID: %u)\n"), processName, processID);
#endif
                    }
                }
            }
            // Release the handle to the process.
            CloseHandle(processHandle);
        }
    }
}

bool GetExistingProcessInformation(const std::string& executableName, uint32& processID)
{
    std::vector<ProcessInfo> processList;
    processList.reserve(500);
    GetFullProcessList(processList);
    for (const ProcessInfo& pi : processList)
    {
        if (pi.name == executableName)
        {
            processID = pi.id;
            return true;
        }
    }
    return false;
}

Process::~Process()
{
    End();
}

bool Process::CheckIfAlreadyRunning()
{
    // TODO(choman): search current proccesses to see if one is already running and capture the handle/process information
    if (m_processID)
    {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_processID);
        if (processHandle != NULL)
            return true;
    }

    uint32 processID;
    if (GetExistingProcessInformation(m_exeName, processID))
    {
        m_processID = processID;
        return true;
    }
    m_processID = 0;
    return false;
}

void Process::StartWithCheck(const char* arguments)
{
    if (!CheckIfAlreadyRunning())
        Start(arguments);
}

void Process::Start(const char* arguments)
{
    LPSTR lpCommandLine = const_cast<char*>(arguments);

    SECURITY_ATTRIBUTES processAttributes;
    processAttributes.lpSecurityDescriptor = NULL;
    processAttributes.bInheritHandle = true;
    processAttributes.nLength = sizeof(processAttributes);

    SECURITY_ATTRIBUTES threadAttributes;
    threadAttributes.lpSecurityDescriptor = NULL;
    threadAttributes.bInheritHandle = true;
    threadAttributes.nLength = sizeof(threadAttributes);

    DWORD newProcessFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;

    int32 showWindowFlags = SW_SHOWDEFAULT;
    if (g_settings[setting_StartProcessesMinimized].v_bool)
        showWindowFlags = SW_MINIMIZE;

    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = showWindowFlags;
    startupInfo.cb = sizeof(startupInfo);

    _PROCESS_INFORMATION pi = {};

    BOOL result = CreateProcess(
        m_fileLocation.c_str(), //LPCSTR                  lpApplicationName,
        lpCommandLine,          //LPSTR                   lpCommandLine,
        &processAttributes,     //LPSECURITY_ATTRIBUTES   lpProcessAttributes,
        &threadAttributes,      //LPSECURITY_ATTRIBUTES   lpThreadAttributes,
        true,                   //BOOL                    bInheritHandles,
        newProcessFlags,        //DWORD                   dwCreationFlags,
        NULL,                   //LPVOID                  lpEnvironment,
        NULL,                   //LPCSTR                  lpCurrentDirectory,
        &startupInfo,           //LPSTARTUPINFOA          lpStartupInfo,
        &pi                     //LPPROCESS_INFORMATION   lpProcessInformation
    );
    if (result == 0)
    {
        DWORD CreateProcessError = GetLastError();
        std::string error;
        bool knownError = false;
        switch (CreateProcessError)
        {
        case 740:
        {
            error = "Program needs to be ran as administrator";
            knownError = true;
            break;
        }
        case 3:
        {
            error = ToString("The system cannot find the path specified \"%s\"", m_fileLocation.c_str());
            knownError = true;
            break;
        }
        }
        if (knownError)
        {
            DebugPrint("${red}CreateProcessA error: %s${normal}", error.c_str());
            ConsoleOutput(ToString("CreateProcessA error: %s", error.c_str()), ConsoleColor_Red);
            CreateErrorWindow(ToString("CreateProcessA error: %s", error.c_str()).c_str());
        }
        else
        {
            DebugPrint("${red}CreateProcessA error: %i${normal}", CreateProcessError);
            ConsoleOutput(ToString("CreateProcessA error: %i", CreateProcessError, ConsoleColor_Red));
            CreateErrorWindow(ToString("CreateProcessA error: %i", CreateProcessError).c_str());
        }
        assert(false);
        End(0);
    }
    else
    {
        m_processID = pi.dwProcessId;
        CloseHandles(pi);
    }
}

void Process::CloseHandles(const _PROCESS_INFORMATION& pi)
{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Process::End(uint32 exitCode)
{
    HANDLE processHandle = NULL;
#if 1
    if (GetExistingProcessInformation(m_exeName, m_processID))
    {
        processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, m_processID);
    }

#else
    if (m_processID)
    {
        processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, m_processID);
    }
    if (processHandle == NULL && GetExistingProcessInformation(m_exeName, m_processID))
    {
        processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, m_processID);
    }

#endif

    if (processHandle != NULL)
    {
        if (TerminateProcess(processHandle, exitCode) == 0)
        {
            DWORD errorValue = GetLastError();
            CreateErrorWindow(ToString("TerminateProcess returned: %i on %s, processID: %u", errorValue, m_exeName.c_str(), m_processID).c_str());
        }
        CloseHandle(processHandle);
        m_processID = 0;
        return;
    }
    else
    {
        //process isn't running
        if (m_processID)
            CreateErrorWindow(ToString("Could not find %s process, processID: %u", m_exeName.c_str(), m_processID).c_str());
        m_processID = 0;
        return;
    }
}

#include <chrono>
#include <thread>
void Sleep(float i_seconds)
{
    std::this_thread::sleep_for(std::chrono::duration<float>{i_seconds});
}
