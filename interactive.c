#include "interactive.h"
#include "wifi.h"
#include "net.h"
#include "settings.h"



static void InteractiveHeaders(TNetDev *Dev, TNet *Net, STREAM *Out)
{
char *Line=NULL, *Tempstr=NULL;

WifiGetStatus(Dev, Net);
NetGetStatus(Dev, Net);

TerminalCursorMove(Out, 0, 2);
Line=MCopyStr(Line, "    Iface:  ~e", Dev->Name, "~0  ", Net->MacAddress, "\n", NULL);
if (Net->Flags & NET_ASSOCIATED) 
{
	Tempstr=FormatStr(Tempstr, "    Wifi:   ~gASSOCIATED~0  ~e%s~0  AP:%s  chan:%02d  qual:%s%0.1f%%~0\n", Net->ESSID, Net->AccessPoint, Net->Channel, OutputNetQualityColor(Net), Net->Quality * 100.0);
	Line=CatStr(Line, Tempstr);
}
else Line=MCatStr(Line, "    Wifi:    ~rnot associated~0~>\n", NULL);

if (StrValid(Net->Address)) Line=MCatStr(Line, "    IP4:    ~e", Net->Address, "~0 mask ", Net->Netmask, NULL);
Line=CatStr(Line, "\n\n");

TerminalPutStr(Line, Out);

Destroy(Tempstr);
Destroy(Line);
}


static void InteractiveWifiMenuUpdate(TERMMENU *Menu)
{
ListNode *Curr;

ConfiguredNets=SettingsLoadNets(NULL);
Curr=ListGetNext(Menu->Options);
while (Curr)
{
Curr->Tag=OutputFormatNet(Curr->Tag, (TNet *) Curr->Item);
Curr=ListGetNext(Curr);
}

ListDestroy(ConfiguredNets, NetDestroy);
ConfiguredNets=NULL;
}


static void InteractiveWifiScan(TNetDev *Dev, TERMMENU *Menu)
{
ListNode *Networks, *Curr;
char *Tempstr=NULL;

Networks=WifiGetNetworks(Dev);
ListClear(Menu->Options, NULL);
Curr=ListGetNext(Networks);
while (Curr)
{
	ListAddItem(Menu->Options, Curr->Item);
	Curr=ListGetNext(Curr);
}

InteractiveWifiMenuUpdate(Menu);
ListDestroy(Networks, NULL);

Destroy(Tempstr);
}


void InteractiveTitleBar(TNetDev *Dev, const char *Str)
{
TNet *Net;
char *Tempstr=NULL;

TerminalCursorMove(StdIO, 0, 0);
Tempstr=MCopyStr(Tempstr, Str, "~>~0", NULL);
TerminalPutStr(Tempstr, StdIO);


Net=(TNet *) calloc(1, sizeof(TNet));
InteractiveHeaders(Dev, Net, StdIO);

Destroy(Tempstr);
NetDestroy(Net);
}


void InteractiveQueryNetConfig(TNetDev *Dev, TNet *Net)
{
char *Tempstr=NULL;

TerminalClear(StdIO);
TerminalCursorMove(StdIO, 0, 0);
TerminalPutStr("~M~w Please enter config for wireless network~>~0", StdIO);

TerminalCursorMove(StdIO, 0, 2);
Net->Key=TerminalReadPrompt(Net->Key, "Key/password: ", 0, StdIO);
StripCRLF(Net->Key);

TerminalPutStr("\n", StdIO);
Net->Address=TerminalReadPrompt(Net->Address, "Address (blank for DHCP): ", 0, StdIO);
StripCRLF(Net->Address);

if (StrValid(Net->Address)) 
{
	TerminalPutStr("\n", StdIO);
	Net->Netmask=TerminalReadPrompt(Net->Netmask, "Netmask: ", 0, StdIO);
	StripCRLF(Net->Netmask);

	TerminalPutStr("\n", StdIO);
	Net->Gateway=TerminalReadPrompt(Net->Gateway, "Gateway: ", 0, StdIO);
	StripCRLF(Net->Gateway);

	TerminalPutStr("\n", StdIO);
	Net->DNSServer=TerminalReadPrompt(Net->DNSServer, "DNS Server: ", 0, StdIO);
	StripCRLF(Net->DNSServer);
}
else Net->Address=CopyStr(Net->Address, "dhcp");


TerminalPutStr("\n", StdIO);
TerminalPutStr("\n", StdIO);
Tempstr=TerminalReadPrompt(Tempstr, "Save for future use? Y/n:  ", 0, StdIO);
StripLeadingWhitespace(Tempstr);
StripTrailingWhitespace(Tempstr);

if (StrValid(Tempstr) && (tolower(Tempstr[0])=='y') ) SettingsSaveNet(Net);


TerminalClear(StdIO);
Destroy(Tempstr);
}


void InteractiveJoinNetwork(TNetDev *Dev, TNet *Conf, STREAM *Out)
{
TNet *Net;

	Net=(TNet *) calloc(1, sizeof(TNet));

	InteractiveTitleBar(Dev, "~M~w joining network");
	WifiSetup(Dev, Conf);
	while (1)
	{		
		usleep(20000);

		InteractiveHeaders(Dev, Net, Out);
		if (Net->Flags & NET_ASSOCIATED) 
		{
				NetSetupInterface(Dev, Conf->Address, Conf->Netmask, Conf->Gateway, Conf->DNSServer);
				break;
		}
	}

	InteractiveTitleBar(Dev, "~B~w done");
	NetDestroy(Net);
}



void Interactive(TNetDev *Dev)
{
TERMMENU *Menu;
char *Tempstr=NULL, *Line=NULL;
const char *p_qcolor;
ListNode *Curr;
TNet *Net;
int wid, len, ch;

TerminalInit(StdIO, TERM_RAWKEYS | TERM_SAVEATTRIBS);

TerminalClear(StdIO);
TerminalGeometry(StdIO, &wid, &len);

DisplayStatus=InteractiveTitleBar;

InteractiveTitleBar(Dev, "~R~w Please wait, scanning for wireless networks");

// move cursor in case 'InteractiveWifiScan' asks for a root password,
//otherwise we will overwrite the Title bar with the request
TerminalCursorMove(StdIO, 0, 6);
Menu=TerminalMenuCreate(StdIO, 2, 7, wid - 4, len - 11);
InteractiveWifiScan(Dev, Menu);

//clear any leftover text from password query
TerminalCursorMove(StdIO, 0, 6);
TerminalPutStr("~>\n~>\n~>\n~>\n", StdIO);

//header for menu
TerminalCursorMove(StdIO, 0, 6);
TerminalPutStr("  Available networks: Those marked with a leading '*' have saved configs", StdIO);

Menu->MenuAttribs=CopyStr(Menu->MenuAttribs, "~N~w");

STREAMSetTimeout(StdIO, 200);
while (1)
{
	Tempstr=FormatStr(Tempstr, "~B~w %d wireless networks found", ListSize(Menu->Options));
	InteractiveTitleBar(Dev, Tempstr);
	TerminalMenuDraw(Menu);
	TerminalCursorMove(StdIO, 0, len-2);
	TerminalPutStr("~B~wKeys: up/down-arrow:select network  Enter:join network  f:forget network~>~0", StdIO);
	TerminalCursorMove(StdIO, 0, len-1);
  TerminalPutStr("~B~w      s:Scan again   d:disconnect   escape-escape:exit~>~0", StdIO);

	ch=TerminalReadChar(StdIO);
	if (ch==ESCAPE) break;
	else if (ch=='s') InteractiveWifiScan(Dev, Menu);
	else if (ch=='d') NetDown(Dev);
	else if (ch=='f') 
	{
		if (Menu->Options->Side)
		{
			Net=(TNet *) Menu->Options->Side->Item;
			SettingsForgetNet(Net->ESSID);
			InteractiveWifiMenuUpdate(Menu);
		}
	}
	else
	{
	Curr=TerminalMenuOnKey(Menu, ch);
	if (Curr)
	{
		Net=(TNet *) Curr->Item;

		SettingsConfigureNet(Net);
		if (! StrValid(Net->Address)) 
		{
			InteractiveQueryNetConfig(Dev, Net);
			InteractiveWifiMenuUpdate(Menu);
			TerminalMenuDraw(Menu);
		}

		InteractiveJoinNetwork(Dev, Net, StdIO);
	}
	}
}

TerminalClear(StdIO);
TerminalCursorMove(StdIO, 0, 0);
TerminalReset(StdIO);

STREAMDestroy(StdIO);

Destroy(Tempstr);
Destroy(Line);
}
