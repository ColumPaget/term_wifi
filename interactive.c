#include "interactive.h"
#include "wifi.h"
#include "net.h"
#include "netdev.h"
#include "settings.h"

#define DISPLAY_ALL_NETS    0
#define DISPLAY_KNOWN_NETS  1
#define DISPLAY_OPEN_NETS   2
#define DISPLAY_OTHER_NETS  4

ListNode *InteractiveNetList=NULL;
int InteractiveDisplayMode=0;


void InteractiveHeaders(TNetDev *Dev, TNet *Net, STREAM *Out)
{
    char *Line=NULL, *Tempstr=NULL;

    WifiGetStatus(Dev, Net);
    NetGetStatus(Dev, Net);

    Line=MCopyStr(Line, "    Iface:  ~e", Dev->Name, "~0  ", Net->MacAddress, "\n", NULL);
    if (Net->Flags & NET_ASSOCIATED)
    {
        Tempstr=FormatStr(Tempstr, "    Wifi:   ~gASSOCIATED~0  ~e%s~0  AP:%s  chan:%03d  qual:%s%0.1f%%~0\n", Net->ESSID, Net->AccessPoint, Net->Channel, OutputNetQualityColor(Net), Net->Quality * 100.0);
        Line=CatStr(Line, Tempstr);
    }
    else Line=MCatStr(Line, "    Wifi:    ~rnot associated~0~>\n", NULL);

    if (StrValid(Net->Address)) Line=MCatStr(Line, "    IP4:    ~e", Net->Address, "~0 mask ", Net->Netmask, NULL);
    Line=CatStr(Line, "\n\n");

    TerminalPutStr(Line, Out);

    Destroy(Tempstr);
    Destroy(Line);
}


static void InteractiveWifiMenuRefresh(TERMMENU *Menu)
{
    ListNode *Curr;

    Curr=ListGetNext(Menu->Options);
    while (Curr)
    {
        Curr->Tag=OutputFormatNet(Curr->Tag, (TNet *) Curr->Item);
        Curr=ListGetNext(Curr);
    }
}



static void InteractiveWifiMenuUpdate(TERMMENU *Menu)
{
//header for menu
    TerminalCursorMove(StdIO, 0, 6);

    switch (InteractiveDisplayMode)
    {
    case DISPLAY_OPEN_NETS:
        TerminalPutStr("  Open networks that are currently visible:~>", StdIO);
        break;
    case DISPLAY_KNOWN_NETS:
        TerminalPutStr("  Known networks that are currently visible:~>", StdIO);
        break;
    case DISPLAY_OTHER_NETS:
        TerminalPutStr("  Known networks:~>", StdIO);
        break;
    default:
        TerminalPutStr("  Available networks: Those marked with a leading '*' have saved configs~>", StdIO);
        break;
    }

    InteractiveWifiMenuRefresh(Menu);
}


static void InteractiveTitleBar(TNetDev *Dev, const char *Str)
{
    TNet *Net;
    char *Tempstr=NULL;

    TerminalCursorMove(StdIO, 0, 0);
    Tempstr=MCopyStr(Tempstr, Str, "~>~0", NULL);
    TerminalPutStr(Tempstr, StdIO);

    Net=NetCreate();
    TerminalCursorMove(StdIO, 0, 2);
    InteractiveHeaders(Dev, Net, StdIO);

    Destroy(Tempstr);
    NetDestroy(Net);
}


static void InteractiveBottomBar(STREAM *StdIO, int wid, int len)
{
    TerminalCursorMove(StdIO, 0, len-2);
    TerminalPutStr("~B~wKeys: up/down-arrow:select network  Enter:join network  f:forget network  k:known nets  o:open nets~>~0", StdIO);
    TerminalCursorMove(StdIO, 0, len-1);
    TerminalPutStr("~B~w      s:Scan again   d:disconnect   escape-escape:exit  i:change interface~>~0", StdIO);
}


static void InteractiveWifiNetworksReload(TERMMENU *Menu, ListNode *Networks)
{
    ListNode *Curr;
    TNet *Net;

    if (ConfiguredNets) ListDestroy(ConfiguredNets, NetDestroy);
    ConfiguredNets=SettingsLoadNets(NULL);

    ListClear(Menu->Options, NULL);

    if (InteractiveDisplayMode == DISPLAY_OTHER_NETS) Curr=ListGetNext(ConfiguredNets);
    else Curr=ListGetNext(Networks);

    while (Curr)
    {
        Net=(TNet *) Curr->Item;
        if (InteractiveDisplayMode == DISPLAY_KNOWN_NETS)
        {
            if (ListFindNamedItem(ConfiguredNets, Net->ESSID)) ListAddItem(Menu->Options, Net);
        }
        else if (InteractiveDisplayMode == DISPLAY_OPEN_NETS)
        {
            if (! (Net->Flags & NET_ENCRYPTED)) ListAddItem(Menu->Options, Net);
        }
        else ListAddItem(Menu->Options, Net);
        Curr=ListGetNext(Curr);
    }

    InteractiveWifiMenuUpdate(Menu);
}


static void InteractiveWifiScan(TNetDev *Dev, TERMMENU *Menu)
{
    InteractiveTitleBar(Dev, "~R~w Please wait, scanning for wireless networks");

// move cursor in case 'InteractiveWifiScan' asks for a root password,
//otherwise we will overwrite the Title bar with the request
    TerminalCursorMove(StdIO, 0, 6);

    if (! InteractiveNetList) InteractiveNetList=ListCreate();
    InteractiveNetList=WifiGetNetworks(Dev);

//clear any leftover text from password query
    TerminalCursorMove(StdIO, 0, 6);
    TerminalPutStr("~>\n~>\n~>\n~>\n", StdIO);

    InteractiveWifiNetworksReload(Menu, InteractiveNetList);
}

void InteractiveWifiUpdate(TNetDev *Dev, STREAM *StdIO, TERMMENU *Menu, int wid, int len)
{
    DisplayStatus=InteractiveTitleBar;

    InteractiveBottomBar(StdIO, wid, len);
    InteractiveWifiScan(Dev, Menu);
}




int InteractiveQueryNetConfig(TNetDev *Dev, TNet *Net)
{
    char *Tempstr=NULL;

    TerminalClear(StdIO);
    TerminalCursorMove(StdIO, 0, 0);
    TerminalPutStr("~M~w Please enter config for wireless network~>~0\n", StdIO);
    TerminalPutStr("~e Double-escape to cancel and return to network select~0\n\n", StdIO);

    TerminalCursorMove(StdIO, 0, 2);

		//don't timeout reading from StdIO, wait forever for user to
		//enter data as appropriate
		STREAMSetTimeout(StdIO, 0);

    if (Net->Flags & NET_ENCRYPTED)
    {

        Net->CountryCode=TerminalReadPrompt(Net->CountryCode, "2-letter country code: ", 0, StdIO);
        if (Net->CountryCode==NULL)
        {
            TerminalClear(StdIO);
            Destroy(Tempstr);
            return(FALSE);
        }

        StripCRLF(Net->CountryCode);
        TerminalPutStr("\n", StdIO);


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

    Net=NetCreate();

    InteractiveTitleBar(Dev, "~M~w joining network");
    while (1)
    {
        WifiSetup(Dev, Conf);
        usleep(250000);

    		TerminalCursorMove(Out, 0, 2);
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


void InteractiveChangeInterface(TNetDev *Dev)
{
    ListNode *Options, *Curr;
    TNetDev *Iface;
    char *Tempstr=NULL;
    int wid, len, ch;


    TerminalClear(StdIO);
    InteractiveTitleBar(Dev, "~B~w Change Network Interface");
    TerminalGeometry(StdIO, &wid, &len);

    TerminalCursorMove(StdIO, 0, 6);

		//don't timeout, wait for user input
		STREAMSetTimeout(StdIO, 0);
    Options=ListCreate();
    Curr=ListGetNext(Interfaces);
    while (Curr)
    {
        Iface=(TNetDev *) Curr->Item;
        if (Iface->Flags & DEV_WIFI)
        {
            Tempstr=FormatStr(Tempstr, "% 15s % 10s", Iface->Name, Iface->Driver);
            ListAddNamedItem(Options, Tempstr, Iface);
        }
        Curr=ListGetNext(Curr);
    }

    Curr=TerminalMenu(StdIO, Options, 2, 7, wid - 4, len - 11);
    if (Curr)
    {
        Iface=(TNetDev *) Curr->Item;
        Dev->Name=CopyStr(Dev->Name, Iface->Name);
        Dev->Driver=CopyStr(Dev->Driver, Iface->Driver);
        Dev->Flags=Iface->Flags;
    }


    ListDestroy(Options, NULL);
    Destroy(Tempstr);
}



void Interactive(TNetDev *iDev)
{
    TERMMENU *Menu;
    char *Tempstr=NULL, *Line=NULL;
    const char *p_qcolor;
    ListNode *Curr;
    TNetDev *Dev;
    TNet *Net;
    int wid, len, ch;
    int NotExit=TRUE;

    Dev=NetDevClone(iDev);
    if (ConfiguredNets) ListDestroy(ConfiguredNets, NetDestroy);
    ConfiguredNets=SettingsLoadNets(NULL);

    TerminalInit(StdIO, TERM_RAWKEYS | TERM_SAVEATTRIBS);

    TerminalClear(StdIO);
    TerminalGeometry(StdIO, &wid, &len);

    Menu=TerminalMenuCreate(StdIO, 2, 7, wid - 4, len - 11);
    InteractiveWifiUpdate(Dev, StdIO, Menu, wid, len);
    Menu->MenuAttribs=CopyStr(Menu->MenuAttribs, "~N~w");

    while (NotExit)
    {
				//if we're in this loop, then we refresh every 2 seconds
				//other screens launched from here will set Timeout to 0
				//so we need to set back to 200 here
    		STREAMSetTimeout(StdIO, 200);

        Tempstr=FormatStr(Tempstr, "~B~w %d wireless networks found", ListSize(Menu->Options));
        InteractiveTitleBar(Dev, Tempstr);
        TerminalMenuDraw(Menu);

        ch=TerminalReadChar(StdIO);
        switch (ch)
        {
        case STREAM_CLOSED:
            //stdin/terminal closed, exit gracefully
            exit(0);
            break;

        case ESCAPE:
            NotExit=FALSE;
            break;

        case 's':
            InteractiveWifiScan(Dev, Menu);
            break;

        case 'i':
            InteractiveChangeInterface(Dev);
            InteractiveWifiUpdate(Dev, StdIO, Menu, wid, len);
            break;

        case 'd':
            NetDown(Dev);
            break;

        case 'f':
            if (Menu->Options->Side)
            {
                Net=(TNet *) Menu->Options->Side->Item;
                SettingsForgetNet(Net->ESSID);
                InteractiveWifiNetworksReload(Menu, InteractiveNetList);
            }
            break;

        case 'o':
            if (InteractiveDisplayMode != DISPLAY_OPEN_NETS) InteractiveDisplayMode=DISPLAY_OPEN_NETS;
            else InteractiveDisplayMode=DISPLAY_ALL_NETS;
            InteractiveWifiNetworksReload(Menu, InteractiveNetList);
            break;

        case 'k':
            if (InteractiveDisplayMode != DISPLAY_KNOWN_NETS) InteractiveDisplayMode=DISPLAY_KNOWN_NETS;
            else InteractiveDisplayMode=DISPLAY_ALL_NETS;
            InteractiveWifiNetworksReload(Menu, InteractiveNetList);
            break;

        case 'h':
            if (InteractiveDisplayMode != DISPLAY_OTHER_NETS) InteractiveDisplayMode=DISPLAY_OTHER_NETS;
            else InteractiveDisplayMode=DISPLAY_ALL_NETS;
            InteractiveWifiNetworksReload(Menu, InteractiveNetList);
            break;
            break;

        default:
            Curr=TerminalMenuOnKey(Menu, ch);
            if (Curr)
            {
                Net=(TNet *) Curr->Item;

                SettingsConfigureNet(Net);

                InteractiveJoinNetwork(Dev, Net, StdIO);
                InteractiveWifiNetworksReload(Menu, InteractiveNetList);
                TerminalMenuDraw(Menu);
            }
            break;
        }
    }

    TerminalClear(StdIO);
    TerminalCursorMove(StdIO, 0, 0);
    TerminalReset(StdIO);

    NetDevDestroy(Dev);
    STREAMDestroy(StdIO);

    Destroy(Tempstr);
    Destroy(Line);
}
