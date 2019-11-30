#ifndef WMAN_NET_H
#define WMAN_NET_H

#include "common.h"


void NetGetStatus(TNetDev *Dev, TNet *Net);
void NetGetInterfaces(ListNode *Interfaces);
void NetSetupInterface(TNetDev *Interface, const char *Address, const char *Netmask, const char *Gateway, const char *DNSServer);
void NetDown(TNetDev *Dev);

#endif
