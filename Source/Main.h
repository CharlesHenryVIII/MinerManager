#pragma once
#include "Types.h"
#include <atomic>

extern std::atomic<bool> g_gameDetected;

int32 ThreadMain(void* data);
