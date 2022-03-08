#include "Main.h" //maybe dont do this?????
#include "WinInterop.h"
#include "DefaultConfig.h"
#include "Utility.h"
#include "ProcessSwitching.h"
#include "Settings.h"

std::atomic<bool> g_gameDetected;

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
    g_gameDetected = false;
    bool gameDetected;

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

            gameDetected = false;
            for (int32 i = 0; i < s_processList.size(); i++)
            {
                const ProcessInfo& process = s_processList[i];
                for (const std::string& t : s_exclusiveText)
                {
                    if (FindString_ExactCaseInsensitive(process.name, t))
                    {
                        gameDetected = true;
                        break;
                    }
                }
                if (!gameDetected)
                {
                    for (const std::string& t : s_inclusiveText)
                    {
                        if (FindStringCaseInsensitive(process.name, t))
                        {
                            gameDetected = true;
                            break;
                        }
                    }
                }
                if (gameDetected)
                    break;
            }
            if (gameDetected == g_currentlyMining)
            {
                if (gameDetected)
                {
                    g_gameDetected = true;
                    ProcessSwitchingEndMiner();
                }
                else
                {
                    g_gameDetected = false;
                    ProcessSwitchingStartMiner();
                }
            }
        }

        Sleep(g_settings[setting_UpdateRate].v_float);
    }

    ProcessSwitchingEndMiner();
    return 0;
}
