#include "settings.h"


void SettingsInit()
{
memset(&Settings, 0, sizeof(Settings));

Settings.PidsDir=CopyStr(Settings.PidsDir, DEFAULT_PIDS_DIR);
Settings.ConfigFile=CopyStr(Settings.ConfigFile, DEFAULT_CONFIG_FILE);
}


void SettingsPostProcess()
{
char *Tempstr=NULL;

if (strncmp(Settings.ConfigFile, "~/", 2)==0) 
{
	Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), Settings.ConfigFile +1, NULL);
	Settings.ConfigFile=CopyStr(Settings.ConfigFile, Tempstr);
}

Destroy(Tempstr);
}



ListNode *SettingsLoadNets(const char *Match)
{
STREAM *S;
char *Tempstr=NULL, *Token=NULL;
const char *ptr;
ListNode *Nets=NULL;
TNet *Net;

Nets=ListCreate();

S=STREAMOpen(Settings.ConfigFile, "r");
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
	StripTrailingWhitespace(Tempstr);

	ptr=GetToken(Tempstr, "\\S", &Token, 0);
	if (strcasecmp(Token, "essid")==0) 
	{
		Net=NULL;

		if ( (! StrValid(Match)) || (strcasecmp(Match, ptr)==0) )
		{
		Net=(TNet *) calloc(1, sizeof(TNet));
		Net->ESSID=CopyStr(Net->ESSID, ptr);
		Net->AccessPoint=CopyStr(Net->AccessPoint, "");
		ListAddNamedItem(Nets, Net->ESSID, Net);
		}
	}
	else if (Net)
	{
		if (strcmp(Token, "wpa1")==0) Net->Flags |= NET_WPA1;
		if (strcmp(Token, "wpa1")==0) Net->Flags |= NET_WPA1;
		if (strcmp(Token, "rsn")==0) Net->Flags |= NET_RSN;
		if (strcmp(Token, "key")==0) Net->Key=CopyStr(Net->Key, ptr);
		if (strcmp(Token, "address")==0) Net->Address=CopyStr(Net->Address, ptr);
		if (strcmp(Token, "netmask")==0) Net->Netmask=CopyStr(Net->Netmask, ptr);
		if (strcmp(Token, "gateway")==0) Net->Gateway=CopyStr(Net->Gateway, ptr);
		if (strcmp(Token, "dns")==0) Net->DNSServer=CopyStr(Net->DNSServer, ptr);
		if (strcmp(Token, "accesspoint")==0) Net->AccessPoint=CopyStr(Net->AccessPoint, ptr);
		if (strcmp(Token, "channel")==0) Net->Channel=atoi(ptr);
	}

	Tempstr=STREAMReadLine(Tempstr, S);
	}
	STREAMClose(S);
}

Destroy(Tempstr);
Destroy(Token);

return(Nets);
}





void SettingsSaveNets(ListNode *List)
{
ListNode *Curr;
char *Tempstr=NULL;
TNet *Net;
STREAM *S;

S=STREAMOpen(Settings.ConfigFile, "w");
if (S)
{
	Curr=ListGetNext(List);
	while (Curr)
	{
		Net=(TNet *) Curr->Item;
		Tempstr=MCopyStr(Tempstr, "essid ", Net->ESSID, "\n", NULL);
		STREAMWriteLine(Tempstr, S);

		if (Net->Flags & NET_RSN) STREAMWriteLine("rsn\n", S);
		if (Net->Flags & NET_WPA2) STREAMWriteLine("wpa2\n", S);
		if (Net->Flags & NET_WPA1) STREAMWriteLine("wpa1\n", S);

		Tempstr=MCopyStr(Tempstr, "key ", Net->Key, "\n", NULL);
		STREAMWriteLine(Tempstr, S);
		Tempstr=MCopyStr(Tempstr, "address ", Net->Address, "\n", NULL);
		STREAMWriteLine(Tempstr, S);

		if (StrValid(Net->Netmask))
		{
			Tempstr=MCopyStr(Tempstr, "netmask ", Net->Netmask, "\n", NULL);
			STREAMWriteLine(Tempstr, S);
		}

		if (StrValid(Net->Gateway))
		{
			Tempstr=MCopyStr(Tempstr, "gateway ", Net->Gateway, "\n", NULL);
			STREAMWriteLine(Tempstr, S);
		}

		if (StrValid(Net->DNSServer))
		{
			Tempstr=MCopyStr(Tempstr, "dns ", Net->DNSServer, "\n", NULL);
			STREAMWriteLine(Tempstr, S);
		}



		if (StrValid(Net->AccessPoint))
		{
			Tempstr=MCopyStr(Tempstr, "accesspoint ", Net->AccessPoint, "\n", NULL);
			STREAMWriteLine(Tempstr, S);
		}
		
		STREAMWriteLine("\n", S);
	
		Curr=ListGetNext(Curr);
	}
	STREAMClose(S);
}
Destroy(Tempstr);

}



void SettingsConfigureNet(TNet *Net)
{
ListNode *Nets, *Curr;
TNet *Found=NULL, *tmpNet;

Nets=SettingsLoadNets(NULL);

Curr=ListGetNext(Nets);
while (Curr)
{
	tmpNet=(TNet *) Curr->Item;
	if (
			(strcmp(Net->ESSID, tmpNet->ESSID)==0) ||
			(strcmp(Net->AccessPoint, tmpNet->AccessPoint)==0) 
		) Found=tmpNet;
	Curr=ListGetNext(Curr);
}

if (! Found) 
{
	Curr=ListFindNamedItem(Nets, Net->ESSID);
	if (Curr) Found=(TNet *) Curr->Item;
}

if (Found)
{
	if (! StrValid(Net->Key)) Net->Key=CopyStr(Net->Key, Found->Key);
	if (! StrValid(Net->Address)) Net->Address=CopyStr(Net->Address, Found->Address);
	if (! StrValid(Net->Netmask)) Net->Netmask=CopyStr(Net->Netmask, Found->Netmask);
	if (! StrValid(Net->Gateway)) Net->Gateway=CopyStr(Net->Gateway, Found->Gateway);
	if (! StrValid(Net->DNSServer)) Net->DNSServer=CopyStr(Net->DNSServer, Found->DNSServer);
}

ListDestroy(Nets, NetDestroy);
}



void SettingsSaveNet(TNet *Net)
{
ListNode *Nets;
TNet *tmpNet;

Nets=SettingsLoadNets(NULL);
tmpNet=(TNet *) calloc(1, sizeof(TNet));
tmpNet->ESSID=CopyStr(tmpNet->ESSID, Net->ESSID);
tmpNet->Key=CopyStr(tmpNet->Key, Net->Key);
tmpNet->Address=CopyStr(tmpNet->Address, Net->Address);
tmpNet->Netmask=CopyStr(tmpNet->Netmask, Net->Netmask);
tmpNet->Gateway=CopyStr(tmpNet->Gateway, Net->Gateway);
tmpNet->DNSServer=CopyStr(tmpNet->DNSServer, Net->DNSServer);
ListAddNamedItem(Nets, tmpNet->ESSID, tmpNet);

SettingsSaveNets(Nets);

ListDestroy(Nets, NetDestroy);
}


void SettingsForgetNet(const char *ESSID)
{
ListNode *Nets, *Node;

Nets=SettingsLoadNets(NULL);
Node=ListFindNamedItem(Nets, ESSID);
if (Node)
{
	NetDestroy((TNet *) Node->Item);
	ListDeleteNode(Node);
	SettingsSaveNets(Nets);
}

}
