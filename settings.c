#include "settings.h"

TSettings Settings;


char *SettingsRenderPath(char *RetStr, const char *iPath)
{
    if (strncmp(iPath, "~/", 2)==0) RetStr=MCopyStr(RetStr, GetCurrUserHomeDir(), iPath +1, NULL);
		else RetStr=CopyStr(RetStr, iPath);

return(RetStr);
}

void SettingsInit()
{
    memset(&Settings, 0, sizeof(Settings));

    Settings.PidsDir=CopyStr(Settings.PidsDir, DEFAULT_PIDS_DIR);
    Settings.ConfigFile=CopyStr(Settings.ConfigFile, DEFAULT_CONFIG_FILE);
    Settings.SyncImportLog=CopyStr(Settings.SyncImportLog, DEFAULT_IMPORT_LOG);
    Settings.ImageViewer=CopyStr(Settings.ImageViewer, "imlib2_view,fim,feh,display,xv,phototonic,qimageviewer,pix,sxiv,qimgv,qview,nomacs,geeqie,ristretto,mirage,fotowall,links -g,convert,img2sixel -e");
}





TNet *SettingsGetNet(ListNode *Nets, const char *Essid, const char *Match)
{
    TNet *Net=NULL;
    ListNode *Node;

    if ( (! StrValid(Match)) || (strcasecmp(Match, Essid)==0) )
    {
        Node=ListFindNamedItem(Nets, Essid);
        if (Node) Net=(TNet *) Node->Item;
        else
        {
            Net=NetCreate();
            Net->ESSID=CopyStr(Net->ESSID, Essid);
            Net->AccessPoint=CopyStr(Net->AccessPoint, "");
            ListAddNamedItem(Nets, Net->ESSID, Net);
        }
    }

    return(Net);
}


int SettingsAddNets(ListNode *Nets, const char *Path, const char *Match)
{
    STREAM *S;
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;
    ListNode *Curr;
    TNet *Net;

		Tempstr=SettingsRenderPath(Tempstr, Path);
    S=STREAMOpen(Tempstr, "r");
    if (! S) return(FALSE);

    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);

            ptr=GetToken(Tempstr, "\\S", &Token, 0);
            if (strcasecmp(Token, "essid")==0) Net=SettingsGetNet(Nets, ptr, Match);
            else if (Net)
            {
                if (strcasecmp(Token, "wpa1")==0) Net->Flags |= NET_WPA1;
                else if (strcasecmp(Token, "wpa2")==0) Net->Flags |= NET_WPA2;
                else if (strcasecmp(Token, "wep")==0) Net->Flags |= NET_WEP;
                else if (strcasecmp(Token, "rsn")==0) Net->Flags |= NET_RSN;
                else if (strcasecmp(Token, "user")==0) Net->UserID=CopyStr(Net->UserID, ptr);
                else if (strcasecmp(Token, "key")==0) Net->Key=CopyStr(Net->Key, ptr);
                else if (strcasecmp(Token, "address")==0) Net->Address=CopyStr(Net->Address, ptr);
                else if (strcasecmp(Token, "netmask")==0) Net->Netmask=CopyStr(Net->Netmask, ptr);
                else if (strcasecmp(Token, "gateway")==0) Net->Gateway=CopyStr(Net->Gateway, ptr);
                else if (strcasecmp(Token, "country")==0) Net->CountryCode=CopyStr(Net->CountryCode, ptr);
                else if (strcasecmp(Token, "dns")==0) Net->DNSServer=CopyStr(Net->DNSServer, ptr);
                else if (strcasecmp(Token, "accesspoint")==0) Net->AccessPoint=CopyStr(Net->AccessPoint, ptr);
                else if (strcasecmp(Token, "title")==0) Net->Title=CopyStr(Net->Title, ptr);
                else if (strcasecmp(Token, "added")==0) Net->DateAdded=CopyStr(Net->DateAdded, ptr);
                else if (strcasecmp(Token, "channel")==0) Net->Channel=atoi(ptr);
            }

            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }


    Curr=ListGetNext(Nets);
    while (Curr)
    {
        Net=(TNet *) Curr->Item;
        if (StrValid(Net->Key) && (! (Net->Flags & NET_ENCRYPTED)) ) Net->Flags |= NET_WPA2;
        Curr=ListGetNext(Curr);
    }

    Destroy(Tempstr);
    Destroy(Token);

    return(TRUE);
}



ListNode *SettingsLoadNets(const char *Path, const char *Match)
{
    ListNode *Nets=NULL;
    char *Token=NULL;
    const char *ptr;

    Nets=ListCreate();
		ptr=GetToken(Path, ":", &Token, 0);
		while (ptr)
		{
    SettingsAddNets(Nets, Token, Match);
		ptr=GetToken(ptr, ":", &Token, 0);
		}

		Destroy(Token);

    return(Nets);
}





int SettingsWriteNets(const char *Path, ListNode *List)
{
    ListNode *Curr;
    char *Tempstr=NULL;
    TNet *Net;
    STREAM *S;
    int RetVal=FALSE;


		Tempstr=SettingsRenderPath(Tempstr, Path);
		MakeDirPath(Tempstr, 0700);
    S=STREAMOpen(Tempstr, "w");
    if (S)
    {
				RetVal=TRUE;
        Curr=ListGetNext(List);
        while (Curr)
        {
            Net=(TNet *) Curr->Item;

            //essid MUST be the first thing in the entry
            Tempstr=MCopyStr(Tempstr, "essid ", Net->ESSID, "\n", NULL);
            STREAMWriteLine(Tempstr, S);


            if (Net->Flags & NET_RSN) STREAMWriteLine("rsn\n", S);
            if (Net->Flags & NET_WPA2) STREAMWriteLine("wpa2\n", S);
            if (Net->Flags & NET_WPA1) STREAMWriteLine("wpa1\n", S);
            if (Net->Flags & NET_WEP) STREAMWriteLine("wep\n", S);

            if (StrValid(Net->Title))
            {
                Tempstr=MCopyStr(Tempstr, "title ", Net->Title, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->CountryCode))
            {
                Tempstr=MCopyStr(Tempstr, "country ", Net->CountryCode, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->UserID))
            {
                Tempstr=MCopyStr(Tempstr, "user ", Net->UserID, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->Key))
            {
                Tempstr=MCopyStr(Tempstr, "key ", Net->Key, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->Address))
            {
                Tempstr=MCopyStr(Tempstr, "address ", Net->Address, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->Netmask))
            {
                Tempstr=MCopyStr(Tempstr, "netmask ", Net->Netmask, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->Gateway))
            {
                Tempstr=MCopyStr(Tempstr, "gateway ", Net->Gateway, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->DNSServer))
            {
                Tempstr=MCopyStr(Tempstr, "dns ", Net->DNSServer, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->AccessPoint))
            {
                Tempstr=MCopyStr(Tempstr, "accesspoint ", Net->AccessPoint, "\n", NULL);
                STREAMWriteLine(Tempstr, S);
            }

            if (StrValid(Net->DateAdded)) Tempstr=MCopyStr(Tempstr, "added ", Net->DateAdded, "\n", NULL);
            else Tempstr=MCopyStr(Tempstr, "added ", GetDateStr("%Y-%m-%dT%H:%M:%S", NULL),  "\n", NULL);
            STREAMWriteLine(Tempstr, S);

            STREAMWriteLine("\n", S);

            Curr=ListGetNext(Curr);
        }
        STREAMClose(S);
    }
    Destroy(Tempstr);

return(RetVal);
}


int SettingsSaveNets(const char *Path, ListNode *List)
{
char *Token=NULL;
const char *ptr;
int RetVal=TRUE;

ptr=GetToken(Path, ":", &Token, 0);
while (ptr)
{
RetVal=SettingsWriteNets(Token, List);
if (RetVal) break;
ptr=GetToken(ptr, ":", &Token, 0);
}

Destroy(Token);

return(RetVal);
}


void SettingsConfigureNet(TNet *Net)
{
    ListNode *Nets, *Curr;
    TNet *Found=NULL, *tmpNet;

    Nets=SettingsLoadNets(Settings.ConfigFile, NULL);

    Curr=ListGetNext(Nets);
    while (Curr)
    {
        tmpNet=(TNet *) Curr->Item;
        if (
            (strcmp(Net->ESSID, tmpNet->ESSID)==0) ||
            (strcmp(Net->AccessPoint, tmpNet->AccessPoint)==0)
        ) Found=tmpNet;
        Curr=ListGetNext(Curr);
    }

    if (! Found)
    {
        Curr=ListFindNamedItem(Nets, Net->ESSID);
        if (Curr) Found=(TNet *) Curr->Item;
    }

    if (Found)
    {
        if (! StrValid(Net->Key)) Net->Key=CopyStr(Net->Key, Found->Key);
        if (! StrValid(Net->UserID)) Net->UserID=CopyStr(Net->UserID, Found->UserID);
        if (! StrValid(Net->Address)) Net->Address=CopyStr(Net->Address, Found->Address);
        if (! StrValid(Net->Netmask)) Net->Netmask=CopyStr(Net->Netmask, Found->Netmask);
        if (! StrValid(Net->Gateway)) Net->Gateway=CopyStr(Net->Gateway, Found->Gateway);
        if (! StrValid(Net->DNSServer)) Net->DNSServer=CopyStr(Net->DNSServer, Found->DNSServer);
    }

    ListDestroy(Nets, NetDestroy);
}



void SettingsSaveNet(TNet *Net)
{
    ListNode *Nets;
    TNet *tmpNet;

    Nets=SettingsLoadNets(Settings.ConfigFile, NULL);
    tmpNet=NetCreate();
    tmpNet->ESSID=CopyStr(tmpNet->ESSID, Net->ESSID);
    tmpNet->UserID=CopyStr(tmpNet->UserID, Net->UserID);
    tmpNet->Key=CopyStr(tmpNet->Key, Net->Key);
    tmpNet->Address=CopyStr(tmpNet->Address, Net->Address);
    tmpNet->Netmask=CopyStr(tmpNet->Netmask, Net->Netmask);
    tmpNet->Gateway=CopyStr(tmpNet->Gateway, Net->Gateway);
    tmpNet->DNSServer=CopyStr(tmpNet->DNSServer, Net->DNSServer);
    tmpNet->AccessPoint=CopyStr(tmpNet->AccessPoint, Net->AccessPoint);
    tmpNet->CountryCode=CopyStr(tmpNet->CountryCode, Net->CountryCode);
    ListAddNamedItem(Nets, tmpNet->ESSID, tmpNet);

    SettingsSaveNets(Settings.ConfigFile, Nets);

    ListDestroy(Nets, NetDestroy);
}


void SettingsForgetNet(const char *ESSID)
{
    ListNode *Nets, *Node;

    Nets=SettingsLoadNets(Settings.ConfigFile, NULL);
    Node=ListFindNamedItem(Nets, ESSID);
    if (Node)
    {
        NetDestroy((TNet *) Node->Item);
        ListDeleteNode(Node);
        SettingsSaveNets(Settings.ConfigFile, Nets);
    }

}
