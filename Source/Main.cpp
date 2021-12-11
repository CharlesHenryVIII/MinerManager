#include "Main.h" //maybe dont do this?????
#include "WinInterop.h"
#include "DefaultConfig.h"
#include "Utility.h"
#include "ProcessSwitching.h"
#include "Settings.h"

int32 ThreadMain(void* data)
{
    ConsoleOutput("MinerManager started", ConsoleColor_Green);
    uint64 s_lastTimeConfigWasModified = {};

    std::vector<std::string> s_inclusiveText;
    std::vector<std::string> s_exclusiveText;

    UpdateSettingsAndTextLists(s_lastTimeConfigWasModified, s_inclusiveText, s_exclusiveText);

    ProcessSwitchingInit(g_settings[setting_ExecutableName].v_string, g_settings[setting_AfterburnerLocation].v_string);

    std::vector<ProcessInfo> s_processList;
    s_processList.reserve(500);
    bool foundGame = false;

    while (g_appRunning)
    {
        uint32 messageFlags = g_messageFromMainToProcess.exchange(0);
        if (messageFlags & THREAD_MESSAGE_START)
        {
            if (g_updating)
                CreateErrorWindow("Trying to start process thats already started");
            g_updating = true;
        }
        if (messageFlags & THREAD_MESSAGE_STOP)
        {
            if (!g_updating)
                CreateErrorWindow("Trying to stop process thats already stopped");
            ProcessSwitchingEndMiner();
            g_updating = false;
        }
        if (messageFlags & THREAD_MESSAGE_EXIT)
        {
            g_appRunning = false;
            g_updating   = false;
            break;
        }

        if (g_updating)
        {
            UpdateSettingsAndTextLists(s_lastTimeConfigWasModified, s_inclusiveText, s_exclusiveText);
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
        }

        Sleep(g_settings[setting_UpdateRate].v_float);
    }

    ProcessSwitchingEndMiner();
    return 0;
}
