#ifndef RUN_COMMAND_H
#define RUN_COMMAND_H

#include "common.h"

#define RUNCMD_ROOT 1

void CommandsInit();
int CommandFound(const char *Cmd);
char *RunCommand(char *Output, const char *Command, int Flags);

#endif
