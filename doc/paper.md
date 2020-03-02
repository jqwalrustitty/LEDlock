% Lock and LED
% Rodger Allen
% February, 2020

# Lock and LED

In principle, the LEDs on a keyboard can be manipulated in such a manner
as to send a message, for example, using Morse Code.

Here will be demonstrated a method that uses a Raspberry Pi Zero,
emulating a standard USB keyboard, which can be used to read an encoded
sequence of LED flashes sent from the Windows host.

## Prerequisites

You will need a Raspberry Pi Zero. The wifi version is nice to have, but
not strictly necessary, although it will make it easier to control.
With or without the wifi, the devce can be deployed unattended.

Grab the source code from where-ever it ends up being thrown.  The
source code can be used to build the *implant* that will go on the host
(using `mingw`), as well as the the *daemon* that will run on the RPi0.
As well as some other goodies, such as docs, config files and install
scripts.

------

# Building the POC

There are enough good tutorials on setting up a RPi, you can go find
your own.  That said, I’ve included some links to setting up gadgetfs.
A lot of this can be grabbed from a bunch of sources, which I have
hopefully had the time to properly reference.

I grabbed whatever was a recent RPi image, *Raspbian Lite*.
I found mounting the image on loopback, and editting a bunch of the
configs *in-situ* and then burning it to the SD card made the initial
configuration much easier.  In the examples, if you are doing it
*in-situ*, then the paths might need to be relative.  Somewhere amongst
these commands will be the answer:

- `losetup (8)`
- `kpartx (8)`
- `mount (8)`
- `dd (1)`

The RPi images have a second `boot` partition that seems to be some sort
of pre-boot-loader, and it's not the same as `/boot` once the device has
booted.  Don't get them mixed up.

## Step-by-Step

0.  [You know the drill]{.smallcaps}

    Interspersed before, amongst, and after these steps are all of your
    usual *sysadmin* steps, like hostnames and ssh keys and passwords.

1.  [Base Configuration]{.smallcaps}

    The kernel needs to be configured to enable gadgetfs.  I don't think
    `libcomposite` is strictly necessary unless you want multiple
    gadgets in the USB interface.

    ```Shell
    echo "dtoverlay=dwc2" | sudo tee -a /boot/config.txt
    echo "dwc2"           | sudo tee -a /etc/modules
    echo "libcomposite"   | sudo tee -a /etc/modules
    ```

    Also insert the `modules-load=dwc2,g_serial` into `/boot/cmdline.txt`.
    The order where it appears in the file is important so consult a howto.

    `dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 root=PARTUUID=7ee80803-02 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait modules-load=dwc2,g_serial quiet init=/usr/lib/raspi-config/init_resize.sh`

    I also touched a `/boot/ssh` file and configured a `/boot/wpa_supplicant.conf`.
    The wpa config file can also be copied to `/etc/wpa_supplicant/`.

    Files touched in this stage (where `/boot` is the other partition):

    - `/boot/cmdline.txt`
    - `/boot/config.txt`
    - `/boot/ssh`
    - `/boot/wpa_supplicant.conf`
    - `/etc/wpa_supplicant/wpa_supplicant.conf`
    - `/etc/modules`

2.  [Wireless Access Point]{.smallcaps}

    I thought it'd be neato if the device could be its own AP, which
    would mean that it would be possible to connect to it in the the
    absence of other wireless options.  This meant installing `dnsmasq`
    and `hostapd`; again, look to howtos and the like.

    The following are files that ended up needing to be configured.  The
    `70-persistent-net.rules` is a gotcha if you move the image to
    another RPi, as it hard-codes the MAC address when creating the
    `uap` device for hostapd.  I ended up not using dnsmasq and just
    used the IPv6 link-local address.

    - `/etc/dnsmasq.conf`
    - `/etc/hostapd/hostapd.conf`
    - `/etc/network/interfaces`
    - `/etc/udev/rules.d/70-persistent-net.rules`
`
3.  [USB HID Emulation]{.smallcaps}

    The files in `/sys/kernel/config/` don't survive a reboot (*Why?*),
    so the `mkHID.sh` scripts create all of the necessary directories
    and config files, and then links the directories appropriately.  Add
    the location of the script to `/etc/rc.local`.

    - `/usr/local/bin/mkHID.sh`
    - `/etc/rc.local`

    If you reboot, and plugin the RPi0 to a host, it should appear as a
    keyboard.

4.  [Daemon Building]{.smallcaps}

    Cross-compiling elsewhere might be possible, but but the RPi can
    compile the infiltrating daemon, `kbdinifil`.  Unpack the source on
    the pi, and run `make kbdinfil`. The binary can get copied to
    `/usr/local/sbin/`.

    ```Shell
    pi$ tar zxf ledlock-9.9.tar.gz
    pi$ cd ledlock-9.9/src/
    pi$ make kbdinfil
    gcc -Wall -Wextra -c -o kbdinfil.o kbdinfil.c
    gcc -Wall -Wextra -c -o encoder.o encoder.c
    gcc -Wall -Wextra -c -o meta.o meta.c
    gcc -Wall -Wextra -c -o crc32.o crc32.c
    gcc -Wall -Wextra -c -o cq.o cq.c
    gcc -Wall -Wextra -c -o filing.o filing.c
    gcc -Wall -Wextra -o kbdinfil kbdinfil.o encoder.o meta.o crc32.o cq.o filing.o
    strip kbdinfil
    ```

    The package also contains a bunch of other directories:

      `doc/`

      : Markdowns to create this document.  Again, `make`.

      `scripts/`

      : Some of the install scripts described below.
        This is still somewhat *fluid* at the time of writing.

      `src/`

      : The code

      `pi/`

      : Some of the config files that can be used as a template
        to build the RPi0.

    As I'm writing this, what is ending up in the `scripts` directory is
    still up for grabs.  Don't build the docs on the RPi.  See below for
    more info.

5.  [Installing `kbdinfil`]{.smallcaps}

    The daemon gets installed into `/usr/local`.  You will also need to
    create a `/var/exfil/` directory, which is where all of the
    exfiltrated data gets stored by default.

    Copy the init script to `/etc/init.d` and run:

    ```Shell
    $ sudo update-rc.d -n kbdinfil defaults
    ```

    When the daemon starts up it tries to open `/dev/hidg0`
    (configurable in the init script), but it might not have been
    created yet, so it backs off and retries until it can.

    Files:

    - `/usr/local/sbin/kbdinfil`
    - `/etc/init.d/kbdinfil`
    - `/etc/rsyslog.d/kbdinfil.conf`
    - `/etc/logrotate.d/kbdinfil`
    - `/var/log/exfil.log`
    - `/var/exfil/`
    - `/var/exfil/kbdinfil-debug.log`

6.  [Building the Implant]{.smallcaps}

    The *Secret Sauce* to compiling the implant is `mingw32`, which will
    make Windows ELF binaries, such as executables.  You can do this on
    the RPi0, or another machine.  Feel free to import the code into a
    Windows environment, such Visual Studio, and compile it natively.

    ```Shell
    pi$ tar zxf ledlock-9.9.tar.gz
    pi$ cd ledlock-9.9/src/
    pi$ make kbdexfil.exe
    i686-w64-mingw32-gcc -c -o kbdexfil.wo kbdexfil.c
    i686-w64-mingw32-gcc -c -o encoder.wo encoder.c
    i686-w64-mingw32-gcc -c -o meta.wo meta.c
    i686-w64-mingw32-gcc -c -o crc32.wo crc32.c
    i686-w64-mingw32-gcc -c -o winkbd.wo winkbd.c
    i686-w64-mingw32-gcc -lws2_32 -o kbdexfil.exe kbdexfil.wo encoder.wo meta.wo crc32.wo winkbd.wo
    i686-w64-mingw32-strip kbdexfil.exe
    ```

    The implant code is horribly bloated.  It was POC, so lots of
    debugging routines are scattered through it, hidden behind a facade
    of `-v`.

    The algorithms should be pretty trivial to code in Assembly, if
    anyone was inclined.  The file-less variation that could be done in
    PowerShell may well be the smallest, even if it was half-import-C
    code utilizing the `InterOp` mechanism.

7.  [Building the Documentation]{.smallcaps}

    For this you are going to need `pandoc`.  And, to build the slides
    you will need the `beamer` templates for \LaTeX [^note-tex] (e.g.,
    `texlive`).  Don't do this on the pi.

    You know the drill by now:

    ```Shell
    build$ tar zxf ledlock-9.9.tar.gz
    build$ cd ledlock-9.9/doc/
    build$ make paper
    pandoc -s -V papersize:a4 -o paper.pdf paper.md
    ```

[^note-tex]: I have waited a *long* time to be able to do that with
  a "`\LaTeX`" in a document for realsies.

------

# Using the Implant

## Running the Implant

``` Shell
Usage: kbdexfil.exe [-h] [-v] [-e -01] [-t timer] <file>
  version: 0.4.8
    -h          this help
    -v          verbose
    -t timer    milliseconds between flashes (1-999)
                (default 2)
    -e[012]     encoding selection
                0 = tydbits (scroll+caps+num) (default)
                1 = trybbles (trybbles+kana)
                2 = unibit (scroll+kana)
    file        filename
```


The only *tweakable* parameters is the `-t timer` setting.  This is the
number of millisconds between flashing the LEDs.  There's not a linear
relationship between the timer and the volume that can be sent.  It can
happily transfer data at a 1 msec delay, but generally not as reliably
as a higher setting.  About the fastest transfer rate seems to be a
little over 10 bytes per second.

Without the `-v` verbose option, the implant will run silently.

Different encodings can be selected with the `-e` parameter.  See below
for more details on the encodings.  The unibit encoding is very flaky.

If you run this with a bog-standard keyboard attached you will see lots
of pretty flashing of the LEDs. Once it has run, it should return the
status of the LEDs back to their original state, but that is a *should*,
not a *will*. If you just run this on a laptop without an external
keyboard, you might not notice anything as laptop keyboards don’t always
have the full complement of LEDs. The RPi we plugin tells the laptop
that it has the full complement of lights, so will flash them invisibly.

*If an LED flashes but noone was around to see it, did it leak data?*

## Example usage

```
> kbdexfil.exe -vvv -e3 -t1 test.txt
filename:       test.txt
mask=10111 delay=1 size=5 crc=0x0xde35e8ec
| 00000 | 10111 | 00000 | 10111 | 00000 | 10111 | 00000 | 10111 | init=10111
>  C  (0x43)    | 00010 | 00100 | 00011 
>  Q  (0x51)    | 00110 | 10000 | 00101 
>  C  (0x43)    | 00010 | 00100 | 00011 
>  Q  (0x51)    | 00110 | 10000 | 00101 
>  _  (0xec)    | 10011 | 00111 | 00000 
>  _  (0xe8)    | 10111 | 00010 | 00100 
>  5  (0x35)    | 00001 | 10101 | 00001 
>  _  (0xde)    | 10110 | 10011 | 00110 
>  _  (0x05)    | 00000 | 00101 | 00001 
>  _  (0000)    | 00100 | 00000 | 00100 
>  _  (0x01)    | 00000 | 00100 | 00001 
>  _  (0x17)    | 00100 | 10001 | 00111 
>     (0x46)    | 00010 | 00101 | 00010 
>     (0x6e)    | 00111 | 00011 | 00110 
>     (0x6f)    | 00011 | 00111 | 00011 
>     (0x72)    | 00111 | 10000 | 00110 
>     (0x64)    | 00011 | 00101 | 00000 
Headers:    12 bytes in 1 seconds @ 12.00 bps
Payload:     5 bytes in 0 seconds @ 5.00 bps
Total:      17 (12+5) bytes in 1 seconds @ 17.00 bps
```

## Running the Listener

```
$ /usr/local/bin/kbdinfil -h
Daemon to receive data from keyboard LEDs
  version: 0.4.8

usage: ./kbdinfil [-hvF1] [-o dir] <device>
    -h          this help
    -F          run in foreground
    -1          receive one frame and exit (runs in foreground)
    -v          verbose (more like debug)
    -vv         more debugging output
    -vvv        holy crap, too much
    -o          output dir (default: /var/exfil)
    device      path to device, e.g., /dev/hidg0
```

It is possible to run it in the foreground, which can be handy for
debugging.  When run as a daemon, the standard IOs (`stdout`, `stderr`,
`stdin`) get `dup2` to `/var/exfil/kbdinfil-debug.log`.  There shouldn't
be too much normally going there, just a log of incoming transmissions.

The verbose, `-v`, option is more debug, than verbose.  When run as a
daemon it ends up in that debug log, otherwise, watch pages.of tydbits
being decoded in real-time.  [NB]{.smallcaps} This can cause the file to
get *HUGE*.

The `-1` run once option, runs the daemon in the foreground to receive
one frame only, then exits.  (Could be the foundation for scripting
send/reply through the keyboard with a second program doing the sending,
and the script managing the back and forth.)

The daemon is configured to use `LOCAL7` as the `syslog` facility.  The
`rsyslog.d` config file directs it log to `/var/log/exfil.log`.

```
22:09:50 kbdinfil: ====== Keyboard LED Listener v0.4.8 ======
22:09:50 kbdinfil: : /usr/local/sbin/kbdinfil
22:09:50 kbdinfil: : PID: 31341
22:09:50 kbdinfil: : Listening on /dev/hidg0
22:09:50 kbdinfil: : Output directory /var/exfil
22:09:50 kbdinfil: : Verbose output to /var/exfil/debug.log
22:11:44 kbdinfil: < mask=10111 delay=2 size=256 crc=0x0x3ce529f1
22:12:04 kbdinfil: > 1582974724-0x3ce529f1-256b-10111.bin [2: 20.00s @ 12.80bps]
22:12:07 kbdinfil: < mask=00111 delay=1 size=256 crc=0x0xc5ca6670
22:12:26 kbdinfil:  CRC mismatch: 0x391dd44d
22:12:26 kbdinfil: > 1582974746-0xc5ca6670-256b-00111.crc [1: 19.01s @ 13.47bps]
```

The incoming files are stored in `/var/exfil`.  They are timestamped and
the *expected* CRC is also added to the name.  The *expected* CRC is
what the implant sent in the frame metadata, as well as the expected
file size.  The encoding ft was used in the transmission is also
incorporated in to the filename. If the file fails to be received
properly, viz it fails the CRC check, then the file gets ".crc" as an
extension, otherwise they are called `.bin`.

| Epoch         | CRC           | Size    | Encoding  | Extension |
|---------------|---------------|---------|-----------|-----------|
| `1582974724`  | `0x3ce529f1`  | `256b`  | `10111`   | `.bin`    |
| `1582974746`  | `0xc5ca6670`  | `256b`  | `00111`   | `.crc`    |

------

# Some Theory

## Keyboards

Mostly, we think of a keyboard as being a one-way input mechanism, that
is, we type and the data goes from the keyboard to the computer. There
is a very limited amount of traffic that can go the other way, that is,
from the computer to the keyboard. You can see it yourself - install
USBpcap and wireshark and sniff that USB traffic.  This traffic is very
small, essentially just one value that is used as a mask for which LEDs
to have on or off.

Stephenson and others have suggested using the LEDs to flash out Morse
Code, but this would be slow, would still require an encoding step for
binary files (e.g., Base-64 encoding), and ultimately would rely on a
good timer.

The problem here is that the messages to the keyboard are essentially
asynchronous, that is, there is no guarantee that the flashes will be
sent when they are needed to be sent. It might be possible to have more
control over the USB traffic if coded for at lower (device) level - but
the code for this is written such that it requires no special or
elevated privileges. It runs in user-land and utilises the
`keybd_event()` function from `user32.dll`.

## Encodings

If the three lights themselves could be consider to be "bits", then what
would be needed to encode a byte (8 bits) into groups of 3 bits? Maybe
3-3-2, and with the third flash we ignore one of the LEDs.

The first problem we encounter is that if the state of the LEDs doesn’t
change then the host OS doesn’t send an update. If the bits are
something like `0b10010011`, and broken down into `0b100`, `0b100` and
`0b11`, then there wouldn’t be an update sent for the second triple.
The only way the receiver would know that there was a second `0b100`
would be if the timings all lined up correctly.

The solution opted for here was to break up the byte into 4 chunks of
2-bits.  One of the LEDs is reserved, and each time a new tydbit is to
be sent, that reserved LED is switched on and off alternately. We can
transmit "C", which is `0x53` or `0b01010001`, where each of the first
two 2-bit tydbits is `0b01`, by forcing a state change with one of the
LEDs even when the bits to be sent have not changed.

| Tydbits       |
|:-------------:|
| `C` or `0x43` |
| `01010001`    |
| `01 01 00 01` |
|               |
|               |
| `Q` or `0x51` |
| `01000011`    |
| `01 00 00 11` |

Table: Encoding `CQ` as Tydbits

| Trybble | Tydbits | Encoding  | Wire  |
|:-------:|:-------:|:---------:|:-----:|
| `000`   | `01`    | `o o *`   | `1`   |
| `100`   | `00`    | `* o o`   | `4`   |
| `000`   | `00`    | `o o o`   | `0`   |
| `100`   | `11`    | `* * *`   | `7`   |
| `000`   | `01`    | `o o *`   | `1`   |
| `100`   | `01`    | `* o *`   | `5`   |
| `000`   | `00`    | `o o o`   | `0`   |
| `100`   | `01`    | `* o *`   | `5`   |

Table: Encoding "CQ" as a sequence of flashes

Since the alternating-trybble is being discarded, it doesn't matter for
the encoding which one it is, so long as they alternate.

| Trybble | Tydbits | Encoding  | Wire  |
|:-------:|:-------:|:---------:|:-----:|
| `100`   | `01`    | `* o *`   | `5`   |
| `000`   | `00`    | `o o o`   | `0`   |
| `100`   | `00`    | `* o o`   | `4`   |
| `000`   | `11`    | `o * *`   | `3`   |
| `100`   | `01`    | `* o *`   | `5`   |
| `000`   | `01`    | `o o *`   | `1`   |
| `100`   | `00`    | `* o o`   | `4`   |
| `000`   | `01`    | `o o *`   | `1`   |

Table: Encoding "CQ" as a sequence of flashes - different Trybblyes

Decoding is just the same thing in reverse. Take four sequential sets of
flashes (actually, when you look at the USB, it is just a single int
value that represents the mask of the LEDs). Drop the first "bit" from
each, and then stitch together the four 2-bit tydbits to make a byte.

The alternating trybble also acts as a tiny bit of error correction,
since the hi-bit is set last, so even if there is a race condition where
the flashes are only partially sent, the listener will wait until the
hi-bit alternates before accepting the tydbit.  This is also why there
is the timer delay in the implant.

The error correction is not hugely robust.  Longer delays between
flashes can help to give the OS time to send the full state change
before the next one overwrites it.  As there is no feedback mechanism
at this point from the listener to the implant, there is no way to
signal that frames need to be resent.

### The Kana Bit

Also addressable on some Windows configurations is the Kana bit (and not
sure why some do, and some don't, recognize it).  This will not appear
to light up unless you have a physical keyboard with the LED.  It is the
fifth bit in the LED bitmask.

Ideally, this extra bit would make for a good alternating parity bit,
but it seems that Windows won't send the change to the Kana bit unless
the "normal" LED states are changed, so the Scroll Lock key is still
used as the parity bit.

The encoding looks roughly like `10*11`, where scroll lock is marked
with a `*`, the `0` is a reserved empty bit, and the the `1`s are
flashed.  A byte decomposes into three of these frames, where the third
loses one bit.  This bit could have been used to stream the data
bitwise, but byte boundaries make things easier (and that extra bit
could be reused for error correcting).

Given that the frame sends three bits instead of two per flash, there is
some improvement in the transfer speed, although the rate is better
modelled with a ratio of three flashes per byte compared to four.
Furthermore, consideration needs to be taken to the average number of
state changes between the frames, which is directly expressed in the
number of keypresses that need to occur.  For the original tydbit,
there is one keypress for the parity, and an average of one of the two
tydbits needs to be changed, so there are 8 keypresses per byte.  The
kana bit encoding has one keypress per parity, plus one-and-a-half
keypresses per frame, averaging out to 7 1/2 keypresses per byte.  The
kana encoding also wastes one bit.  Oddly enough, the kana encoding
seems to be more reliable than the tydbit encoding, with a lower rate of
uncorrected errors.

A unibit encoding is also possible, which may make things stealthier.
For instance, most laptops don't have a scroll lock key, sometimes a
numlock, and, of course, never a kana key.  The unibit encoding is
correspondigly much slower, requiring eight frames per byte, and is,
indeed, about half as slow as the tydbit encoding, which requires four
frames per byte.  The unibt encoding is also very flaky, having a very
high uncorrected error rate.  The unibit encoding in the implant uses
scroll for parity and kana for the data bit.

## Metadata

The transmission starts by sending a repeating signal of the *mask* of
the encoding to be used.  The four bytes "`CQCQ`" are then used as a
sanity check to validate that the chosen encoding is correct.  This is
used by Morse Operators to signal "Attention".

Some metadata is sent before the payload data:

  - CRC32 checksum (4 bytes)
  - size of file (2 bytes)
  - delay (1 byte)
  - mask (1 byte)
    + **Total** 8 bytes

The *CRC* is used to validate the payload got sent correctly.  The
*delay* is the timer used by the implant; this is only for reference and
padding, and might get changed.  The *mask* is resent as final check
that the metadata has been all decoded correctly.

The maximum size of the transfer is 65535 bytes (16-bit), but even at
the fastest transfer speed that is going to take almost two hours to
send.  Better to break up the data into smaller chunks first before
feeding it to the implant.

| Segment |Size   | Example             | Note
|---------|:-----:|---------------------|----
| Magic   | 4     | `43 51 43 51`       | `CQCQ`
| CRC32   | 4     | `EC E8 35 DE`       | checksum of payload
| Size    | 2     | `05 00`             | 5 bytes
| Delay   | 1     | `01`                | 5 bytes
| Mask    | 1     | `07`                | tydbit encoding
| Payload | 1-255 | ...                 | . . . . .

| Tydbits                                   |
|:------------------------------------------|
| `1 4 0 7 1 5 0 5 1 4 0 7 1 5 0 5`         |
| `3 6 3 4 3 6 2 4 0 7 1 5 3 5 3 6`         |
| `0 4 1 5 0 4 0 4`                         |
| `0 4 0 5`                                 |
| `0 4 1 7`                                 |
| `1 4 1 6 1 6 3 6 1 6 3 7 1 7 0 6 1 6 1 4` |

## Some References

- [Neal Stephenson, Cryptonomicon]()
- [HID Usage Tables 1.12](https://usb.org/hid)
- [Leaking data using DIY USB HID device](https://techblog.vsza.hu/posts/Leaking_data_using_DIY_USB_HID_device.html)
- [Extracting data with keyboard emulation](https://hackaday.com/2012/10/30/extracting-data-with-keyboard-emulation)
- [Reverse Keyboard LED Channel POC](https://forums.hak5.org/topic/25578-version-1-working-reverse-keyboard-led-channel-poc/)
- [CTRL-ALT-LED: Leaking Data from Air-Gapped Computers via Keyboard LEDs](https://arxiv.org/abs/1907.05851)
- [USB Gadgets](https://github.com/ckuethe/usbarmory/wiki/USB-Gadgets)
- [Composite USB Gadgets on the Raspberry Pi Zero](https://www.isticktoit.net/?p=1383)
- [Linux USB gadget configured through configfs](https://www.kernel.org/doc/Documentation/usb/gadget_configfs.txt)

## Some Reference Data

| KEY         | USB         | `keybd_event()`     |
|-------------|-------------|---------------------|
| NUM_LOCK    | 0x01 0b001  | `VK_NUMLOCK` - 0x90 |
| CAPS_LOCK   | 0x02 0b010  | `VK_CAPITAL` - 0x14 |
| SCROLL_LOCK | 0x04 0b100  | `VK_SCROLL` - 0x91  |

<!--
# File        : doc/paper.md
# Copyright   : (c) Rodger Allen 2020
# Licence     : CC BY-SA (2020)
# ------------------------------------------------------------------------------
# vi: et ts=2 ai tw=72
-->
