
#ifndef WMAN_COMMON_H
#define WMAN_COMMON_H

#include "libUseful-4/libUseful.h"
#include <glob.h>

#define NET_WPA1 1
#define NET_WPA2 2
#define NET_RSN  4
#define NET_WEP 32
#define NET_ADHOC 128
#define NET_STORE 2048
#define NET_ASSOCIATED 4096
#define NET_JOINING 8192

#define NET_ENCRYPTED (NET_WPA1 | NET_WPA2 | NET_RSN | NET_WEP)


#define DEV_WIFI 1

typedef struct
{
char *Name;
int Flags;
char *Driver;
} TNetDev;


typedef struct
{
char *Interface;
char *MacAddress;
char *Address;
char *Netmask;
char *Gateway;
char *DNSServer;
char *ESSID;
char *AccessPoint;
char *CountryCode;
int Flags;
int Channel;
char *BitRates;
char *UserID;
char *Key;
float Quality;
float dBm;
} TNet;


extern ListNode *ConfiguredNets;
extern ListNode *Interfaces;
extern STREAM *StdIO;

typedef void (*INTERACTIVE_STATUS_CALLBACK)(TNetDev *Dev, const char *Text);

extern INTERACTIVE_STATUS_CALLBACK DisplayStatus;
void PidFileKill(const char *AppName);
TNet *NetCreate();
void NetDestroy(void *p_Net);

const char *OutputNetQualityColor(TNet *Net);
char *OutputFormatNet(char *Output, TNet *Net);

int FrequencyToChannel(int freq);
void QueryRootPassword(const char *Prompt);

#endif
