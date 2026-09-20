#include "sync.h"
#include "settings.h"

int SyncAlreadyImported(const char *Path, const char *Hash)
{
    char *Tempstr=NULL, *FName=NULL, *FHash=NULL;
    const char *ptr;
    int RetVal=TRUE;
    STREAM *S;

    S=STREAMOpen(Settings.SyncImportLog, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);

            ptr=GetToken(Tempstr, "\\S", &FName, GETTOKEN_QUOTES);
            ptr=GetToken(ptr, "\\S", &FHash, 0);

            if ( (strcmp(GetBasename(Path), FName)==0) && (strcmp(Hash, FHash) == 0) ) RetVal=TRUE;

            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(FName);
    Destroy(FHash);

    return(RetVal);
}


void SyncRegisterImported(const char *Path, const char *Hash)
{
    char *Tempstr=NULL;
    STREAM *S;

    S=STREAMOpen(Settings.SyncImportLog, "a");
    if (S)
    {
        Tempstr=MCopyStr(Tempstr, "'", GetBasename(Path), "' ", Hash, " ", GetDateStr("%Y-%m-%dT%HL%M:%S", NULL), "\n", NULL);
        STREAMWriteLine(Tempstr, S);
        STREAMClose(S);
    }

    Destroy(Tempstr);
}




int Import(const char *Path)
{
    ListNode *Nets;
    char *Hash=NULL;
    int RetVal=FALSE;


    HashFile(&Hash, "sha1", Path, ENCODE_BASE64);
    //if (! SyncAlreadyImported(Path, Hash))
    {
        Nets=SettingsLoadNets(Settings.ConfigFile, NULL);
fprintf(stderr, "LN: %d\n", Nets);
        if (SettingsAddNets(Nets, Path, NULL))
        {
            SettingsSaveNets(Settings.ConfigFile, Nets);
fprintf(stderr, "LS: %d\n", ListSize(Nets));

            RetVal=TRUE;
            SyncRegisterImported(Path, Hash);
        }
    }

    Destroy(Hash);

    return(RetVal);
}

char *ExportBuildPath(char *Path, const char *iPath)
{
    char *MachineID=NULL, *HostName=NULL;

    MachineID=FileRead(MachineID, "/etc/machine-id");
    StripTrailingWhitespace(MachineID);
    HostName=CopyStr(HostName, OSSysInfoString(OSINFO_HOSTNAME));
    StripTrailingWhitespace(HostName);

    Path=MCopyStr(Path, iPath, "/", NULL);
    if (StrValid(MachineID)) Path=MCatStr(Path, MachineID, "-", NULL);
    if (StrValid(HostName)) Path=MCatStr(Path, HostName, "-", NULL);
    Path=CatStr(Path, "term_wifi.sync");

    Destroy(MachineID);
    Destroy(HostName);

    return(Path);
}


int Export(const char *iPath)
{
    ListNode *Nets;
    int RetVal=FALSE;
    char *Path=NULL;
    struct stat Stat;


    stat(iPath, &Stat);
    Path=CopyStr(Path, iPath);
    StrRTruncChar(Path, '/');

    //path must be a directory
    if (S_ISDIR(Stat.st_mode) || (! StrValid(Path))) Path=ExportBuildPath(Path, iPath);
    else Path=CopyStr(Path, iPath);

    Nets=SettingsLoadNets(Settings.ConfigFile, NULL);
    if (ListSize(Nets) > 0)
    {
        MakeDirPath(Path, 0777);
        SettingsSaveNets(Path, Nets);
        chmod(Path, 0666);
        RetVal=TRUE;
    }

    Destroy(Path);
    return(RetVal);
}

