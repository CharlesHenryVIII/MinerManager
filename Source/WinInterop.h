#pragma once
//#include "Math.h"
#include "Types.h"
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

struct Process {
    std::string name;
    uint64 id;
};

void DebugPrint(const char* fmt, ...);
std::string ToString(const char* fmt, ...);
void SetThreadName(std::thread::native_handle_type threadID, std::string name);
bool CreateFolder(const std::string& folderLocation);
int32 CreateMessageWindow(const char* message, const char* title, uint32 type);
void CreateErrorWindow(const char* message);
void GetFullProcessList(std::vector<Process>& output);
void StartProcess();
void Sleep(float seconds);
