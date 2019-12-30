#ifndef RUN_COMMAND_H
#define RUN_COMMAND_H

#include "common.h"

#define RUNCMD_ROOT 1
#define RUNCMD_DAEMON 2
#define RUNCMD_NOSHELL 4

void CommandsInit();
int CommandFound(const char *Cmd);
char *RunCommand(char *Output, const char *Command, int Flags);

#endif
