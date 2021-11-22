#pragma once

#include <string>
#include <atomic>

extern std::atomic<bool> g_appRunning;
extern std::atomic<bool> g_updating;
extern bool g_currentlyMining;

void ProcessSwitchingInit(const std::string& minerFileLocation, const std::string& afterburnerFileLocation);
void ProcessSwitchingEndMiner();
void ProcessSwitchingStartMiner();

