#include "WinInterop.h"
#include "DefaultConfig.h"
#include "Utility.h"

const char* configFileName = "MinerManager.config";

struct Settings {
    float UpdateRate;
    std::string executableName;
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
    if (removeSpaces)
        TextRemoval(input, " ");
}

void GetUpdatedFileInfo(std::vector<std::string>& fileText, uint64& lastModifiedTime)
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
        printf("Couldn't get config file handle\n");
#else
        CreateErrorWindow("Could not get file handle to \"MinerManager.config\"");
#endif
        return;
    }

    configFile.GetTime();
    if (!configFile.m_timeIsValid)
    {
        CreateErrorWindow("Could not get \"MinerManager.config\" file time/date");
    }
    if (configFile.m_time <= lastModifiedTime)
    {
        printf("No config file update\n");
        return;
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

    return;
}

int main()
{
    Settings programSettings = {};
    uint64 lastTimeSettingsWereModified = {};

    std::vector<std::string> inclusiveText;
    std::vector<std::string> exclusiveText;
    
    {
        std::vector<std::string> configFileText;
        GetUpdatedFileInfo(configFileText, lastTimeSettingsWereModified);

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

                    break;
                }
                else if (character == ' ')
                {
                    //word ended
                    continue;
                }
                else
                {
                    switch (parsingState)
                    {
                    case Parse_Settings:
                    {
                        CleanString(textLine, true);
                        //Organize string
                        SplitText keyValue = TextSplit(textLine.c_str(), "=");
                        if (keyValue.before == "" || keyValue.after == "")
                        {
                            assert(false);
                            CreateErrorWindow(ToString("unknown string array format: %s", textLine.c_str()).c_str());
                            return 1;
                        }
                        const std::string& key = keyValue.before;
                        const std::string& value = keyValue.after;

                        if (key.find("UpdateRate") != std::string::npos)
                        {
                            float valueTranslated = std::stof(value);
                            programSettings.UpdateRate = valueTranslated;
                        }
                        else if (key.find("ExecutableName") != std::string::npos)
                        {
                            programSettings.executableName = value;
                        }

                        break;
                    }
                    case Parse_Inclusive:
                    {
                        CleanString(textLine, false);
                        if (textLine != "")
                        {
                            inclusiveText.push_back(textLine);
                            c = (int32)textLine.size() - 1;
                        }
                        break;
                    }
                    case Parse_Exclusive:
                    {
                        CleanString(textLine, false);
                        if (textLine != "")
                        {
                            exclusiveText.push_back(textLine);
                            c = (int32)textLine.size() - 1;
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }

    bool running = true;
    Process minerProcess();
    std::vector<ProcessInfo> processList;
    processList.reserve(500);
    while (running)
    {
        GetFullProcessList(processList);

        bool foundGame = false;
        for (const ProcessInfo& process : processList)
        {
            for (const std::string& t : exclusiveText)
            {
                if (t == process.name)
                {
                    foundGame = true;
                    break;
                }
            }
            if (!foundGame)
            {
                for (const std::string& t : inclusiveText)
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
        if (foundGame)
        {
            //kill miner
        }
        else
        {
            StartProcess();

            //check if miner is runnning and start if not
        }

        Sleep(programSettings.UpdateRate);
    }

    return 0;
}