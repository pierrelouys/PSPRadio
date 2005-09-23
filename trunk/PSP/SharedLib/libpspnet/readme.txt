WiFi test app -- Work in progress

Good sample for figuring out how to load PRXs from flash,
patch them (See "SLIME NOTE"s) and access socket APIs

Loosely based on Nem's HelloWorld and TyRaNiD's kdumper
May be clean up once a more stable SDK appears.

Look for REVIEW and "SLIME NOTE" notes

Work in progress

-----
Files:
    FW10\PSP\GAME\WIFITEST
        -- version for 1.0 firmware

    FW150\PSP\GAME\WIFITEST
    FW150\PSP\GAME\WIFITEST
        -- version for 1.5 Kxploit (program is the same)

    SRC\*.*
        -- source code, with minimal dependencies on headers/sdks

-----
To use:

#1) configure the "Network Settings" for "Infrastructure Mode"
    There must be one configuration
        (multiple configuration selection not implemented)
    Do not use "DHCP"
        (otherwise the Connecting status will stop at 00000003)

#2) Be sure you can connect to that WiFi net using your PC

#3) run the program on the PSP (both 1.0 and 1.50 versions available)

#4) wait for the telnet instructions

#5) on the PC, connect to the PSP using telnet

#6) type in up to 255 characters and see them on the PSP screen

#7) if you get bored, disconnect from telnet and the app will exit

#8) on exit, see file "err.txt" on the memory stick for error logging

Very primitive, but a start

