# gmouse
Mouse Systems Mouse emulator

"gmouse.c" uses SDL for capturing a mouse, then outputs the movements and clicks over serial port to another (old) computer that can load a Mouse Systems Mouse mouse driver and use that mouse in legacy applications. Use a null modem serial cable for connecting, and use a serial adapter if your (new) computer does not have a serial port.

Motivation for doing this was that USB mice were not supported on old computers anymore and adapters are generally available for PS/2 only, and new (comfortable) USB mice are not PS/2 compatible. The application can be placed to a Raspberry Zero for example, and ran at boot, which would turn it into a very small adapter.

I tried to create support for Microsoft Mouse as well, which would allow 40Hz rate instead of 20Hz, but for some reason it fails even when I take the DTR/DSR lines into account. The work in progress is in "mmouse.c".
