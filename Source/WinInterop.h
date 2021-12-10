#pragma once
//#include "Math.h"
#include "Types.h"
#include "Utility.h"
#include <vector>

#include <string>
#include <thread>

#define PROCESS_COUNT 2048

struct File {
    enum class Mode {
        Read,  //Read:   Open file for read access.
        Write, //Write:  Open and empty file for output.
        //Append,//Append: Open file for output.
    };

    bool m_handleIsValid     = false;
    bool m_textIsValid       = false;
    bool m_timeIsValid       = false;
    bool m_binaryDataIsValid = false;
    std::string         m_filename;
    std::string         m_dataString;
    std::vector<uint8>  m_dataBinary;
    uint64 m_time = {};

    File(char const* fileName,        File::Mode fileMode, bool createIfNotFound);
    File(const std::string& fileName, File::Mode fileMode, bool createIfNotFound);
    ~File();

    bool Write(const std::string& text);
    bool Write(void* data, size_t sizeInBytes);
    bool Write(const void* data, size_t sizeInBytes);
    void GetData();
    void GetText();
    void GetTime();
    bool Delete();

private:

    void* m_handle;
    uint32  m_accessType;
    uint32  m_shareType;
    uint32  m_openType;

    void GetHandle();
    void Init(const std::string& filename, File::Mode fileMode, bool createIfNotFound);
    bool FileDestructor();
};

struct _PROCESS_INFORMATION;

struct Settings;
struct Process {
    Process(std::string fileLocation) : m_fileLocation(fileLocation)
    {
        // TODO(choman): Get exe name from windows process
        const size_t backSlashPosition = m_fileLocation.find_last_of('\\');
        const size_t frontSlashPosition = m_fileLocation.find_last_of('/');
        size_t lastSlashPosition = m_fileLocation.size();

        if (backSlashPosition != std::string::npos && frontSlashPosition != std::string::npos)
            lastSlashPosition = Max(backSlashPosition, frontSlashPosition);
        if (backSlashPosition != std::string::npos)
            lastSlashPosition = backSlashPosition;
        if (frontSlashPosition != std::string::npos)
            lastSlashPosition = frontSlashPosition;

        if (lastSlashPosition = m_fileLocation.size())
        {
            m_exeName = m_fileLocation;
        }
        else
        {
            std::string_view exeName = m_fileLocation;
            exeName.remove_prefix(lastSlashPosition);
            m_exeName = exeName;
        }

    };
    ~Process();

    //void Start(char* arguments = nullptr);
    void Start(const Settings& settings, const char* arguments = nullptr);
    void StartWithCheck(const Settings& settings, const char* arguments = nullptr);
    void End(uint32 exitCode = 0);

private:
    void CloseHandles(const _PROCESS_INFORMATION& pi);

    uint32 m_processID = 0;
    std::string m_fileLocation;
    std::string m_exeName;
};

struct ProcessInfo {
    std::string name;
    uint32 id;
};

enum ConsoleColor : int32 {
    ConsoleColor_Black,
    ConsoleColor_Red,
    ConsoleColor_Green,
    ConsoleColor_Yellow,
    ConsoleColor_Blue,
    ConsoleColor_Purple,
    ConsoleColor_Cyan,
    ConsoleColor_White,
    ConsoleColor_Count,
};

void DebugPrint(const char* fmt, ...);
void ConsoleOutput(const std::string& text, ConsoleColor color = ConsoleColor_White);
std::string ToString(const char* fmt, ...);
void SetThreadName(std::thread::native_handle_type threadID, std::string name);
bool CreateFolder(const std::string& folderLocation);
int32 CreateMessageWindow(const char* message, const char* title, uint32 type);
void CreateErrorWindow(const char* message);
void GetFullProcessList(std::vector<ProcessInfo>& output);
bool GetExistingProcessInformation(const std::string& executableName, uint32& processID);
void Sleep(float seconds);
