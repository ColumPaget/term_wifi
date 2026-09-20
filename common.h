
#ifndef WMAN_COMMON_H
#define WMAN_COMMON_H

#ifdef HAVE_LIBUSEFUL5_LIBUSEFUL_H
#include "libUseful-5/libUseful.h"
#else
#include "libUseful-bundled/libUseful.h"
#endif


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
int Flags;
int Channel;
char *ESSID;
char *Title;
char *Interface;
char *MacAddress;
char *Address;
char *Netmask;
char *Gateway;
char *DNSServer;
char *AccessPoint;
char *CountryCode;
char *BitRates;
char *UserID;
char *Key;
char *Path;
char *DateAdded;
float Quality;
float dBm;
} TNet;


extern ListNode *ConfiguredNets;
extern ListNode *Interfaces;
extern STREAM *StdIO;

typedef void (*INTERACTIVE_STATUS_CALLBACK)(TNetDev *Dev, const char *Text);

extern INTERACTIVE_STATUS_CALLBACK DisplayStatus;
void PidPathKill(const char *Path);
void PidFileKill(const char *AppName);
TNet *NetCreate();
void NetDestroy(void *p_Net);

const char *OutputNetQualityColor(TNet *Net);
char *OutputFormatNet(char *Output, TNet *Net);

int FrequencyToChannel(int freq);
void QueryRootPassword(const char *Prompt);

void NetSetESSID(TNet *Net, const char *ESSID);

char *FindCommandFromList(char *RetStr, const char *List);
#endif
