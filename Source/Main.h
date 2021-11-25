#include "Types.h"

int32 ThreadMain(void* data);

struct Settings {
    float UpdateRate;
    std::string executableName;
    std::string afterburnerLocation;
    bool startProcessesMinimized;
};
