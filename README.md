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


# INTERACTIVE (USER INTERFACE) MODE

if term_wifi is run without any arguments the user will be presented with a simple terminal user interface. You can use the '-i' option to show the tui for a given wifi device like so:

```
	term_wifi -i wlan1
```


# OPTIONS

you can use `-i <interface>` to specify and interface to use for the 'connect', 'scan' and tui commands.

