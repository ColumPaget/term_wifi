#include "net.h"
#include "netdev.h"
#include "wifi.h"
#include "runcommand.h"
#include "settings.h"
#include "command_line.h"
#include "interactive.h"
#include "qrcode.h"
#include "help.h"


#define ACT_INTERACTIVE 0
#define ACT_ADD    1
#define ACT_JOIN   2
#define ACT_LEAVE  3
#define ACT_LIST   4
#define ACT_SCAN   5
#define ACT_FORGET 6
#define ACT_IFACE_LIST 7
#define ACT_QRCODE     8
#define ACT_HELP   99




void ListInterfaces()
{
    ListNode *Curr, *Networks;
    TNetDev *Iface;

    printf("Interfaces: \n");
    Curr=ListGetNext(Interfaces);
    while (Curr)
    {
        Iface=(TNetDev *) Curr->Item;
        if (Iface->Flags & DEV_WIFI) printf("% 15s %s\n", Iface->Name, Iface->Driver);
        Curr=ListGetNext(Curr);
    }

}


void ListNetworks()
{
    ListNode *Curr, *Networks;
    TNet *Net;

    printf("Configured networks: \n");
    Networks=SettingsLoadNets(NULL);
    Curr=ListGetNext(Networks);
    while (Curr)
    {
        Net=(TNet *) Curr->Item;
        printf("% 15s ip:%s netmask:%s gateway:%s dns:%s\n", Net->ESSID, Net->Address, Net->Netmask, Net->Gateway, Net->DNSServer);
        Curr=ListGetNext(Curr);
    }

}


void ScanForNetworks(TNetDev *Dev)
{
    ListNode *Networks, *Curr;
    char *Tempstr=NULL, *Output=NULL;
    TNet *Net;

    ConfiguredNets=SettingsLoadNets(NULL);
    Networks=WifiGetNetworks(Dev);
    Curr=ListGetNext(Networks);
    while (Curr)
    {
        Net=(TNet *) Curr->Item;

        Output=OutputFormatNet(Output, Net);
        Output=CatStr(Output, "\n");
        TerminalPutStr(Output, StdIO);
        Curr=ListGetNext(Curr);
    }

    ListDestroy(ConfiguredNets, NetDestroy);

    Destroy(Tempstr);
    Destroy(Output);
}






int main(int argc, char *argv[])
{
    ListNode *Networks;
    char *Tempstr=NULL;
    TNet *Conf, *Net;
    int Action;
    TNetDev *Dev;

    StdIO=STREAMFromDualFD(0, 1);
    SettingsInit();
    CommandsInit();

    Interfaces=ListCreate();
    NetDevLoadInterfaces(Interfaces);

    Conf=NetCreate();
    Action=ParseCommandLine(argc, argv, Conf);

    Dev=NetDevSelectInterface(Interfaces, Conf->Interface);
    if (Dev)
    {
        switch (Action)
        {
        case ACT_ADD:
            if (! StrValid(Conf->Key)) Conf->Key=TerminalReadPrompt(Conf->Key, "Key/password: ", 0, StdIO);
            SettingsConfigureNet(Conf);
            SettingsSaveNet(Conf);
            break;

        case ACT_JOIN:
            SettingsConfigureNet(Conf);
            if (Dev->Flags & DEV_WIFI)
            {
                if (! StrValid(Conf->Key)) Conf->Key=TerminalReadPrompt(Conf->Key, "Key/password: ", 0, StdIO);
                printf("Configure wifi: dev:%s essid:%s\n", Conf->Interface, Conf->ESSID);
                WifiSetup(Dev, Conf);
            }

            Net=(TNet *) calloc(1, sizeof(TNet));
            while (1)
            {
                WifiGetStatus(Dev, Net);
                if (Net->Flags & NET_ASSOCIATED) break;
            }
            NetDestroy(Net);

            printf("Configure IPv4: ip:%s netmask:%s gw:%s dns:%s\n", Conf->Address, Conf->Netmask, Conf->Gateway, Conf->DNSServer);
            NetSetupInterface(Dev, Conf->Address, Conf->Netmask, Conf->Gateway, Conf->DNSServer);
            if (Conf->Flags & NET_STORE) SettingsSaveNet(Conf);
            break;

        case ACT_QRCODE:
            SettingsConfigureNet(Conf);
            DisplayQRCode(Conf);
            break;

        case ACT_FORGET:
            SettingsForgetNet(Conf->ESSID);
            break;

        case ACT_LIST:
            ListNetworks();
            break;

        case ACT_SCAN:
            ScanForNetworks(Dev);
            break;

        case ACT_LEAVE:
            NetDown(Dev);
            break;

        case ACT_INTERACTIVE:
            Interactive(Dev);
            break;

        case ACT_IFACE_LIST:
            ListInterfaces();
            break;

        case ACT_VERSION:
						printf("term_wifi %s\n", VERSION);
            break;

        case ACT_HELP:
            DisplayHelp();
            break;


        }
    }

    Destroy(Tempstr);

    return(0);
}
