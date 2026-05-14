#ifndef SERVERADMIN_COMMAND_CORE_H
#define SERVERADMIN_COMMAND_CORE_H

#include "cbase.h"

#ifdef BDSBASE

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

#endif

#endif // SERVERADMIN_COMMAND_CORE_H