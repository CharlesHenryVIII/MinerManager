#include "ProcessSwitching.h"
#include "WinInterop.h"
#include "Main.h"

std::atomic<uint32> g_messageFromMainToProcess  = 0;
std::atomic<bool>   g_appRunning                = true;
std::atomic<bool>   g_updating                  = true;
bool                g_currentlyMining           = false;

Process* miner;
Process* afterburner;

void ProcessSwitchingInit(const std::string& minerFileLocation, const std::string& afterburnerFileLocation)
{
    miner = new Process(minerFileLocation);
    afterburner = new Process(afterburnerFileLocation);
}

void ProcessSwitchingEndMiner(const Settings& settings)
{
    miner->End();

    afterburner->Start(settings, "-argumentList \"-profile1\"");

    ConsoleOutput("Game found, miner ended", ConsoleColor_Cyan);
    g_currentlyMining = false;
}

void ProcessSwitchingStartMiner(const Settings& settings)
{
    //check if miner is runnning and start if not
    miner->StartWithCheck(settings);

    afterburner->Start(settings, "-argumentList \"-profile2\"");

    ConsoleOutput("Miner started", ConsoleColor_Green);
    g_currentlyMining = true;
}
