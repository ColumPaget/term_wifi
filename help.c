#include "help.h"

void DisplayHelp()
{
    printf("version: %s\n", VERSION);
    printf("usage:\n");
    printf("  term_wifi interfaces                                                list interfaces\n");
    printf("  term_wifi scan <interface>                                          scan networks and output details\n");
    printf("  term_wifi add <essid> <address> <netmask> <gateway>  <dns server>   add a config for a network\n");
    printf("  term_wifi add <essid> dhcp                                          add a config for a network using dhcp\n");
    printf("  term_wifi forget <essid>                                            delete (forget) network\n");
    printf("  term_wifi list                                                      list configured networks\n");
    printf("  term_wifi join <interface> <essid>                                  join a configured network\n");
    printf("  term_wifi leave <interface>                                         leave current network\n");
    printf("  term_wifi connect <essid>                                           join configured network with default interface\n");
    printf("  term_wifi qrcode <essid>                                            display qr code for saved network with essid '<essid>'\n");
    printf("  term_wifi qrcode <essid> -viewer <list>                             display qr code for saved network with essid '<essid>' using first viewer program found in comma-separated list '<list>'\n");
    printf("  term_wifi qrcode <essid> -o <path>                                  write qr code for network '<essid>' to PNG file at <path>\n");
    printf("  term_wifi -?                                                        this help\n");
    printf("  term_wifi -h                                                        this help\n");
    printf("  term_wifi -help                                                     this help\n");
    printf("  term_wifi --help                                                    this help\n");
    printf("\n");
    printf("options that apply to connect/interactive mode\n");
    printf("  -i <interface>                                                      interface to use\n");
    printf("  -ap <access point mac address>                                      access point to join (if many for same essid)\n");
    printf("  -k <key>                                                            authentication key for given essid/network)\n");

    printf("\n");
    printf("term-wifi requires iwconfig or iw to be installed, wpa_supplicant for WPA networks, dhcpcd for dhcp support and ifconfig and route for network setup\n");
    printf("if term-wifi is not run as root, then it will try to use sudo, or if that's not installed, su, to run network commands with root permissions\n");
    printf("qr-code export requires qrencode utility to be installed, and also an image viewer\n");
}

