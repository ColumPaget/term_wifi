#include "wireless_tools.h"
#include "runcommand.h"






static void IWGetNetworksParseLine(TNet *Net, const char *Line)
{
char *Key=NULL, *Token=NULL;
const char *ptr;

	ptr=GetToken(Line, ":", &Key, 0);
	while (isspace(*ptr)) ptr++;

	if (strcmp(Key, "SSID")==0) Net->ESSID=CopyStr(Net->ESSID, ptr);
	else if (strcmp(Key, "DS Parameter set")==0) 
	{
		ptr=GetToken(ptr, "\\S", &Token, 0);
		while (ptr)
		{
		if (strcmp(Token, "channel")==0)
		{
		ptr=GetToken(ptr, "\\S", &Token, 0);
		Net->Channel=atoi(Token);
		}
		ptr=GetToken(ptr, "\\S", &Token, 0);
		}
	}
	else if (strcmp(Key, "Mode")==0) 
	{
		if (strcasecmp(ptr, "ad-hoc")==0) Net->Flags |= NET_ADHOC;
	}
	else if (strcmp(Key, "WPA")==0) 
	{
		if (strstr(ptr, "Version: 2")) Net->Flags |=NET_WPA2;
		else Net->Flags |=NET_WPA1;
	}
	else if (strcmp(Key, "RSN")==0)  Net->Flags |= NET_RSN;
	else if (strcmp(Key, "signal")==0)  Net->dBm = atof(ptr);
	else if (strcasecmp(Key, "Supported rates")==0) Net->BitRates=MCatStr(Net->BitRates, ptr, " ", NULL);
	else if (strcasecmp(Key, "Extended supported rates")==0) Net->BitRates=MCatStr(Net->BitRates, ptr, " ", NULL);

Destroy(Token);
Destroy(Key);
}



void IWGetNetworks(TNetDev *Dev, ListNode *Networks)
{
char *Tempstr=NULL, *Output=NULL;
const char *ptr;
TNet *Net=NULL;

NetSetupInterface(Dev, "", "", "", "");
Tempstr=MCopyStr(Tempstr, "iw dev ", Dev->Name, " scan trigger", NULL);
Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);
sleep(2);

Tempstr=MCopyStr(Tempstr, "iw dev ", Dev->Name, " scan dump", NULL);
Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);

ptr=GetToken(Output, "\n", &Tempstr, 0);
while (ptr)
{
	StripLeadingWhitespace(Tempstr);
	StripTrailingWhitespace(Tempstr);

	if (
				(strncmp(Tempstr, "BSS ", 4) == 0) &&
				(strncmp(Tempstr, "BSS Load:", 9) != 0) 
	)
	{
		//add previously configured net
		if (Net) ListAddNamedItem(Networks, Net->ESSID, Net);

		//load next network
		Net=(TNet *) calloc(1, sizeof(TNet));
		GetToken(Tempstr+4, "\\S|(", &Net->AccessPoint, GETTOKEN_MULTI_SEP);
	}
	else if (Net) IWGetNetworksParseLine(Net, Tempstr);

	ptr=GetToken(ptr, "\n", &Tempstr, 0);
}

if (Net) ListAddNamedItem(Networks, Net->ESSID, Net);

Destroy(Tempstr);
Destroy(Output);
}


int IWSetupInterface(TNetDev *Dev, TNet *Conf)
{
char *Tempstr=NULL, *Cmd=NULL;

Cmd=MCopyStr(Cmd, "iw dev ", Dev->Name, " connect -w \"", Conf->ESSID, "\" ", NULL);

if (Conf->Channel > 0) 
{
//	Tempstr=FormatStr(Tempstr, " chan '%d' ", Conf->Channel);
	Cmd=CatStr(Cmd, Tempstr);
}

Tempstr=RunCommand(Tempstr, Cmd, RUNCMD_ROOT);

Destroy(Tempstr);
Destroy(Cmd);
}




//IW config, at the wifi level
void IWGetStatus(TNetDev *Device, TNet *Net)
{
char *Tempstr=NULL, *Output=NULL, *Line=NULL;
const char *ptr;

if (! Device) return;

Tempstr=MCopyStr(Tempstr, "iwconfig ", Device->Name, NULL);
Output=RunCommand(Output, Tempstr, 0);

/*
ptr=GetToken(Output, "\n", &Line, 0);
while (ptr)
{
	StripTrailingWhitespace(Line);
	
	WirelessToolsGetStatusParseLine(Net, Line);
	ptr=GetToken(ptr, "\n", &Line, 0);
}


if (
        (! StrValid(Net->AccessPoint)) ||
        (strcasecmp(Net->AccessPoint,"not-associated")==0) 
		) Net->Flags &= ~NET_ASSOCIATED;
		else Net->Flags |= NET_ASSOCIATED;
*/

Destroy(Line);
Destroy(Output);
Destroy(Tempstr);
}


