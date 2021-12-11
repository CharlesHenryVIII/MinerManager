#pragma once
#include "Types.h"

#include <string>
#include <vector>

#define SETTINGS_IMPLIMENTATION 3

enum GetSettingValueType {
    SettingValueType_Invalid,
    SettingValueType_Float,
    SettingValueType_Bool,
    SettingValueType_Exe,
    SettingValueType_Count,
};

enum SettingOptions {
    setting_Invalid,
    setting_UpdateRate,
    setting_ExecutableName,
    setting_AfterburnerLocation,
    setting_StartProcessesMinimized,
    setting_Count,
};
#if SETTINGS_IMPLIMENTATION == 3

struct IndividualSetting {
    std::string name;
    GetSettingValueType dataType;
    float       v_float;
    bool        v_bool;
    std::string v_string;
};

extern IndividualSetting g_settings[];
//{
//    { "",                           SettingValueType_Invalid,{}},
//    { "UpdateRate",                 SettingValueType_Float, {} },
//    { "ExecutableName",             SettingValueType_Exe,   {} },
//    { "AfterburnerLocation",        SettingValueType_Exe,   {} },
//    { "StartProcessesMinimized",    SettingValueType_Bool,  {} },
//};

//g_settings[setting_UpdateRate].v_string

#elif SETTINGS_IMPLIMENTATION == 2
#define SETTINGS_STRING_MAX 255

struct IndividualSetting {
    std::string name;
    GetSettingValueType dataType;
    union {
        float v_float;
        bool  v_bool;
        char  v_string[SETTINGS_STRING_MAX];
    };
};

IndividualSetting g_settings[] = {
    { "UpdateRate",                 SettingValueType_Float, {} },
    { "ExecutableName",             SettingValueType_Exe,   {} },
    { "AfterburnerLocation",        SettingValueType_Exe,   {} },
    { "StartProcessesMinimized",    SettingValueType_Bool,  {} },
};

//g_settings[setting_UpdateRate].v_string

#elif SETTINGS_IMPLIMENTATION == 1

struct IndividualSetting {
    std::string name;
    GetSettingValueType dataType;
    float       valueFloat  = {};
    bool        valueBool   = {};
    std::string valueString = {};
};

IndividualSetting g_settings[] ={{ "UpdateRate",                 SettingValueType_Float, {}, {}, {} },
                                 { "ExecutableName",             SettingValueType_Exe,   {}, {}, {} },
                                 { "AfterburnerLocation",        SettingValueType_Exe,   {}, {}, {} },
                                 { "StartProcessesMinimized",    SettingValueType_Bool,  {}, {}, {} } };

#elif SETTINGS_IMPLIMENTATION == 0
struct IndividualSetting {
    std::string name;
    GetSettingValueType dataType;
    float       valueFloat  = {};
    bool        valueBool   = {};
    std::string valueString = {};
};

struct Settings {
#if 1
    IndividualSetting UpdateRate                = { "UpdateRate",                 SettingValueType_Float, {} };
    IndividualSetting executableName            = { "ExecutableName",             SettingValueType_Exe,   {} };
    IndividualSetting afterburnerLocation       = { "AfterburnerLocation",        SettingValueType_Exe,   {} };
    IndividualSetting startProcessesMinimized   = { "StartProcessesMinimized",    SettingValueType_Bool,  {} };
#else
    float UpdateRate;
    std::string executableName;
    std::string afterburnerLocation;
    bool startProcessesMinimized;
#endif
};

Settings g_settings = {};
#else
#endif

    //g_settings.UpdateRate.value;
    //g_settings.UpdateRate.name;
    //g_settings.UpdateRate.type;


bool GetUpdatedFileInfo(std::vector<std::string>& fileText, uint64& lastModifiedTime);
void UpdateSettingsAndTextLists(uint64& lastTimeSettingsWereModified, std::vector<std::string>& s_inclusiveText, std::vector<std::string>& s_exclusiveText);
