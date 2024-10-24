#ifndef TERM_WIFI_CMD_LINE_H
#define TERM_WIFI_CMD_LINE_H

#include "common.h"


#define ACT_INTERACTIVE 0
#define ACT_ADD    1
#define ACT_JOIN   2
#define ACT_LEAVE  3
#define ACT_LIST   4
#define ACT_SCAN   5
#define ACT_FORGET 6
#define ACT_IFACE_LIST 7
#define ACT_QRCODE     8
#define ACT_STATUS 9
#define ACT_VERSION 98
#define ACT_HELP    99



int ParseCommandLine(int argc, char *argv[], TNet *Conf);

#endif
