#ifndef TERM_WIFI_NETDEV_H
#define TERM_WIFI_NETDEV_H

#include "common.h"

TNetDev *NetDevCreate(const char *Name, int Flags);
TNetDev *NetDevClone(TNetDev *Parent);
void NetDevDestroy(void *Item);
int NetDevLoadInterfaces(ListNode *Interfaces);
TNetDev *NetDevSelectInterface(ListNode *Interfaces, const char *Interface);

#endif
