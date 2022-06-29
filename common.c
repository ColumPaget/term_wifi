#include "common.h"
#include "settings.h"
#include "runcommand.h"

INTERACTIVE_STATUS_CALLBACK DisplayStatus=NULL;
ListNode *ConfiguredNets=NULL;
ListNode *Interfaces=NULL;
STREAM *StdIO=NULL;

void SendSignal(pid_t pid, int Sig)
{
    char *Tempstr=NULL, *Cmd=NULL;

//if we are root, then just use 'kill' function
    if (geteuid()==0) kill(pid, Sig);
    else
    {
        Cmd=FormatStr(Cmd, "kill -%d %d\n", Sig, pid);
        Tempstr=RunCommand(Tempstr, Cmd, RUNCMD_ROOT);
    }

    Destroy(Tempstr);
    Destroy(Cmd);
}


void PidFileKill(const char *AppName)
{
    char *Path=NULL, *Tempstr=NULL;
    STREAM *S;
    pid_t pid;

    Path=MCopyStr(Path, Settings.PidsDir, "/", AppName, ".pid", NULL);
    S=STREAMOpen(Path, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        pid=atoi(Tempstr);

        SendSignal(pid, SIGTERM);
        usleep(10000);
        SendSignal(pid, SIGKILL);


        STREAMClose(S);
        unlink(Path);
    }

    Destroy(Tempstr);
    Destroy(Path);
}


void QueryRootPassword(const char *Prompt)
{
    STREAM *S;

    S=STREAMFromDualFD(0,1);
    Settings.RootPassword=TerminalReadPrompt(Settings.RootPassword, Prompt, TERM_SHOWSTARS, S);
    StripCRLF(Settings.RootPassword);
    STREAMDestroy(S);
}

int FrequencyToChannel(int freq)
{
    return( (freq - 2412) / 5 + 1);
}


TNet *NetCreate()
{
    TNet *Net;

    Net=(TNet *) calloc(1, sizeof(TNet));
    Net->Interface=CopyStr(Net->Interface, "");
    Net->ESSID=CopyStr(Net->ESSID, "");
    Net->Address=CopyStr(Net->Address, "");
    Net->Netmask=CopyStr(Net->Netmask, "");
    Net->Key=CopyStr(Net->Key, "");
    Net->CountryCode=CopyStr(Net->CountryCode, "");
    Net->AccessPoint=CopyStr(Net->AccessPoint, "");

    return(Net);
}



void NetDestroy(void *p_Net)
{
    TNet *Net;

    Net=(TNet *) p_Net;

    Destroy(Net->Interface);
    Destroy(Net->ESSID);
    Destroy(Net->AccessPoint);
    Destroy(Net->Address);
    Destroy(Net->Netmask);
    Destroy(Net->Gateway);
    Destroy(Net->BitRates);
    Destroy(Net->CountryCode);
    Destroy(Net->UserID);
    Destroy(Net->Key);
    free(Net);
}



const char *OutputNetQualityColor(TNet *Net)
{
    if (Net->Quality > 0.75) return("~g");
    if (Net->Quality > 0.4) return("~y");

    if (Net->dBm > -65) return("~g");
    if (Net->dBm > -70) return("~y");
    return("~r");
}


char *OutputFormatNet(char *Output, TNet *Net)
{
    char *Tempstr=NULL;
    const char *p_qcolor, *ptr;
    float Rate=0.0, val;

    if (ListFindNamedItem(ConfiguredNets, Net->ESSID)) Output=CopyStr(Output, "* ");
    else Output=CopyStr(Output, "  ");

    if (Net->Flags & NET_RSN) Output=CatStr(Output, "~gRSN ~0 ");
    else if (Net->Flags & NET_WPA2) Output=CatStr(Output, "~gWPA2~0 ");
    else if (Net->Flags & NET_WPA1) Output=CatStr(Output, "~gWPA1~0 ");
    else if (Net->Flags & NET_WEP) Output=CatStr(Output, "~bWEP ~0  ");
    else Output=CatStr(Output, "~rOPEN~0 ");

    if (Net->Quality > 0)
    {
        p_qcolor=OutputNetQualityColor(Net);
        Tempstr=FormatStr(Tempstr, "  %s% 6.1f%%~0 ", p_qcolor, Net->Quality * 100.0);
        Output=CatStr(Output,Tempstr);
    }
    else
    {
        p_qcolor=OutputNetQualityColor(Net);
        Tempstr=FormatStr(Tempstr, "  %s%0.1fdBm~0 ", p_qcolor, Net->dBm);
        Output=CatStr(Output,Tempstr);
    }

    ptr=GetToken(Net->BitRates, "\\S", &Tempstr, 0);
    while (ptr)
    {
        val=atof(Tempstr);
        if (val > Rate) Rate=val;
        ptr=GetToken(ptr, "\\S", &Tempstr, 0);
    }

    Tempstr=FormatStr(Tempstr, "  %0.1fMb/s   chan:%02d  %s  ", Rate, Net->Channel, Net->AccessPoint);
    Output=CatStr(Output, Tempstr);

    Output=MCatStr(Output, "~e", Net->ESSID, "~0 ", NULL);

    Destroy(Tempstr);

    return(Output);
}

