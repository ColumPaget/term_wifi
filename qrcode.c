#include "qrcode.h"
#include "settings.h"


static void QREncode(const char *Msg, const char *Path, const char *Args)
{
    char *Tempstr=NULL;
    STREAM *S, *StdOut;

    Tempstr=MCopyStr(Tempstr, "cmd:qrencode ", Args, NULL);
    if (StrValid(Path)) Tempstr=MCatStr(Tempstr, " -o ", Path, NULL);
    S=STREAMOpen(Tempstr, "");
    if (S)
    {
        STREAMWriteLine(Msg, S);
        STREAMCommit(S);

        StdOut=STREAMOpen("stdout:", "");
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            STREAMWriteLine(Tempstr, StdOut);
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);
}



void DisplayQRCode(TNet *Net)
{
    char *SecureType=NULL, *Tempstr=NULL, *Msg=NULL, *Path=NULL;
    STREAM *S;

    if (Net->Flags & (NET_WPA1 | NET_WPA2 | NET_RSN)) SecureType=CopyStr(SecureType, "WPA");
    else if (Net->Flags & NET_WEP) SecureType=CopyStr(SecureType, "WEP");
    else if (StrValid(Net->Key)) SecureType=CopyStr(SecureType, "WPA");
    else SecureType=CopyStr(SecureType, "");

    Msg=MCopyStr(Msg, "WIFI:S:", Net->ESSID, ";T:", SecureType, ";P:", Net->Key, ";;", NULL);

    printf("QRCODE URL: %s\n", Msg);

    Path=CopyStr(Path, "/tmp/wifi.png");
    if (StrValid(Settings.OutputPath)) Path=CopyStr(Path, Settings.OutputPath);

    QREncode(Msg, Path, "");
    if (! StrValid(Settings.OutputPath))
    {
        Tempstr=FindCommandFromList(Tempstr, Settings.ImageViewer);
        if (StrValid(Tempstr))
        {
            if (strcmp(GetBasename(Tempstr), "convert")==0) Tempstr=MCatStr(Tempstr, " ", Path, " sixel:-", NULL);
            else Tempstr=MCatStr(Tempstr, " ", Path, NULL);
            system(Tempstr);
        }
        else QREncode(Msg, "", "-t ANSI256");
    }

    Destroy(SecureType);
    Destroy(Tempstr);
    Destroy(Path);
    Destroy(Msg);
}



