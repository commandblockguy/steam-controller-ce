# steam-controller-ce
*A program for playing music on a Steam controller from a TI-84+CE*

To use, download the latest release and send it to a TI-84+CE using 
[TI-Connect CE](https://education.ti.com/en/products/computer-software/ti-connect-ce-sw) for Windows/Mac or TiLP 2 for Linux.
On OS 5.5, you will need to use [arTIfiCE](https://yvantt.github.io/arTIfiCE/) to run native programs like this one.
You will also need the USB libraries. These are still under development, and the source can be found here:
https://github.com/CE-Programming/toolchain/tree/usbdrvce.
Since it is time-consuming to build the libraries, you may also download a (probably outdated) [prebuilt version](https://commandblockguy.xyz/downloads/usblibs.8xg).
Lastly, you need to send [the font the program uses](https://commandblockguy.xyz/downloads/Gohufont.8xv) to the calculator.

To generate a song, you will need to use [my fork of SteamControllerSinger](https://github.com/commandblockguy/SteamControllerSinger) with the -e flag.
From there, use convhex or the newer [convbin](https://github.com/mateoconlechuga/convbin/releases) with one of the following commands:
`convhex -a -v -n SONG data.bin SONG.8xv` or `convbin -r -j bin -k 8xv -name SONG -i data.bin -o SONG.8xv`.
TI-Connect should be used to send this output file to the calculator as well.

To build from source, use the [CE toolchain](https://github.com/CE-Programming/toolchain/tree/usbdrvce).
