#ifndef IW_H
#define IW_H

#include "common.h"
#include "net.h"

void IWGetNetworks(TNetDev *Dev, ListNode *Networks);
int IWSetupInterface(TNetDev *Dev, TNet *Conf);

void IWGetStatus(TNetDev *Device, TNet *Net);

#endif
