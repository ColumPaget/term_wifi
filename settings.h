
#ifndef WMAN_SETTINGS_H
#define WMAN_SETTINGS_H

#include "common.h"

#define DEFAULT_PIDS_DIR "/var/run/"
#define DEFAULT_CONFIG_FILE "~/.config/term_wifi/term_wifi.conf:~/.term_wifi.conf"
#define DEFAULT_IMPORT_LOG "~/.config/term_wifi/sync_imports.dat"

#define FLAG_DAEMON 1
#define FLAG_INTERACTIVE 2

typedef struct
{
int Flags;
char *PidsDir;
char *ConfigFile;
char *RootPassword;
char *WPASupplicantSock;
char *OutputPath;
char *ImageViewer;
char *ExportPath;
char *SyncImportLog;
} TSettings;

extern TSettings Settings;

void SettingsInit();
void SettingsPostProcess();
int SettingsAddNets(ListNode *Nets, const char *Path, const char *Match);
ListNode *SettingsLoadNets(const char *Path, const char *ESSID);
int SettingsSaveNets(const char *Path, ListNode *List);
void SettingsSaveNet(TNet *Net);
void SettingsConfigureNet(TNet *Net);
void SettingsForgetNet(const char *Essid);

#endif
