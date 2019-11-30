#include "interactive.h"
#include "wifi.h"
#include "net.h"
#include "settings.h"


STREAM *Out;



static void InteractiveHeaders(TNetDev *Dev, STREAM *Out)
{
char *Line=NULL, *Tempstr=NULL;
TNet *Net;

Net=(TNet *) calloc(1, sizeof(TNet));
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

NetDestroy(Net);
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
char *Tempstr=NULL;

TerminalCursorMove(Out, 0, 0);
Tempstr=MCopyStr(Tempstr, Str, "~>~0", NULL);
TerminalPutStr(Tempstr, Out);

InteractiveHeaders(Dev, Out);

Destroy(Tempstr);
}


void InteractiveQueryNetConfig(TNetDev *Dev, TNet *Net)
{
char *Tempstr=NULL;

TerminalClear(Out);
TerminalCursorMove(Out, 0, 0);
TerminalPutStr("~M~w Please enter config for wireless network~>~0", Out);

TerminalCursorMove(Out, 0, 2);
Net->Key=TerminalReadPrompt(Net->Key, "Key/password: ", 0, Out);
StripCRLF(Net->Key);

TerminalPutStr("\n", Out);
Net->Address=TerminalReadPrompt(Net->Address, "Address (blank for DHCP): ", 0, Out);
StripCRLF(Net->Address);

if (StrValid(Net->Address)) 
{
	TerminalPutStr("\n", Out);
	Net->Netmask=TerminalReadPrompt(Net->Netmask, "Netmask: ", 0, Out);
	StripCRLF(Net->Netmask);

	TerminalPutStr("\n", Out);
	Net->Gateway=TerminalReadPrompt(Net->Gateway, "Gateway: ", 0, Out);
	StripCRLF(Net->Gateway);

	TerminalPutStr("\n", Out);
	Net->DNSServer=TerminalReadPrompt(Net->DNSServer, "DNS Server: ", 0, Out);
	StripCRLF(Net->DNSServer);
}
else Net->Address=CopyStr(Net->Address, "dhcp");


TerminalPutStr("\n", Out);
TerminalPutStr("\n", Out);
Tempstr=TerminalReadPrompt(Tempstr, "Save for future use? Y/n:  ", 0, Out);
StripLeadingWhitespace(Tempstr);
StripTrailingWhitespace(Tempstr);

if (StrValid(Tempstr) && (tolower(Tempstr[0])=='y') ) SettingsSaveNet(Net);


TerminalClear(Out);
Destroy(Tempstr);
}



void Interactive(TNetDev *Dev)
{
TERMMENU *Menu;
char *Tempstr=NULL, *Line=NULL;
const char *p_qcolor;
ListNode *Curr;
TNet *Net;
int wid, len, ch;

Out=STREAMFromDualFD(0, 1);
TerminalInit(Out, TERM_RAWKEYS | TERM_SAVEATTRIBS);

TerminalClear(Out);
TerminalGeometry(Out, &wid, &len);

DisplayStatus=InteractiveTitleBar;

InteractiveTitleBar(Dev, "~R~w Please wait, scanning for wireless networks");
InteractiveHeaders(Dev, Out);

// move cursor in case 'InteractiveWifiScan' asks for a root password,
//otherwise we will overwrite the Title bar with the request
TerminalCursorMove(Out, 0, 6);
Menu=TerminalMenuCreate(Out, 2, 7, wid - 4, len - 10);
InteractiveWifiScan(Dev, Menu);

//clear any leftover text from password query
TerminalCursorMove(Out, 0, 6);
TerminalPutStr("~>\n~>\n~>\n~>\n", Out);

//header for menu
TerminalCursorMove(Out, 0, 6);
TerminalPutStr("  Available networks: Those marked with a leading '*' have saved configs", Out);

Tempstr=FormatStr(Tempstr, "~B~w %d wireless networks found", ListSize(Menu->Options));
InteractiveTitleBar(Dev, Tempstr);

Menu->MenuAttribs=CopyStr(Menu->MenuAttribs, "~N~w");

STREAMSetTimeout(Out, 200);
while (1)
{
	InteractiveHeaders(Dev, Out);
	TerminalMenuDraw(Menu);
	TerminalCursorMove(Out, 0, len-2);
	TerminalPutStr("~B~wKeys:  up-arrow/down-arrow:select network   Enter:join network   f:forget network~>\r\n       s:Scan again   d:disconnect   escape-escape:exit~>~0", Out);

	ch=TerminalReadChar(Out);
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
			InteractiveHeaders(Dev, Out);
			InteractiveWifiMenuUpdate(Menu);
			TerminalMenuDraw(Menu);
		}

		InteractiveTitleBar(Dev, "~M~w joining network");
		WifiSetup(Dev, Net);
		usleep(20000);
		NetSetupInterface(Dev, Net->Address, Net->Netmask, Net->Gateway, Net->DNSServer);

		InteractiveTitleBar(Dev, "~B~w done");
	}
	}
}

TerminalClear(Out);
TerminalCursorMove(Out, 0, 0);
TerminalReset(Out);

STREAMDestroy(Out);

Destroy(Tempstr);
Destroy(Line);
}
