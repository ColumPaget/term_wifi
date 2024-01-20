# SYNOPSIS

term_wifi is a command-line or terminal-menu based wifi setup tool intended for machines that lack a graphical environment. Initially only IP4 is supported. This program is a front-end to the iw, iwconfig and wpa_supplicant tools, and should automatically find them if they're in /bin, /usr/bin, /sbin or /usr/sbin.

Most operations that involve wifi will have to be performed as the root user. Either run this program as root or, when running as another user, the program will prompt you for your root password. This is not stored in any config file.

The program can be used in command-line mode, or in the default 'interactive' mode that displays a simple terminal user interface.

# INSTALL

```
./configure
make
```

This should produce an executable called 'term_wifi' that you can copy to a bin directory.


# REQUIRED 'HELPER' PROGRAMS

term_wifi requires iwconfig or iw to be installed for wifi scanning/setup, wpa_supplicant for WPA networks, dhcpcd for dhcp support and ifconfig and route for network setup.

if term_wifi is not run as root, then it will try to use sudo, or if that's not installed, su, to run network commands with root permissions.

qr-code export requires qrencode utility to be installed, and also an image viewer. term_wifi searches for a default list of image viewers, but this can be overridden with the `-viewer` option.



# INTERACTIVE (USER INTERFACE) MODE

if term_wifi is run without any arguments the user will be presented with a simple terminal user interface. You can use the '-i' option to show the tui for a given wifi device like so:

```
	term_wifi -i wlan1
```


# USAGE: Command line mode

To view available wifi networks:

```
	term_wifi scan <interface>
```

To configure a wifi network for use:

```
	term_wifi add <essid> dhcp
	term_wifi add <essid> <ip4 address> <netmask> <gateway> <dns server>
```

You can add a network to the stored config with either of these commands. These two commands also accept `-k <key>` and `-ap <access point>` options. the `-k` option allows specifying the wifi key/password. If it's not specified with this option then the user will be prompted for it. The `-ap` option allows specifiying an access-point mac address, in order to distinguish different networks with the same essid.

Once a setup is added to known networks you can join to it with:

```
	term_wifi join <interface> <essid>
```

'connect' does the same thing as 'join' but uses the default interface:

```
	term_wifi connect <essid>
```

If a wifi password is not known at this stage, you will be prompted for one. If the current user is not root you will likely be prompted for the system root password too.

To leave a wifi network that you're currently connected to use:

```
	term_wifi leave <interface>
```

To view available wifi interfaces:

```
	term_wifi interfaces
```

To view currently configured wifi networks:

```
	term_wifi list
```

To 'forget' (delete) a configured network

```
	term_wifi forget <essid>
```



# COMMAND LINE REFERENCE

```
  term_wifi interfaces                                                list interfaces
  term_wifi scan <interface>                                          scan networks and output details
  term_wifi add <essid> <address> <netmask> <gateway>  <dns server>   add a config for a network
  term_wifi add <essid> dhcp                                          add a config for a network using dhcp
  term_wifi forget <essid>                                            delete (forget) network
  term_wifi list                                                      list configured networks
  term_wifi join <interface> <essid>                                  join a configured network
  term_wifi leave <interface>                                         leave current network
  term_wifi connect <essid>                                           join configured network with default interface
  term_wifi qrcode <essid>                                            display qr code for saved network with essid '<essid>'
  term_wifi qrcode <essid> -viewer <list>                             display qr code for saved network with essid '<essid>' using first viewer program found in comma-separated list '<list>'
  term_wifi qrcode <essid> -o <path>                                  write qr code for network '<essid>' to PNG file at <path>
  term_wifi -?                                                        this help
  term_wifi -h                                                        this help
  term_wifi -help                                                     this help
  term_wifi --help                                                    this help

options that apply to connect/interactive mode
  -i <interface>                                                      interface to use
  -ap <access point mac address>                                      access point to join (if many for same essid)
  -k <key>                                                            authentication key for given essid/network)
```

you can use `-i <interface>` to specify and interface to use for the 'connect', 'scan' and tui commands.


QR CODES
========

the command-line 'qrcode' action will produce a qrcode PNG and attempt to find an image-viewer to display it. You can specify an image-viewer command using the '-viewer' option. Failing that, term_wifi has an internal list of image-viewer programs to try. After all the image viewers, there are two options that output sixel images to the terminal, provided your terminal supports sixel image display. Finally, if nothing matching is found, the ultimate fallback is to use qrencodes 'ANSI256' display message to create a giant QR code with ANSI graphics.


