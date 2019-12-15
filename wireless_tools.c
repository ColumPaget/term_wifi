#include "wireless_tools.h"
#include "runcommand.h"

static TNet *WirelessToolsParseCell(const char *Config)
{
char *Token=NULL;
const char *ptr;
TNet *Net;

Net=(TNet *) calloc(1, sizeof(TNet));
ptr=GetToken(Config, "\\S", &Token, 0);
while (ptr)
{
if (strcmp(Token, "Address:")==0) 
{
	ptr=GetToken(ptr, "\\S", &Net->AccessPoint, 0);
}
ptr=GetToken(ptr, "\\S", &Token, 0);
}

Destroy(Token);
return(Net);
}

//  Quality=19/70  Signal level=-91 dBm
static const char *WifiToolsParseQuality(const char *Data, TNet *Net)
{
char *Token=NULL;
const char *ptr;
float Quality, Max;


Quality=(float) strtol(Data, (char **) &ptr, 10);
if (ptr)
{
ptr++;
Max=(float) strtol(ptr, (char **) &ptr, 10);
}

Net->Quality=Quality / Max;

while (isspace(*ptr)) ptr++;
if (strncasecmp(ptr, "Signal level=", 13)==0) 
{
	ptr=GetToken(ptr+13, "\\S", &Token,0);
	Net->dBm=atof(Token);
}

Destroy(Token);

return(ptr);
}

static void WirelessToolsParseBitRates(TNet *Net, const char *Rates)
{
char *Token=NULL, *Tempstr=NULL;
const char *ptr;

ptr=GetToken(Rates, ";", &Token, 0);
while (ptr)
{
	Tempstr=FormatStr(Tempstr, "%0.1f ", atof(Token));
	Net->BitRates=CopyStr(Net->BitRates, Tempstr);
	ptr=GetToken(ptr, ";", &Token, 0);
}

Destroy(Tempstr);
Destroy(Token);
}


static void WirelessToolsGetNetworksParseLine(TNet *Net, const char *Line)
{
char *Key=NULL;
const char *ptr;

	ptr=GetToken(Line, ":|=", &Key, GETTOKEN_MULTI_SEP);
	if (strcmp(Key, "ESSID")==0) 
	{
		Net->ESSID=CopyStr(Net->ESSID, ptr);
		StripQuotes(Net->ESSID);
	}
	else if (strcmp(Key, "Channel")==0) Net->Channel=atoi(ptr);
	else if (strcmp(Key, "Quality")==0) WifiToolsParseQuality(ptr, Net);
	else if (strcmp(Key, "Mode")==0) 
	{
		if (strcasecmp(ptr, "ad-hoc")==0) Net->Flags |= NET_ADHOC;
	}
	else if (strcmp(Key, "IE")==0) 
	{
		if (strstr(ptr, "WPA2")) Net->Flags |=NET_WPA2;
		else if (strstr(ptr, "WPA")) Net->Flags |=NET_WPA1;
	}
	else if (strcmp(Key, "Encryption key")==0) 
	{
		if (strcmp(ptr, "on")==0) Net->Flags |=NET_WEP;
	}
	else if (strcmp(Key, "Bit Rates")==0) WirelessToolsParseBitRates(Net, ptr);

Destroy(Key);
}



int WirelessToolsGetNetworks(TNetDev *Dev, ListNode *Networks)
{
char *Tempstr=NULL, *Output=NULL;
const char *ptr;
TNet *Net=NULL;
int result=FALSE;

NetSetupInterface(Dev, "", "", "", "");
Tempstr=MCopyStr(Tempstr, "iwlist ", Dev->Name, " scan", NULL);
Output=RunCommand(Output, Tempstr, RUNCMD_ROOT);
if (StrValid(Output))
{
ptr=GetToken(Output, "\n", &Tempstr, 0);
while (ptr)
{
	StripLeadingWhitespace(Tempstr);
	StripTrailingWhitespace(Tempstr);

	if (strncmp(Tempstr, "Cell ", 5)==0) 
	{
		//add previously configured net
		if (Net) ListAddNamedItem(Networks, Net->ESSID, Net);

		//load next network
		Net=WirelessToolsParseCell(Tempstr+5);
	}

	if (Net) WirelessToolsGetNetworksParseLine(Net, Tempstr);

	ptr=GetToken(ptr, "\n", &Tempstr, 0);
}

if (Net) ListAddNamedItem(Networks, Net->ESSID, Net);

result=TRUE;
}

Destroy(Tempstr);
Destroy(Output);

return(result);
}


int WirelessToolsSetupInterface(TNetDev *Dev, TNet *Conf)
{
char *Tempstr=NULL, *Cmd=NULL;
int RetVal=FALSE;

Cmd=MCopyStr(Cmd, "iwconfig ", Dev->Name, " essid \"", Conf->ESSID, "\" ", NULL);

if (Conf->Channel > 0) 
{
	Tempstr=FormatStr(Tempstr, " chan '%d' ", Conf->Channel);
	Cmd=CatStr(Cmd, Tempstr);
}

if (StrValid(Conf->Key)) 
{
Cmd=CatStr(Cmd, " restricted ");
if (! (Conf->Flags & (NET_WPA1 | NET_WPA2))) Cmd=MCatStr(Cmd, " key ", Conf->Key, NULL);
}
else Cmd=CatStr(Cmd, " key off ");

Tempstr=RunCommand(Tempstr, Cmd, RUNCMD_ROOT);
if (Tempstr != NULL) RetVal=TRUE;

Destroy(Tempstr);
Destroy(Cmd);

return(RetVal);
}


/*
void WifiSetRate(TNetDev *Dev, int Rate)
{
char *Tempstr=NULL;

if (! Dev) return;
if (Rate < 1) return;

Tempstr=FormatStr(Tempstr,"%s %s rate %dM ",IWCONFIG_PATH,Dev->Name,Rate);

RunCommand(Tempstr, RUNCMD_SU);

Destroy(Tempstr);
}
*/



const char *IwConfigToks[]={"ESSID","Mode","Frequency","Access Point","Cell","Bit Rate","Link Quality","Sensitivity","Retry limit","RTS thr","Fragment thr","Encryption key","Security mode","Signal level","Noise level",NULL};
typedef enum {IWCT_ESSID,IWCT_MODE,IWCT_FREQ,IWCT_AP,IWCT_CELL,IWCT_RATE,IWCT_QUALITY,IWCT_SENS,IWCT_RETRYLIMIT,IWCT_RETRYTHR,IWCT_RTSTHR,IWCT_FRAGTHR,IWCT_ENCKEY,IWCT_SECUREMODE,IWCT_SIGNAL,IWCT_NOISE} EIWConfigToks;


static const char *WirelessToolsGetStatusParseDifficultItems(TNet *Net, const char *Data, const char *Remainder)
{
char *Token=NULL;
const char *ptr;
int val;

ptr=GetToken(Data,"\\S",&Token,0);
while (ptr)
{
		while (isspace(*ptr)) ptr++;
		StripTrailingWhitespace(Token);
		StripLeadingWhitespace(Token);
		val=MatchTokenFromList(Token,IwConfigToks,0);
		ptr=GetToken(ptr,"\\S",&Token,0);
}


if (val==IWCT_ESSID) 
{
ptr=Remainder;
while (isspace(*ptr)) ptr++;
ptr=GetToken(ptr,"\\S",&Net->ESSID,GETTOKEN_QUOTES);
}

Destroy(Token);
return(ptr);
}


static void WirelessToolsGetStatusParseLine(TNet *Net, char *Line)
{
char *Token=NULL;
const char *ptr;

	ptr=Line;
	while (ptr)
	{
		ptr=GetToken(ptr,":|=",&Token,GETTOKEN_MULTI_SEPARATORS|GETTOKEN_QUOTES);
		if (! ptr) break;
		while (isspace(*ptr)) ptr++;
		StripTrailingWhitespace(Token);
		StripLeadingWhitespace(Token);

		switch (MatchTokenFromList(Token, IwConfigToks, 0))
		{
		case IWCT_ESSID:
		ptr=GetToken(ptr,"\\S",&Net->ESSID,0);
		if (
			(! StrValid(Net->ESSID)) ||
			(strcasecmp(Net->ESSID,"not-associated")==0)
			) Net->Flags &= ~NET_ASSOCIATED;
			else Net->Flags |= NET_ASSOCIATED;
		break;
	
		case IWCT_MODE:
		ptr=GetToken(ptr,"\\S",&Token,0);
		if (strcasecmp(Token,"ad-hoc")==0) Net->Flags |= NET_ADHOC;
		break;
		
/*
		case IWCT_RATE:
		ptr=GetToken(ptr,"\\S",&Token,0);
		Net->Rate=CopyStr(Net->Rate,Token);
		ptr=GetToken(ptr,"\\S",&Token,0);
		Net->Rate=CatStr(Net->Rate,Token);
		break;
*/
		
		case IWCT_FREQ:
		ptr=GetToken(ptr,"\\S",&Token,0);
//		Net->Frequency=CopyStr(Net->Frequency, Token);
		ptr=GetToken(ptr,"\\S",&Token,0);
//		Net->Frequency=CatStr(Net->Frequency, Token);
		break;
		
		case IWCT_AP:
		case IWCT_CELL:
		ptr=GetToken(ptr,"\\S",&Net->AccessPoint,0);
		break;
	
		case IWCT_QUALITY:
		ptr=WifiToolsParseQuality(ptr, Net);
		break;


		case -1:
		ptr=WirelessToolsGetStatusParseDifficultItems(Net, Token, ptr);
		break;
		}
	}

Destroy(Token);
}


int WirelessToolsGetStatus(TNetDev *Device, TNet *Net)
{
char *Tempstr=NULL, *Output=NULL, *Line=NULL;
const char *ptr;
int result=FALSE;

if (! Device) return(FALSE);

Tempstr=MCopyStr(Tempstr, "iwconfig ", Device->Name, NULL);
Output=RunCommand(Output, Tempstr, 0);

if (StrValid(Output))
{
ptr=GetToken(Output, "\n", &Line, 0);
while (ptr)
{
	StripTrailingWhitespace(Line);
	
	WirelessToolsGetStatusParseLine(Net, Line);
	ptr=GetToken(ptr, "\n", &Line, 0);
}


if (
        (! StrValid(Net->AccessPoint)) ||
        (strcasecmp(Net->AccessPoint,"not-associated")==0) 
		) Net->Flags &= ~NET_ASSOCIATED;
		else Net->Flags |= NET_ASSOCIATED;

		result=TRUE;
}

Destroy(Line);
Destroy(Output);
Destroy(Tempstr);

return(result);
}


