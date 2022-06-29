#include "net.h"
#include "runcommand.h"
#include "settings.h"


void NetSetupInterface(TNetDev *Interface, const char *Address, const char *Netmask, const char *Gateway, const char *DNSServer)
{
    char *Tempstr=NULL, *Output=NULL;

    if ( StrValid(Address) && (strcmp(Address, "dhcp")==0) )
    {
        if (DisplayStatus) DisplayStatus(Interface, "~Y~nLaunching dhcpcd");

        Tempstr=MCopyStr(Tempstr, "dhcpcd-", Interface->Name, NULL);
        PidFileKill(Tempstr);
        Tempstr=MCopyStr(Tempstr, "dhclient-", Interface->Name, NULL);
        PidFileKill(Tempstr);
        usleep(10000);

        if (CommandFound("dhcpcd")) Tempstr=MCopyStr(Tempstr, "dhcpcd ", Interface->Name, " -h ", OSSysInfoString(OSINFO_HOSTNAME), NULL);
        else Tempstr=MCopyStr(Tempstr, "dhclient ", Interface->Name, " -pf ", Settings.PidsDir, "dhclient-", Interface->Name, ".pid", NULL);

        Output=RunCommand(Output, Tempstr, RUNCMD_ROOT | RUNCMD_NOSHELL);
    }
    else
    {
        Tempstr=MCopyStr(Tempstr, "ifconfig ", Interface->Name, NULL);
        if (StrValid(Address))
        {
            Tempstr=MCatStr(Tempstr, " ", Address, NULL);
            if (DisplayStatus) DisplayStatus(Interface, "~Y~nConfiguring network");
        }

        if (StrValid(Netmask)) Tempstr=MCatStr(Tempstr, " netmask ", Netmask, NULL);
        Tempstr=CatStr(Tempstr, " up");

        Tempstr=CatStr(Tempstr, " &> /dev/null");
        Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);

        if (StrValid(Gateway))
        {
            Tempstr=MCopyStr(Tempstr, "route add default gw ", Gateway, " &> /dev/null", NULL);
            Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);
        }

        if (StrValid(DNSServer))
        {
            Tempstr=MCopyStr(Tempstr, "echo \"nameserver ", DNSServer, "\" > /etc/resolv.conf", NULL);
            Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);
        }
    }


    Destroy(Tempstr);
    Destroy(Output);
}



void NetDown(TNetDev *Dev)
{
    char *Tempstr=NULL, *Output=NULL;

    Tempstr=MCopyStr(Tempstr,"wpa_supplicant-",Dev->Name,NULL);
    PidFileKill(Tempstr);

    Tempstr=MCopyStr(Tempstr,"dhcpcd-",Dev->Name,NULL);
    PidFileKill(Tempstr);

    Tempstr=MCopyStr(Tempstr, "ifconfig ", Dev->Name, " down", NULL);
    Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);

    Destroy(Tempstr);
    Destroy(Output);
}


const char *IfConfigToks[]= {"inet", "Bcast", "Mask", "HWaddr", "RX bytes", "TX bytes", NULL};
typedef enum {IFCT_INET_ADDR, IFCT_BCAST, IFCT_NETMASK, IFCT_HWADDR, IFCT_RX, OFCT_TX} EIFCONFIGS;

static void NetGetStatusFromIfConfigParseLine(TNet *Net, const char *Line)
{
    char *Tempstr=NULL;
    const char *ptr;

    ptr=Line;
    while (ptr)
    {
        ptr=GetToken(ptr,"\\S|:|=",&Tempstr,GETTOKEN_MULTI_SEPARATORS|GETTOKEN_QUOTES);
        if (! ptr) break;
        while (isspace(*ptr)) ptr++;
        StripTrailingWhitespace(Tempstr);
        StripLeadingWhitespace(Tempstr);

        switch (MatchTokenFromList(Tempstr, IfConfigToks, 0))
        {
        case IFCT_INET_ADDR:
            //grab and throw away the 'addr:' part of 'inet addr:'
            ptr=GetToken(ptr, ":", &Net->Address, 0);
            ptr=GetToken(ptr, "\\S", &Net->Address, 0);
            break;

        case IFCT_BCAST:
            //ptr=GetToken(ptr, "\\S", &Net->Broadcast, 0);
            break;

        case IFCT_NETMASK:
            ptr=GetToken(ptr, "\\S", &Net->Netmask, 0);
            break;

        case IFCT_HWADDR:
            ptr=GetToken(ptr, "\\S", &Net->MacAddress, 0);
            break;
        }
    }

    Destroy(Tempstr);
}


static void NetGetStatusFromIfconfig(TNetDev *Dev, TNet *Net)
{
    STREAM *S;
    char *Output=NULL, *Line=NULL, *Tempstr=NULL;
    const char *ptr;
    ListNode *Curr;
    int val, val2;

    Net->Address=CopyStr(Net->Address,"");

    Tempstr=MCopyStr(Tempstr, "ifconfig ", Dev->Name, NULL);
    Output=RunCommand(Output, Tempstr, 0);
    ptr=GetToken(Output, "\n", &Line, 0);
    while (ptr)
    {
        StripTrailingWhitespace(Line);
        NetGetStatusFromIfConfigParseLine(Net, Line);

        ptr=GetToken(ptr, "\n", &Line, 0);
    }

    Destroy(Line);
    Destroy(Output);
    Destroy(Tempstr);
}



void NetGetStatus(TNetDev *Dev, TNet *Net)
{
    NetGetStatusFromIfconfig(Dev, Net);
}
