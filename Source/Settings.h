#pragma once
#include "Types.h"

#include <string>
#include <vector>

enum SettingDataType {
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

struct IndividualSetting {
    std::string name;
    SettingDataType dataType;
    float       v_float;
    bool        v_bool;
    std::string v_string;
};

extern IndividualSetting g_settings[];

bool GetUpdatedFileInfo(std::vector<std::string>& fileTextArray, std::string& fileText, uint64& lastModifiedTime);
void UpdateSettingsAndTextLists(uint64& lastTimeSettingsWereModified, std::vector<std::string>& s_inclusiveText, std::vector<std::string>& s_exclusiveText);
