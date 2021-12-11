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

bool GetUpdatedFileInfo(std::vector<std::string>& fileText, uint64& lastModifiedTime)
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
#if _DEBUG
        DebugPrint("Couldn't get config file handle\n");
#else
        CreateErrorWindow("Could not get file handle to \"MinerManager.config\"");
#endif
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
    fileText.clear();
    fileText = TextToStringArray(configFile.m_dataString.c_str(), "\n");

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
    const size_t hashLocation = input.find_first_of('#');
    if (hashLocation != std::string::npos)
        input.erase(hashLocation);

    //Trim string
    while (input.find_last_of(' ') == input.size() - 1 || input.find_last_of('\n') == input.size() - 1)
        input.erase(input.size() - 1, 1);
}

void UpdateSettingsAndTextLists(uint64& lastTimeSettingsWereModified, std::vector<std::string>& s_inclusiveText, std::vector<std::string>& s_exclusiveText)
{
    std::vector<std::string> configFileText;
    if (!GetUpdatedFileInfo(configFileText, lastTimeSettingsWereModified))
        return;

    ParseState parsingState = Parse_None;
    std::vector<std::string> settingsText;
    for (std::string& textLine : configFileText)
    {
        for (int32 c = 0; c < textLine.size(); c++)
        {
            char& character = textLine[c];
            if (character == '#')
            {
                break;
            }
            else if (character == '$')
            {
                if (TextDetection(textLine, "SETTINGS"))
                    parsingState = Parse_Settings;
                else if (TextDetection(textLine, "INCLUSIVE"))
                    parsingState = Parse_Inclusive;
                else if (TextDetection(textLine, "EXCLUSIVE"))
                    parsingState = Parse_Exclusive;
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
                    std::string original = textLine;
                    CleanString(textLine);

                    //Split string into string setting name and string value
                    SplitText keyValue = TextSplit(textLine.c_str(), "=");
                    if (keyValue.before == "" || keyValue.after == "")
                    {
                        assert(false);
                        CreateErrorWindow(ToString("unknown string array format: %s", textLine.c_str()).c_str());
                    }
                    //very clean (remove spaces around equal sign
                    CleanString(keyValue.before);
                    CleanString(keyValue.after);
                    const std::string& key = keyValue.before;
                    const std::string& value = keyValue.after;

                    //Look for setting name
                    bool foundSetting = false;
                    for (IndividualSetting& is : g_settings)
                    {
                        if (key.find(is.name) != std::string::npos)
                        {
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
                            foundSetting = true;
                        }
                    }
                    if (foundSetting = false)
                    {
                        CreateErrorWindow(ToString("Unknown setting %s", key).c_str());
                    }
                }
                else if (parsingState == Parse_Inclusive)
                {
                    CleanString(textLine);
                    if (textLine != "")
                    {
                        s_inclusiveText.push_back(textLine);
                        c = (int32)textLine.size() - 1;
                    }
                    break;
                }
                else if (parsingState == Parse_Exclusive)
                {
                    CleanString(textLine);
                    if (textLine != "")
                    {
                        s_exclusiveText.push_back(textLine);
                        c = (int32)textLine.size() - 1;
                    }
                    break;
                }
            }
        }
    }
    ConsoleOutput("MinerManager settings updated", ConsoleColor_White);
}
