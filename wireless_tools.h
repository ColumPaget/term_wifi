#ifndef WIRELESS_TOOLS_H
#define WIRELESS_TOOLS_H

#include "common.h"
#include "net.h"

int WirelessToolsGetNetworks(TNetDev *Dev, ListNode *Networks);
int WirelessToolsSetupInterface(TNetDev *Dev, TNet *Conf);

int WirelessToolsGetStatus(TNetDev *Device, TNet *Net);


#endif
