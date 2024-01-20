#include "wpa_supplicant.h"
#include "runcommand.h"
#include "settings.h"


static void WPASupplicantParseFlags(const char *Data, TNet *Net)
{
    char *Token=NULL;
    const char *ptr;

    ptr=GetToken(Data, "[|]|+", &Token, GETTOKEN_MULTI_SEP);
    while (ptr)
    {
        if (StrValid(Token))
        {
            if (strncmp(Token, "WPA-", 4) == 0) Net->Flags |=NET_WPA1;
            else if (strncmp(Token, "WPA2-", 5) == 0) Net->Flags |=NET_WPA2;
            else if (strcmp(Token, "TKIP") == 0) /* do nothing */;
            else if (strcmp(Token, "ESS") == 0) /* do nothing */;
            else if (strcmp(Token, "WPS") == 0) /* do nothing */;
            else
            {
                Net->Flags |=NET_RSN;
            }
        }

        ptr=GetToken(ptr, "[|]|+", &Token, GETTOKEN_MULTI_SEP);
    }

    Destroy(Token);
}


static int WPASupplicantParseNetwork(const char *Data, ListNode *Networks)
{
    char *Token=NULL;
    const char *ptr;
    TNet *Net;
    int RetVal=FALSE;

    if (StrValid(Data))
    {
        ptr=GetToken(Data, "	", &Token, 0);
        if (strcmp(Token, "bssid") !=0)
        {
            Net=NetCreate();
            Net->AccessPoint=CopyStr(Net->AccessPoint, Token);
            ptr=GetToken(ptr, "	", &Token, 0);
            Net->Channel=FrequencyToChannel(atoi(Token));
            ptr=GetToken(ptr, "	", &Token, 0);
            Net->dBm=atoi(Token);
            ptr=GetToken(ptr, "	", &Token, 0);
            WPASupplicantParseFlags(Token, Net);

            Net->ESSID=CopyStr(Net->ESSID, ptr);

            ListAddNamedItem(Networks, Net->ESSID, Net);
            RetVal=TRUE;
        }
    }

    Destroy(Token);

    return(RetVal);
}


static int WPASupplicantScanResults(STREAM *S, ListNode *Networks)
{
    char *Tempstr=NULL;
    int RetVal=FALSE;

    STREAMSetTimeout(S, 10);
    STREAMWriteLine("SCAN_RESULTS", S);
    STREAMFlush(S);
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        if (strncmp(Tempstr, "bssid", 5)==0) break;
        Tempstr=STREAMReadLine(Tempstr, S);
    }

    Tempstr=STREAMReadLine(Tempstr, S);
    while (StrValid(Tempstr))
    {
        StripTrailingWhitespace(Tempstr);
        RetVal=WPASupplicantParseNetwork(Tempstr, Networks);
        Tempstr=STREAMReadLine(Tempstr, S);
    }

    Destroy(Tempstr);
    return(RetVal);
}


int WPASupplicantGetNetworks(TNetDev *Dev, ListNode *Networks)
{
    STREAM *S;
    char *Tempstr=NULL;
    int result;

    if (! StrValid(Settings.WPASupplicantSock)) return(FALSE);
    Tempstr=MCopyStr(Tempstr, "unixdgram:", Settings.WPASupplicantSock, "/", Dev->Name, NULL);
    S=STREAMOpen(Tempstr, "rw");
    if (S)
    {
        Tempstr=FormatStr(Tempstr, "/tmp/.wpa_client_%d.sock", getpid());
        UnixSocketBindPath(S->in_fd, Tempstr);

        STREAMWriteLine("SCAN", S);
        STREAMFlush(S);
        Tempstr=STREAMReadLine(Tempstr, S);
        WPASupplicantScanResults(S, Networks);

        STREAMClose(S);
    }

    Destroy(Tempstr);

    if (ListSize(Networks) > 0) return(TRUE);
    return(FALSE);
}


int WPASupplicantWriteConfig(const char *Path, TNet *Net)
{
    char *Tempstr=NULL;
    int RetVal=FALSE;
    STREAM *S;

    S=STREAMOpen(Path, "w");
    if (S)
    {
        if (StrValid(Net->CountryCode))
        {
            Tempstr=MCopyStr(Tempstr, "country=", Net->CountryCode, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }

        if (StrLen(Net->UserID)) Tempstr=FormatStr(Tempstr,"network={\nssid=\"%s\"\nkey_mgmt=WPA-EAP\neap=TTLS\nphase2=\"auth=PAP\"\nidentity=\"%s\"\npassword=\"%s\"\n}\n", Net->ESSID, Net->UserID, Net->Key);
        else Tempstr=FormatStr(Tempstr,"network={\nssid=\"%s\"\nscan_ssid=1\nkey_mgmt=WPA-PSK\npsk=\"%s\"\n}\n", Net->ESSID, Net->Key);
        STREAMWriteLine(Tempstr,S);
        STREAMClose(S);
        RetVal=TRUE;
    }

    Destroy(Tempstr);

    return(RetVal);
}


int WPASupplicantRun(TNetDev *Dev, TNet *Net, const char *Driver)
{
    char *Tempstr=NULL, *Output=NULL, *Path=NULL;
    int RetVal=FALSE;

    Path=FormatStr(Path,"/tmp/%s.wpa",Dev->Name);
    if (WPASupplicantWriteConfig(Path, Net))
    {
        Tempstr=FormatStr(Tempstr,"wpa_supplicant -i %s -D %s -B -c %s -P %s/wpa_supplicant-%s.pid", Dev->Name, Driver, Path, Settings.PidsDir, Dev->Name);
        Output=RunCommand(Output, Tempstr, RUNCMD_ROOT | RUNCMD_NOSHELL);

        if (strstr(Output, "Failed to initialize driver interface")) RetVal=FALSE;
        else RetVal=TRUE;
    }

    unlink(Path);

    Destroy(Tempstr);
    Destroy(Output);
    Destroy(Path);

    return(RetVal);
}


int WPASupplicantActivate(TNetDev *Dev, TNet *Net)
{
    const char *WPADrivers[]= {"nl80211","wext","wired",NULL};
    char *Tempstr=NULL;
    int i, RetVal=FALSE;

    if (DisplayStatus) DisplayStatus(Dev, "~M~wLaunching wpa_supplicant");
    Tempstr=MCopyStr(Tempstr,"wpa_supplicant-",Dev->Name,".pid",NULL);
    PidFileKill(Tempstr);
    usleep(10000);

    for (i=0; WPADrivers[i] !=NULL; i++)
    {
        if (WPASupplicantRun(Dev, Net, WPADrivers[i]))
        {
            RetVal=TRUE;
            break;
        }
        usleep(10000);
    }

    usleep(50000);

    Destroy(Tempstr);

    return(RetVal);
}

