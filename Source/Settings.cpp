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

struct ConfigVersion {
    int32 major;
    int32 minor;
};

ConfigVersion g_configVerion = { 0, 1 };


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
    Parse_Information,
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

        size_t informationHeaderRowNumber = std::string::npos;
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

                    if (TextDetection(textLine, "INFORMATION"))
                    {
                        parsingState = Parse_Information;
                        informationHeaderRowNumber = k;
                    }
                    else if (TextDetection(textLine, "SETTINGS"))
                    {
                        parsingState = Parse_Settings;
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
                    if (parsingState == Parse_Information)
                    {
                        if (textLine == "")
                            break;

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
                        
                        if (key.find("Version") != std::string::npos)
                        {
                            SplitText versionNumber = TextSplit(value, ".");
                            g_configVerion.major = atoi(versionNumber.before.c_str());
                            g_configVerion.minor = atoi(versionNumber.after.c_str());
                        }
                    }
                    else if (parsingState == Parse_Settings)
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
                        for (int32 i = (setting_Invalid + 1); i < setting_Count; i++)
                        {
                            IndividualSetting& is = g_settings[i];
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
                                    is.v_string = ConfigFileGetValueExe(value);
                                    break;
                                }
                                }
                            }
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

        if (informationHeaderRowNumber == std::string::npos)
        {
            //version 0.0 needs to be converted to version 0.1
            auto settingsLocation = configFileText.find("$SETTINGS$");
            if (settingsLocation == std::string::npos)
                CreateErrorWindow("Could not find $SETTINGS$ in config file");

            //Add INFORMATION section and version number
            configFileText.insert(settingsLocation, "$INFORMATION$\nVersion = 0.1\n\n");

            //Find and add new settings
            auto lastSettingLocation = configFileText.find("AfterburnerLocation");
            if (lastSettingLocation == std::string::npos)
                CreateErrorWindow("Could not find AfterburnerLocation in config file");
            auto lastSettingLocationEndLine = configFileText.find_first_of("\n", lastSettingLocation);
            configFileText.insert(lastSettingLocationEndLine + 1, "StartProcessesMinimized = true #This will start the miner process(es) minimized\n");

            //Write array to disk
            File configFile(configFileName, File::Mode::Write, false);
            configFile.Write(configFileText);
            reloadConfigFile = true;
        }

    } while (reloadConfigFile);

    ConsoleOutput("MinerManager settings updated", ConsoleColor_White);
}
