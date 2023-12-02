#include "netdev.h"

#define SYSFS_NETS_GLOB "/sys/class/net/*"

TNetDev *NetDevCreate(const char *Name, int Flags)
{
    TNetDev *Dev;

    Dev=(TNetDev *) calloc(1, sizeof(TNetDev));
    Dev->Name=CopyStr(Dev->Name, Name);
    Dev->Flags=Flags;

    return(Dev);
}

TNetDev *NetDevClone(TNetDev *Parent)
{
    TNetDev *Dev;

    Dev=NetDevCreate(Parent->Name, Parent->Flags);
    Dev->Driver=CopyStr(Dev->Driver, Parent->Driver);

    return(Dev);
}

void NetDevDestroy(void *p_Item)
{
    TNetDev *Dev;

    Dev=(TNetDev *) p_Item;
    Destroy(Dev->Name);
    Destroy(Dev->Driver);
    free(Dev);
}


int NetDevLoadInterfaces(ListNode *Interfaces)
{
    glob_t Glob;
    TNetDev *Dev;
    char *Tempstr=NULL, *Token=NULL;
    const char *iname, *ptr;
    int i, Flags=0;
    STREAM *S;

    glob(SYSFS_NETS_GLOB, 0, 0, &Glob);
    for (i=0; i < Glob.gl_pathc; i++)
    {
        iname=GetBasename(Glob.gl_pathv[i]);
        if (StrValid(iname))
        {
            Tempstr=MCopyStr(Tempstr, Glob.gl_pathv[i], "/wireless", NULL);
            if (access(Tempstr, F_OK)==0) Flags |= DEV_WIFI;

            Dev=NetDevCreate(iname, Flags);

            Tempstr=MCopyStr(Tempstr, Glob.gl_pathv[i], "/device/uevent", NULL);
            S=STREAMOpen(Tempstr, "r");
            if (S)
            {
                Tempstr=STREAMReadLine(Tempstr, S);
                while (Tempstr)
                {
                    StripTrailingWhitespace(Tempstr);
                    ptr=GetToken(Tempstr, "=", &Token, 0);
                    if (strcmp(Token, "DRIVER")==0) Dev->Driver=CopyStr(Dev->Driver, ptr);
                    Tempstr=STREAMReadLine(Tempstr, S);
                }
                STREAMClose(S);
            }

            ListAddNamedItem(Interfaces, iname, Dev);
        }
    }

    Destroy(Tempstr);

    return(ListSize(Interfaces));
}


TNetDev *NetDevSelectInterface(ListNode *Interfaces, const char *Interface)
{
    ListNode *Curr;
    TNetDev *Dev;

    if (StrValid(Interface)) Curr=ListFindNamedItem(Interfaces, Interface);
    else
    {
        Curr=ListFindNamedItem(Interfaces, "wlan0");
        if (! Curr)
        {
            Curr=ListGetNext(Interfaces);
            while (Curr)
            {
                Dev=(TNetDev *) Curr->Item;
                if (Dev->Flags & DEV_WIFI) break;
                Curr=ListGetNext(Curr);
            }
        }
    }

    if (Curr) return((TNetDev *) Curr->Item);

    return(NULL);
}

