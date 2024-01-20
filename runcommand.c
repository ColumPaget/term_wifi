#include "runcommand.h"
#include "settings.h"
#include <sys/wait.h>

ListNode *Commands=NULL;

void CommandsInit()
{
    const char *CmdList[]= {"echo", "su","sudo","ifconfig", "route", "iwconfig","iwlist","iw","wpa_supplicant","dhcpcd","dhclient","kill",NULL};
    char *Path=NULL, *Tempstr=NULL;
    int i;

    Commands=ListCreate();
    Path=MCopyStr(Path, "/usr/sbin:/sbin:/bin:/usr/bin:", getenv("PATH"), NULL);

    for (i=0; CmdList[i] != NULL; i++)
    {
        Tempstr=FindFileInPath(Tempstr, CmdList[i], Path);
        if (StrValid(Tempstr))
        {
            ListAddNamedItem(Commands, CmdList[i], CopyStr(NULL, Tempstr));
        }
    }

    Destroy(Tempstr);
    Destroy(Path);
}

int CommandFound(const char *Name)
{
    if (StrValid(GetVar(Commands, Name))) return(TRUE);
    return(FALSE);
}

char *RunCommand(char *RetStr, const char *Command, int Flags)
{
    char *Exec=NULL, *Tempstr=NULL;
    const char *p_args, *p_Path, *p_ExecPath;
    STREAM *S;
    int i;

    for (i=0; i < 100; i++)
    {
        if (waitpid(-1, NULL, WNOHANG) < 1) break;
    }

//if we're already root (effective uid 0) then remove the RUNCMD_ROOT flag as we
//don't want to switch to root (this will only be removed in the current function because
//the Flags variable is pass-by-value
    if (geteuid()==0) Flags &= ~RUNCMD_ROOT;

    p_args=GetToken(Command, "\\S", &Exec, 0);
    p_ExecPath=GetVar(Commands, Exec);

//if we can't find an executable for the command, indicate this by returning null
    if (! StrValid(p_ExecPath))
    {
        Destroy(Exec);
        Destroy(RetStr);
        return(NULL);
    }


    if (Flags & RUNCMD_ROOT)
    {
        p_Path=GetVar(Commands, "sudo");
        if (StrValid(p_Path))
        {
            Exec=MCopyStr(Exec, "cmd:", p_Path, "  -S ", p_ExecPath, " ", p_args, NULL);
            if (! StrValid(Settings.RootPassword)) QueryRootPassword("~eOperation requires sudo (usually user's) password~0\r\nPassword: ");
        }
        else
        {
            p_Path=GetVar(Commands, "su");
            if (StrValid(p_Path))
            {
                Exec=MCopyStr(Exec, "cmd:", p_Path, "  -c '", p_ExecPath, " ", p_args, "'", NULL);
                if (! StrValid(Settings.RootPassword)) QueryRootPassword("~eOperation requires root password~0\r\nPassword: ");
            }
        }

        if (! p_Path) printf("ERROR: neither sudo nor su found, operations will likely fail\n");
    }
    else Exec=MCopyStr(Exec, "cmd:", p_ExecPath, " ", p_args, NULL);

    Tempstr=CopyStr(Tempstr, "pty");
    if (Flags & RUNCMD_DAEMON) Tempstr=CatStr(Tempstr, " daemon");
    if (Flags & RUNCMD_NOSHELL) Tempstr=CatStr(Tempstr, " noshell");

    S=STREAMOpen(Exec, Tempstr);
    if (S)
    {
        if (Flags & RUNCMD_ROOT)
        {
            for (i=0; i < 10; i++)
            {
                if (STREAMCountWaitingBytes(S) > 6) break;
                usleep(10000);
            }
            Tempstr=MCopyStr(Tempstr, Settings.RootPassword, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }
        RetStr=STREAMReadDocument(RetStr, S);
        STREAMClose(S);
    }
    else
    {
        //we couldn't launch the command. Indicate this by returning null
        Destroy(RetStr);
        RetStr=NULL;
    }

    Destroy(Tempstr);
    Destroy(Exec);

    return(RetStr);
}
