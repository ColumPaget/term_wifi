#ifndef WMAN_WPA_SUPPLICANT_H
#define WMAN_WPA_SUPPLICANT_H

#include "net.h"
#include "common.h"


int WPASupplicantGetNetworks(TNetDev *Dev, ListNode *Networks);
int WPASupplicantActivate(TNetDev *Dev, TNet *Net);

#endif
