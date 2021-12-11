#include "Settings.h"
#include "Types.h"
#include "WinInterop.h"
#include "DefaultConfig.h"

#include <vector>
#include <string>

const char* configFileName = "MinerManager.config";
IndividualSetting g_settings[] =
{
    { "",                           SettingValueType_Invalid,{}, {}, {} },
    { "UpdateRate",                 SettingValueType_Float,  {}, {}, {} },
    { "ExecutableName",             SettingValueType_Exe,    {}, {}, {} },
    { "AfterburnerLocation",        SettingValueType_Exe,    {}, {}, {} },
    { "StartProcessesMinimized",    SettingValueType_Bool,   {}, {}, {} },
};

struct DefaultSetting {
    //SettingDataType dataType;
    std::string value;
    std::string comments;
};
static DefaultSetting s_defaultSettings[] =
{
    { "",                                                               "" },
    { "5.0",                                                            "Seconds" },
    { "PhoenixMiner.exe",                                               "Either the full path or the local path to the miner executable" },
    { "C:\\Program Files (x86)\\MSI Afterburner\\MSIAfterburner.exe",   "" },
    { "true",                                                           "This will start the miner process(es) minimized" },
};

bool GetUpdatedFileInfo(std::vector<std::string>& fileTextArray, std::string& fileText, uint64& lastModifiedTime)
{
    //Check for file not there, create it if its not, and fill it with the default config file
    //Config File Scope

    {
        File configFile(configFileName, File::Mode::Read, false);
        if (!configFile.m_handleIsValid)
        {
            File newConfigFile(configFileName, File::Mode::Write, true);
            newConfigFile.Write(g_defaultConfigText.c_str(), g_defaultConfigText.size());
        }
    }
    File configFile(configFileName, File::Mode::Read, false);

    assert(configFile.m_handleIsValid);
    if (!configFile.m_handleIsValid)
    {
        CreateErrorWindow("Could not get file handle to \"MinerManager.config\"");
        return false;
    }

    configFile.GetTime();
    if (!configFile.m_timeIsValid)
    {
        CreateErrorWindow("Could not get \"MinerManager.config\" file time/date");
    }
    if (configFile.m_time <= lastModifiedTime)
    {
        //DebugPrint("No config file update\n");
        return false;
    }

    lastModifiedTime = configFile.m_time;

    configFile.GetText();
    assert(configFile.m_handleIsValid);
    if (!configFile.m_textIsValid)
    {
        CreateErrorWindow("Could not get \"MinerManager.config\" file text");
    }
    fileText = configFile.m_dataString;
    fileTextArray.clear();
    fileTextArray = TextToStringArray(configFile.m_dataString.c_str(), "\n");

    return true;
}

float ConfigFileGetValueFloat(const std::string& input)
{
    float result = std::stof(input);
    return result;
}
bool ConfigFileGetValueBool(const std::string& input)
{
    bool result = FindStringCaseInsensitive(input, "true") || FindStringCaseInsensitive(input, "1");
    return result;
}
std::string ConfigFileGetValueExe(const std::string& input)
{
    // TODO(choman): Properly handle exe switches?
    std::string result = input;
    while (result[0] == ' ')
        result.erase(0, 1);
    while ((result[result.size() - 1] < 'A') ||
           (result[result.size() - 1] > 'z') ||
          ((result[result.size() - 1] > 'Z') && (result[result.size() - 1] < 'a')))
        result.erase(result.size() - 1, 1);
    return result;
}

enum ParseState {
    Parse_None,
    Parse_Settings,
    Parse_Inclusive,
    Parse_Exclusive,
    Parse_Count,
};

void CleanString(std::string& input)
{
    //Remove # comments
    if (input == "")
        return;

    const size_t hashLocation = input.find_first_of('#');
    if (hashLocation != std::string::npos)
        input.erase(hashLocation);

    //Trim string
    while (input.find_last_of(' ') == input.size() - 1 || input.find_last_of('\n') == input.size() - 1)
        input.erase(input.size() - 1, 1);
}

void UpdateSettingsAndTextLists(uint64& lastTimeSettingsWereModified, std::vector<std::string>& s_inclusiveText, std::vector<std::string>& s_exclusiveText)
{
#ifdef _DEBUG
    assert(arrsize(g_settings)        == setting_Count);
    assert(arrsize(s_defaultSettings) == setting_Count);
#endif

    bool reloadConfigFile = false;
    do {
        reloadConfigFile = false;

        std::string configFileText;
        std::vector<std::string> configFileTextArray;
        if (!GetUpdatedFileInfo(configFileTextArray, configFileText, lastTimeSettingsWereModified))
            return;

        size_t settingHeaderRowNumber = std::string::npos;
        bool settingsFound[setting_Count] = {};
        size_t settingsRowNumbers[setting_Count] = {};
        memset(settingsRowNumbers, uint32(-1), sizeof(settingsRowNumbers));

        ParseState parsingState = Parse_None;
        std::vector<std::string> settingsText;
        for (size_t k = 0; k < configFileTextArray.size(); k++)
        {
            const std::string& textLine = configFileTextArray[k];
            for (size_t j = 0; j < textLine.size(); j++)
            {
                const char& character = textLine[j];
                if (character == '#')
                {
                    break;
                }
                else if (character == '$')
                {
                    if (TextDetection(textLine, "SETTINGS"))
                    {
                        parsingState = Parse_Settings;
                        settingHeaderRowNumber = k;
                    }
                    else if (TextDetection(textLine, "INCLUSIVE"))
                    {
                        parsingState = Parse_Inclusive;
                    }
                    else if (TextDetection(textLine, "EXCLUSIVE"))
                    {
                        parsingState = Parse_Exclusive;
                    }
                    else
                    {
                        std::string text = ToString("Unknown section title in config file: %s", textLine.c_str());
                        DebugPrint(text.c_str());
                        ConsoleOutput(text.c_str(), ConsoleColor_White);
                        break;
                    }

                    break;
                }
                else if (character == ' ')
                {
                    continue;
                }
                else
                {
                    if (parsingState == Parse_Settings)
                    {
                        std::string cleanedTextLine = textLine;
                        CleanString(cleanedTextLine);

                        //Split string into string setting name and string value
                        SplitText keyValue = TextSplit(cleanedTextLine.c_str(), "=");
                        if (keyValue.before == "" || keyValue.after == "")
                        {
                            assert(false);
                            CreateErrorWindow(ToString("unknown string array format: %s", cleanedTextLine.c_str()).c_str());
                        }
                        //very clean (remove spaces around equal sign
                        CleanString(keyValue.before);
                        CleanString(keyValue.after);
                        const std::string& key = keyValue.before;
                        const std::string& value = keyValue.after;

                        //Look for setting name
                        bool foundSetting = false;
                        for (int32 i = (setting_Invalid + 1); i < setting_Count; i++)
                        {
                            IndividualSetting& is = g_settings[i];
                            if (key.find(is.name) != std::string::npos)
                            {
                                if (settingsFound[i] == true)
                                {
                                    CreateErrorWindow(ToString("Duplicate setting '%s' found in config file: %s", is.name.c_str(), key.c_str()).c_str());
                                }

                                switch (is.dataType)
                                {
                                case SettingValueType_Float:
                                {
                                    is.v_float = ConfigFileGetValueFloat(value);
                                    break;
                                }
                                case SettingValueType_Bool:
                                {
                                    is.v_bool = ConfigFileGetValueBool(value);
                                    break;
                                }
                                case SettingValueType_Exe:
                                {
#if SETTINGS_IMPLIMENTATION == 3
                                    is.v_string = ConfigFileGetValueExe(value);
#elif SETTINGS_IMPLIMENTATION == 2
                                    std::string result = ConfigFileGetValueExe(value);
                                    if (result.size() < SETTINGS_STRING_MAX)
                                    {
                                        memcpy(is.v_string, result.data(), result.size());
                                    }
                                    else
                                    {
                                        CreateErrorWindow(ToString("Setting value too long (must be under 255 characters): %s", result.c_str()).c_str());
                                    }
#endif
                                    break;
                                }
                                }
                                settingsFound[i] = true;
                                foundSetting = true;
                            }
                        }
                        if (foundSetting = false)
                        {
                            CreateErrorWindow(ToString("Unknown setting %s", key).c_str());
                        }
                        break;
                    }
                    else if (parsingState == Parse_Inclusive)
                    {
                        std::string cleanedTextLine = textLine;
                        CleanString(cleanedTextLine);
                        if (cleanedTextLine != "")
                        {
                            s_inclusiveText.push_back(cleanedTextLine);
                            j = (int32)cleanedTextLine.size() - 1;
                        }
                        break;
                    }
                    else if (parsingState == Parse_Exclusive)
                    {
                        std::string cleanedTextLine = textLine;
                        CleanString(cleanedTextLine);
                        if (cleanedTextLine != "")
                        {
                            s_exclusiveText.push_back(cleanedTextLine);
                            j = (int32)cleanedTextLine.size() - 1;
                        }
                        break;
                    }
                }
            }
        }

        bool needToUpdateConfigFile = false;
        for (int32 i = (setting_Invalid + 1); i < setting_Count; i++)
        {

            if (settingHeaderRowNumber == std::string::npos)
            {
                CreateErrorWindow("Settings header '$SETTINGS$' not found in configuration file");
            }
            if (!settingsFound[i])
            {
                needToUpdateConfigFile = true;
                if (settingHeaderRowNumber == configFileTextArray.size() - 1)
                    configFileTextArray.push_back("\n");

                std::string newLine = ToString("%s = %s", g_settings[i].name.c_str(), s_defaultSettings[i].value.c_str());
                if (s_defaultSettings[i].comments != "")//add comments
                    newLine = newLine + ToString(" #%s", s_defaultSettings[i].comments.c_str());
                newLine = newLine + "\n";

                configFileTextArray.insert(configFileTextArray.begin() + settingHeaderRowNumber + i, newLine);
            }
        }
        if (needToUpdateConfigFile)
        {
            //Write array to disk
            File configFile(configFileName, File::Mode::Write, false);
            for (int32 i = 0; i < configFileTextArray.size(); i++)
            {
                configFile.Write(configFileTextArray[i]);
            }
            reloadConfigFile = true;
        }

    } while (reloadConfigFile);

    ConsoleOutput("MinerManager settings updated", ConsoleColor_White);
}
