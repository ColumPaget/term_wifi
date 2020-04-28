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


int InteractiveQueryNetConfig(TNetDev *Dev, TNet *Net)
{
char *Tempstr=NULL;

TerminalClear(StdIO);
TerminalCursorMove(StdIO, 0, 0);
TerminalPutStr("~M~w Please enter config for wireless network~>~0\n", StdIO);
TerminalPutStr("~e Double-escape to cancel and return to network select~0\n\n", StdIO);

TerminalCursorMove(StdIO, 0, 2);

if (Net->Flags & NET_ENCRYPTED)
{

Net->UserID=TerminalReadPrompt(Net->UserID, "Username (blank for none): ", 0, StdIO);
if (Net->UserID==NULL)
{
TerminalClear(StdIO);
Destroy(Tempstr);
return(FALSE);
}

StripCRLF(Net->UserID);
TerminalPutStr("\n", StdIO);


Net->Key=TerminalReadPrompt(Net->Key, "Key/password: ", 0, StdIO);
if (Net->Key==NULL)
{
TerminalClear(StdIO);
Destroy(Tempstr);
return(FALSE);
}


StripCRLF(Net->Key);
TerminalPutStr("\n", StdIO);
}



Net->Address=TerminalReadPrompt(Net->Address, "Address (blank for DHCP): ", 0, StdIO);
if (Net->Address==NULL)
{
TerminalClear(StdIO);
Destroy(Tempstr);
return(FALSE);
}

StripCRLF(Net->Address);
TerminalPutStr("\n", StdIO);

if (StrValid(Net->Address)) 
{

	Net->Netmask=TerminalReadPrompt(Net->Netmask, "Netmask: ", 0, StdIO);
	if (Net->Netmask==NULL)
	{
	TerminalClear(StdIO);
	Destroy(Tempstr);
	return(FALSE);
	}
	
	StripCRLF(Net->Netmask);
	TerminalPutStr("\n", StdIO);

	Net->Gateway=TerminalReadPrompt(Net->Gateway, "Gateway: ", 0, StdIO);
	if (Net->Gateway==NULL)
	{
	TerminalClear(StdIO);
	Destroy(Tempstr);
	return(FALSE);
	}

	StripCRLF(Net->Gateway);
	TerminalPutStr("\n", StdIO);

	Net->DNSServer=TerminalReadPrompt(Net->DNSServer, "DNS Server: ", 0, StdIO);
	if (Net->DNSServer==NULL)
	{
	TerminalClear(StdIO);
	Destroy(Tempstr);
	return(FALSE);
	}

	StripCRLF(Net->DNSServer);
}
else Net->Address=CopyStr(Net->Address, "dhcp");


TerminalPutStr("\n", StdIO);
TerminalPutStr("\n", StdIO);
Tempstr=TerminalReadPrompt(Tempstr, "Save for future use? Y/n:  ", 0, StdIO);
if (Tempstr==NULL)
{
	TerminalClear(StdIO);
	return(FALSE);
}

StripLeadingWhitespace(Tempstr);
StripTrailingWhitespace(Tempstr);

if (StrValid(Tempstr) && (tolower(Tempstr[0])=='y') ) SettingsSaveNet(Net);


TerminalClear(StdIO);
Destroy(Tempstr);

return(TRUE);
}



void InteractiveJoinNetwork(TNetDev *Dev, TNet *Conf, STREAM *Out)
{
TNet *Net;

	if (! StrValid(Conf->Address)) 
	{
		if (! InteractiveQueryNetConfig(Dev, Conf)) return;
	}

	Net=(TNet *) calloc(1, sizeof(TNet));

	InteractiveTitleBar(Dev, "~M~w joining network");
	while (1)
	{		
		WifiSetup(Dev, Conf);
		usleep(250000);

		InteractiveHeaders(Dev, Net, Out);
		if (Net->Flags & NET_ASSOCIATED) 
		{
				usleep(10000); //if we've just associated then allow some time for the process to complete
				NetSetupInterface(Dev, Conf->Address, Conf->Netmask, Conf->Gateway, Conf->DNSServer);
				Net->Flags &= ~NET_JOINING;
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

		InteractiveJoinNetwork(Dev, Net, StdIO);
		InteractiveWifiMenuUpdate(Menu);
		TerminalMenuDraw(Menu);

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
