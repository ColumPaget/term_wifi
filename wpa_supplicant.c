#include "wpa_supplicant.h"
#include "runcommand.h"
#include "settings.h"


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
const char *WPADrivers[]={"nl80211","wext","wired",NULL};
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

