#include "wifi.h"
#include "wireless_tools.h"
#include "iw.h"
#include "wpa_supplicant.h"
#include "settings.h"


static TNet *MatchConfiguredNet(TNet *Net, ListNode *Networks, int MatchAP)
{
ListNode *Curr;
TNet *Found;

Curr=ListGetNext(Networks);
while (Curr)
{
	Found=(TNet *) Curr->Item;
	if (
			(MatchAP && (strcasecmp(Found->AccessPoint, Net->AccessPoint)==0) ) ||
			((! MatchAP) && (strcasecmp(Found->ESSID, Net->ESSID)==0) )
		 )
	{
	return(Found);
	}
	Curr=ListGetNext(Curr);
}

return(NULL);
}


static TNet *FindConfiguredNet(ListNode *Configs, ListNode *Networks, int MatchAP)
{
TNet *Net, *Found;
ListNode *Curr;

Curr=ListGetNext(Configs);
while (Curr)
{
	Net=(TNet *) Curr->Item;
	Found=MatchConfiguredNet(Net, Networks, MatchAP);
 	if (Found)
	{
	//the one piece of information that's not going to be present in a freshly configured net
	//is what encryption standard it uses. Hence we must set flags for the matching network
	//in order that the configured net has the same encryption as the actual scanned net

		Net->Flags |= Found->Flags;
		return(Net);
	}
	Curr=ListGetNext(Curr);
}
return(NULL);
}


int WifiScanNetworks(TNetDev *Dev, ListNode *Networks)
{
if (! WirelessToolsGetNetworks(Dev, Networks))
{
	IWGetNetworks(Dev, Networks);
}

return(ListSize(Networks));
}


static int WifiSortCompare(void *Data, void *p1, void *p2)
{
TNet *N1, *N2;

N1=(TNet *) p1;
N2=(TNet *) p2;

if (N2->dBm < N1->dBm) return(TRUE);
return(N2->Quality < N1->Quality);
}


ListNode *WifiGetNetworks(TNetDev *Dev)
{
ListNode *Networks;
int i;

Networks=ListCreate();

for (i=0; i < 10; i++)
{
WifiScanNetworks(Dev, Networks);
if (ListSize(Networks) > 0) break;
usleep(300000);
}

ListSort(Networks, NULL, WifiSortCompare);

return(Networks);
}




int WifiSetup(TNetDev *Dev, TNet *Conf)
{
ListNode *Networks, *Configs, *Curr;
TNet *Net;
int i, result=FALSE;

Configs=SettingsLoadNets(Conf->ESSID);
Networks=ListCreate();

NetSetupInterface(Dev, "", "", "", "");
for (i=0; i < 4; i++)
{
WifiScanNetworks(Dev, Networks);

Net=FindConfiguredNet(Configs, Networks, TRUE);
if (Net) break;
usleep(100000);
}

if (! Net) Net=FindConfiguredNet(Configs, Networks, FALSE);
if (Net)
{
	Conf->Flags |= Net->Flags;
	//update config with these items if they are not set, so if we then save the 
  if (! StrValid(Conf->Key)) Conf->Key=CopyStr(Conf->Key, Net->Key);
  if (! StrValid(Conf->Address)) Conf->Address=CopyStr(Conf->Address, Net->Address);
  if (! StrValid(Conf->Netmask)) Conf->Netmask=CopyStr(Conf->Netmask, Net->Netmask);
  if (! StrValid(Conf->Gateway)) Conf->Gateway=CopyStr(Conf->Gateway, Net->Gateway);
}

if (Conf->Flags & (NET_RSN | NET_WPA1 | NET_WPA2)) WPASupplicantActivate(Dev, Conf);
else 
{
	if (! WirelessToolsSetupInterface(Dev, Conf))
	{
		IWSetupInterface(Dev, Conf);
	}
}

ListDestroy(Networks, NetDestroy);
ListDestroy(Configs, NetDestroy);

return(result);
}





void WifiGetStatus(TNetDev *Device, TNet *Net)
{
if (! WirelessToolsGetStatus(Device, Net))
{
	//if wireless tools is not installed, then try using iw
	IWGetStatus(Device, Net);
}
}

