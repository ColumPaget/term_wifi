#ifndef WMAN_WIFI_H
#define WMAN_WIFI_H

#include "common.h"
#include "net.h"

const char *WifiQualityColor(TNet *Net);

void WifiListNetworks(TNetDev *Dev);
int WifiSetup(TNetDev *Dev, TNet *Conf);

ListNode *WifiGetNetworks(TNetDev *Dev);
char *WifiFormatNet(char *Output, TNet *Net);

void WifiGetStatus(TNetDev *Device, TNet *Net);


#endif
