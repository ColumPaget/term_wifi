#include "wpa_supplicant.h"
#include "runcommand.h"
#include "settings.h"

int WPASupplicantActivate(TNetDev *Dev, TNet *Net)
{
char *Tempstr=NULL, *Output=NULL, *Path=NULL;
STREAM *S;

	if (DisplayStatus) DisplayStatus(Dev, "~M~wLaunching wpa_supplicant");
  Tempstr=MCopyStr(Tempstr,"wpa_supplicant-",Dev->Name,".pid",NULL);
  PidFileKill(Tempstr);
	usleep(10000);

  Path=FormatStr(Path,"/tmp/%s.wpa",Dev->Name);
  S=STREAMOpen(Path, "w");
  if (S)
  {
    if (StrLen(Net->UserID)) Tempstr=FormatStr(Tempstr,"network={\nssid=\"%s\"\nkey_mgmt=WPA-EAP\neap=TTLS\nphase2=\"auth=PAP\"\nidentity=\"%s\"\npassword=\"%s\"\n}\n", Net->ESSID, Net->UserID, Net->Key);
    else Tempstr=FormatStr(Tempstr,"network={\nssid=\"%s\"\nscan_ssid=1\nkey_mgmt=WPA-PSK\npsk=\"%s\"\n}\n", Net->ESSID, Net->Key);
    STREAMWriteLine(Tempstr,S);

    STREAMClose(S);

    Tempstr=FormatStr(Tempstr,"wpa_supplicant -i %s -B -c %s -P %s/wpa_supplicant-%s.pid", Dev->Name, Path, Settings.PidsDir, Dev->Name);
    Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);
    usleep(50000);
  }

  unlink(Path);

Destroy(Tempstr);
Destroy(Output);
Destroy(Path);

return(TRUE);
}

