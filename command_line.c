#include "command_line.h"
#include "settings.h"

int ParseCommandLine(int argc, char *argv[], TNet *Conf)
{
    CMDLINE *CL;
    int Act=ACT_INTERACTIVE;
    const char *ptr;

    CL=CommandLineParserCreate(argc, argv);

    ptr=CommandLineFirst(CL);
    if (StrValid(ptr))
    {
        if (strcmp(ptr, "list")==0) Act=ACT_LIST;
        else if (strcmp(ptr, "interfaces")==0) Act=ACT_IFACE_LIST;
        else if (strcmp(ptr, "scan")==0)
        {
            ptr=CommandLineNext(CL);
            if (ListFindNamedItem(Interfaces, ptr))
            {
                Conf->Interface=CopyStr(Conf->Interface, ptr);
            }
            Act=ACT_SCAN;
        }
        else if (strcmp(ptr, "add")==0)
        {
            Act=ACT_ADD;
            Conf->Flags |= NET_STORE;
            Conf->ESSID=CopyStr(Conf->ESSID, CommandLineNext(CL));
            Conf->Address=CopyStr(Conf->Address, CommandLineNext(CL));
            if (strcasecmp(Conf->Address, "dhcp") !=0)
            {
                Conf->Netmask=CopyStr(Conf->Netmask, CommandLineNext(CL));
                Conf->Gateway=CopyStr(Conf->Gateway, CommandLineNext(CL));
                Conf->DNSServer=CopyStr(Conf->DNSServer, CommandLineNext(CL));
            }
        }
        else if (strcmp(ptr, "join")==0)
        {
            Act=ACT_JOIN;
            Conf->Interface=CopyStr(Conf->Interface, CommandLineNext(CL));
            if (! ListFindNamedItem(Interfaces, Conf->Interface))
            {
                printf("ERROR: '%s' is not an interface\n", Conf->Interface);
                printf("usage: %s join <interface> <essid>\n", argv[0]);
                exit(1);
            }

            Conf->ESSID=CopyStr(Conf->ESSID, CommandLineNext(CL));
        }
        else if (strcmp(ptr, "connect")==0)
        {
            Act=ACT_JOIN;
            Conf->ESSID=CopyStr(Conf->ESSID, CommandLineNext(CL));
            Conf->Interface=CopyStr(Conf->Interface, CommandLineNext(CL));
        }
        else if (strcmp(ptr, "leave")==0)
        {
            Act=ACT_LEAVE;
            Conf->Interface=CopyStr(Conf->Interface, CommandLineNext(CL));
            if (! ListFindNamedItem(Interfaces, Conf->Interface))
            {
                printf("ERROR: '%s' is not an interface\n", Conf->Interface);
                printf("usage: %s join <interface> <essid>\n", argv[0]);
                exit(1);
            }
        }
        else if (strcmp(ptr, "forget")==0)
        {
            Act=ACT_FORGET;
            Conf->ESSID=CopyStr(Conf->ESSID, CommandLineNext(CL));
        }
        else if (strcmp(ptr, "qrcode")==0)
        {
            Act=ACT_QRCODE;
            Conf->ESSID=CopyStr(Conf->ESSID, CommandLineNext(CL));
        }
				else if (strcmp(ptr, "status")==0) Act=ACT_STATUS;
        else if (strcmp(ptr, "help")==0) Act=ACT_HELP;
        else if (strcmp(ptr, "version")==0) Act=ACT_VERSION;
    }



    ptr=CommandLineCurr(CL);
    while (ptr)
    {
        if (strcmp(ptr, "-?")==0) Act=ACT_HELP;
        else if (strcmp(ptr, "-h")==0) Act=ACT_HELP;
        else if (strcmp(ptr, "-help")==0) Act=ACT_HELP;
        else if (strcmp(ptr, "--help")==0) Act=ACT_HELP;
        else if (strcmp(ptr, "-version")==0) Act=ACT_VERSION;
        else if (strcmp(ptr, "--version")==0) Act=ACT_VERSION;
        else if (strcmp(ptr, "-i")==0) Conf->Interface=CopyStr(Conf->Interface, CommandLineNext(CL));
        else if (strcmp(ptr, "-ap")==0) Conf->AccessPoint=CopyStr(Conf->AccessPoint, CommandLineNext(CL));
        else if (strcmp(ptr, "-k")==0) Conf->Key=CopyStr(Conf->Key, CommandLineNext(CL));
        else if (strcmp(ptr, "-w")==0) Settings.WPASupplicantSock=CopyStr(Settings.WPASupplicantSock, CommandLineNext(CL));
        else if (strcmp(ptr, "-o")==0) Settings.OutputPath=CopyStr(Settings.OutputPath, CommandLineNext(CL));
        else if (strcmp(ptr, "-viewer")==0) Settings.ImageViewer=CopyStr(Settings.ImageViewer, CommandLineNext(CL));
        else if (strcmp(ptr, "-view")==0)
        {
            ptr=CommandLineNext(CL);
            if (strcasecmp(ptr, "sixel")==0) Settings.ImageViewer=CopyStr(Settings.ImageViewer, "img2sixel -e,convert");
            else Settings.ImageViewer=CopyStr(Settings.ImageViewer, CommandLineNext(CL));
        }

        ptr=CommandLineNext(CL);
    }

    return(Act);
}

