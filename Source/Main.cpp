#include "WinInterop.h"
#include "DefaultConfig.h"
#include "Utility.h"

const char* configFileName = "MinerManager.config";

struct Settings {
    float UpdateRate;
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

int main()
{
    Settings programSettings = {};
    std::vector<std::string> inclusiveText;
    std::vector<std::string> exclusiveText;
    
    {
        std::vector<std::string> configFileText;
        ParseState parsingState = Parse_None;
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
                return 1;
            }

            configFile.GetText();
            assert(configFile.m_handleIsValid);
            if (!configFile.m_textIsValid)
            {
                CreateErrorWindow("Could not get \"MinerManager.config\" file text");
                return 1;
            }
            configFileText = TextToStringArray(configFile.m_dataString.c_str(), "\n");
        }
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
    std::vector<Process> processList = GetFullProcessList();

    bool foundGame = false;
    for (const Process& process : processList)
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
        //check if miner is runnning and start if not
    }

    return 0;
}