#include "Main.h" //maybe dont do this?????
#include "WinInterop.h"
#include "DefaultConfig.h"
#include "Utility.h"
#include "ProcessSwitching.h"

const char* configFileName = "MinerManager.config";

struct Settings {
    float UpdateRate;
    std::string executableName;
    std::string afterburnerLocation;
};

enum ParseState {
    Parse_None,
    Parse_Settings,
    Parse_Inclusive,
    Parse_Exclusive,
    Parse_Count,
};

void CleanString(std::string& input, bool removeSpaces)
{
    if (int32 hashLocation = TextDetection(input, "#"))
    {
        input.erase(hashLocation);
    }
    while (input.find_last_of(' ') == input.size() - 1 || input.find_last_of('\n') == input.size() - 1)
        input.erase(input.size() - 1, 1);
    if (removeSpaces)
        TextRemoval(input, " ");
}

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

void UpdateSettingsAndTextLists(uint64& lastTimeSettingsWereModified, Settings& s_programSettings, std::vector<std::string>& s_inclusiveText, std::vector<std::string>& s_exclusiveText)
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
                //word ended
                continue;
            }
            else
            {
                if (parsingState == Parse_Settings)
                {
                    const std::string original = textLine;
                    CleanString(textLine, true);
                    //Organize string
                    SplitText keyValue = TextSplit(textLine.c_str(), "=");
                    if (keyValue.before == "" || keyValue.after == "")
                    {
                        assert(false);
                        CreateErrorWindow(ToString("unknown string array format: %s", textLine.c_str()).c_str());
                        //return 1;
                    }
                    const std::string& key = keyValue.before;
                    const std::string& value = keyValue.after;

                    if (key.find("UpdateRate") != std::string::npos)
                    {
                        float valueTranslated = std::stof(value);
                        s_programSettings.UpdateRate = valueTranslated;
                        break;
                    }
                    else if (key.find("ExecutableName") != std::string::npos)
                    {
                        s_programSettings.executableName = value;
                        break;
                    }
                    else if (key.find("AfterburnerLocation") != std::string::npos)
                    {
                        SplitText split = TextSplit(original.c_str(), "=");
                        while (split.after[0] == ' ')
                            split.after.erase(0, 1);
                        while (split.after[split.after.size() - 1] == '\n' || split.after[split.after.size() - 1] == ' ')
                            split.after.erase(split.after.size() - 1, 1);
                        s_programSettings.afterburnerLocation = split.after;
                        break;
                    }
                }
                if (parsingState == Parse_Inclusive)
                {
                    CleanString(textLine, false);
                    if (textLine != "")
                    {
                        s_inclusiveText.push_back(textLine);
                        c = (int32)textLine.size() - 1;
                    }
                    break;
                }
                if (parsingState == Parse_Exclusive)
                {
                    CleanString(textLine, false);
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

int32 ThreadMain(void* data)
{
    ConsoleOutput("MinerManager started", ConsoleColor_Green);
    Settings s_programSettings = {};
    uint64 s_lastTimeConfigWasModified = {};

    std::vector<std::string> s_inclusiveText;
    std::vector<std::string> s_exclusiveText;

    UpdateSettingsAndTextLists(s_lastTimeConfigWasModified, s_programSettings, s_inclusiveText, s_exclusiveText);

    ProcessSwitchingInit(s_programSettings.executableName, s_programSettings.afterburnerLocation);

    std::vector<ProcessInfo> s_processList;
    s_processList.reserve(500);
    bool foundGame = false;

    while (g_appRunning)
    {
        if (g_updating)
        {
            UpdateSettingsAndTextLists(s_lastTimeConfigWasModified, s_programSettings, s_inclusiveText, s_exclusiveText);
            GetFullProcessList(s_processList);

            //for (const ProcessInfo& process : s_processList)
            foundGame = false;
            for (int32 i = 0; i < s_processList.size(); i++)
            {
                const ProcessInfo& process = s_processList[i];
                for (const std::string& t : s_exclusiveText)
                {
                    if (t == process.name)
                    {
                        foundGame = true;
                        break;
                    }
                }
                if (!foundGame)
                {
                    for (const std::string& t : s_inclusiveText)
                    {
                        if (FindStringCaseInsensitive(process.name, t))
                        {
                            foundGame = true;
                            break;
                        }
                    }
                }
                if (foundGame)
                    break;
            }
            if (foundGame == g_currentlyMining)
            {
                if (foundGame)
                {
                    ProcessSwitchingEndMiner();
                }
                else
                {
                    ProcessSwitchingStartMiner();
                }
            }
            Sleep(s_programSettings.UpdateRate);
        }

        Sleep(5);
    }

    ProcessSwitchingEndMiner();
    return 0;
}