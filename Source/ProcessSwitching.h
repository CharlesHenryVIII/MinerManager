#pragma once
#include "Utility.h"

#include <string>
#include <atomic>

#define THREAD_MESSAGE_START    BIT(0)
#define THREAD_MESSAGE_STOP     BIT(1)
#define THREAD_MESSAGE_EXIT     BIT(2)

extern std::atomic<uint32>  g_messageFromMainToProcess;
extern std::atomic<bool>    g_appRunning;
extern std::atomic<bool>    g_updating;
extern bool                 g_currentlyMining;

struct Settings;
void ProcessSwitchingInit(const std::string& minerFileLocation, const std::string& afterburnerFileLocation);
void ProcessSwitchingEndMiner();
void ProcessSwitchingStartMiner();

