#ifndef SERVERADMIN_COMMAND_CORE_H
#define SERVERADMIN_COMMAND_CORE_H

#include "cbase.h"

typedef void (*AdminCommandFunction)(const CCommand& args);

struct CommandEntry
{
    const char* moduleName;
    const char* chatCommand;
    bool requiresArguments;
    const char* consoleMessage;
    const char* helpMessage;
    const char* requiredFlags;
    AdminCommandFunction function;
};

#endif // SERVERADMIN_COMMAND_CORE_H