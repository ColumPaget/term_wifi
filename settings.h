
#ifndef WMAN_SETTINGS_H
#define WMAN_SETTINGS_H

#include "common.h"

#define DEFAULT_PIDS_DIR "/var/run/"
#define DEFAULT_CONFIG_FILE "~/.term_wifi.conf"

#define FLAG_DAEMON 1
#define FLAG_INTERACTIVE 2

typedef struct
{
int Flags;
char *PidsDir;
char *ConfigFile;
char *RootPassword;
char *WPASupplicantSock;
} TSettings;

extern TSettings Settings;

void SettingsInit();
void SettingsPostProcess();
ListNode *SettingsLoadNets(const char *ESSID);
void SettingsSaveNet(TNet *Net);
void SettingsConfigureNet(TNet *Net);
void SettingsForgetNet(const char *Essid);

#endif
