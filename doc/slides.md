% Lock and LED
% Rodger Allen
% February, 2020


# The Story

## The Story Teller

- That's me
- Just your average working pentester with a bit of an obsession for data privacy
  - Seriously, an *obsession*[^pocorgtfo19]
- In my workplace, I sit right next to a team of people tasked with DLP responsibilities
  - I call them my **Muses**
  - Recently, they implemented `umass`[^umass] device controls
- I also like to read a bit
  - Here's a story you might like

[^umass]: `umass(4)` - USB Mass Storage Devices, e.g., external disk drives

## The Novel

![Neal Stephenson, *Cryptonomicon* (1999)](images/Cryptonomicon-1stEd.jpg){ width=50% height=50% }

- Yeah, I'm going to read you a bit from page 827.
- Thank you for your patience

## The Revelation

- **WE CAN WRITE TO KEYBOARDS!!!**

![](images/keyboard-dell.jpg)

- These humble devices aren't just one way - from our fingers to the keys to the host.
  We can write back to them from the host!

## The Challenges

1.  **Locked-down `umass` on Windows host**
2.  **Flash keyboard LEDs from user-land**
       i. no extra privileges
      ii. no device drivers
3.  **Better *protocol* than Morse**
4.  **Device to emulate USB keyboard**


## The More Sensible Abstract

In principle, the LEDs on a keyboard can be manipulated in such a manner
as to send a message, for example, using Morse Code.

Here will be demonstrated a method that uses a Raspberry Pi Zero,
emulating a standard USB keyboard, to read an encoded sequence of LED
flashes sent from a Windows host, covertly bypassing USB mass storage
security controls.

Cookies may, or may not, be served after the demonstration.


<!-- *************************************************************** -->

# The History

## The Rubber Ducky - 2005?

- Commercial tools available from Hak5, first appeared years ago

![Just a USB stick?](images/usb-flash-drive.jpg){ width=25% height=25% }

- Appears to the host as **two** devices:
  - a keyboard, and
  - a `umass` device

. . .

1.  **Locked-down `umass` on Windows host**
      - Writes are done to the `umass` device
      - *Bash Bunny*?  Similar issues.

<!--
- Hak5 also make the *Bash Bunny* which can emulate other USB devices,
  such as ethernet or serial adaptors.
-->

## The Air-Gap - July 2019

- *CTRL-ALT-LED: Leaking Data from Air-Gapped Computers via Keyboard LEDs*[^ctrl-alt-led]
  1. Film the keyboard with a camera
  2. Flash the LEDs as fast as possible
  3. Decode the data offline from stream of images

. . .

2.  **Flash keyboard LEDs from user-land, no extra privileges, no device drivers**
      - Flashing the LEDs that fast requires device level access

## The Guru of Exfiltration

- Mordechai Guri - Air-Gap @ Ben-Gurion University [^air-gap]
  - *LED-it-GO: Leaking (A Lot of) Data from Air-Gapped Computers via the (Small) Hard Drive LED*
  - *DiskFiltration: Acoustic Data Exfiltration from Speakerless Air-Gapped Computers via Covert Hard-Drive Noise*
  - *Mosquito: Covert Ultrasonic Transmissions between Two Air-Gapped Computers using Speaker-to-Speaker Communication*
  - *Bitwhisper: Covert signaling channel between air-gapped computers using thermal manipulations*
  - *Fansmitter: Acoustic Data Exfiltration from (Speakerless) Air-Gapped Computers*
  <!-- - *PowerHammer: Exfiltrating Data from Air-Gapped Computers through Power Line* -->
  - *Brightness: Leaking Sensitive Data from Air-Gapped Workstations via Screen Brightness*

## The Teensy USBPWN - 2012

- András Veres-Szentkirályi's *USBPWN*[^usb-pwn] uses a **Teensy**
  + a signal processing take on the flashing of the LEDs
  + synchronous, which is great for data integrity
  + pushes file-less VB code to the host

![Counterfeit Teensy[^teensy]](images/teensy-counterfeit_3.jpg){ width=25% height=25% }

. . .

4.  **Need a device to emulate USB keyboard**
      - Unfortunately my soldering skills aren't real flash


<!-- *************************************************************** -->

# The Theory

## The Morse Code Tree [^morse-tree]

- Morse code has a few problems. It is inefficient and dependant on timings.
  + We're not going to use Morse, but here's a fun representation

![](images/morse-tree.png)

## The Morse Code Code

```C
#include <stdio.h>
#include <string.h>
char buf[99], tree[] = " ETIANMSURWDKGOHVF L PJBXCYZQ  ";
int main() {
    while (scanf("%s", buf)) {
        int n, t=0;
        for(n=0; buf[n]!=0; n++)
            t = 2*t + 1 + (buf[n]&1);
        putchar(tree[t]);
    }
}
```
```Shell
$ ./morse
-- --- .-. ... . -.-. --- -.. .
MORSECODE
```

## The Function

```C
// Synthesizes a keystroke.
void keybd_event(
  BYTE      bVk,  // VK_SCROLL, VK_CAPITAL, VK_NUMLOCK
  BYTE      bScan,
  DWORD     dwFlags,
  ULONG_PTR dwExtraInfo
);
```

- Handy little function buried away in `User32.dll`
  + Therefore **user-land**
  + Invoke with `VK_SNAPSHOT` for screen grabbing
- Some caveats and warnings:
  + Calls are asynchronous, so requires a delay
  + Other race conditions (3 LEDs = 6+ function calls)
  + User interaction can alter state

## The Titbit about Tydbits

| #-bits  | Shape         | Names             |
|:-------:|:--------------|:------------------|
| 8       | `0b11111111`  | **byte** octet    |
| 4       | `0b1111`      | **nybble** nibble |
| 3       | `0b111`       | **trybble** tribit triad triade tribble |
| 2       | `0b11`        | **tydbit** dibit crumb quad quarter taste tayste tidbit lick lyck semi-nibble |
| 1       | `0b1`         | **bit** unibit    |
Table: Different names for collections of bits

## The Trouble with Trybbles

- A **Trybble** (3-bits) would seem to be the natural encoding for **three LEDs**.  However...
  + The OS keeps state on the LEDs, so when you try to send
    + **`101`** followed by **`101`**,
  + the OS doesn't send the second set of flashes.

. . .

- Force the change of state by alternating a single trybble.
  - Reserve one LED for alternating on and off.  This is the Trybble.
  - Break a byte into 4 **Tydbits** (2-bits).
  - The alternating trybbles are prepended to each tydbit in turn.
  - A byte is then sent as four sets of the three flashing LEDs.

## The Encoding Example - C

| Tydbits       |
|:-------------:|
| `01000011`    |
| `01 00 00 11` |
Table: The character **`C`** has the ASCII code `0x43` or `0b01000011`

| Trybble | Tydbits | Encoding  | Wire  |
|:-------:|:-------:|:---------:|:-----:|
| `000`   | `01`    | `o o *`   | `1`   |
| `100`   | `00`    | `* o o`   | `4`   |
| `000`   | `00`    | `o o o`   | `0`   |
| `100`   | `11`    | `* * *`   | `7`   |
Table: Alternating Trybbles are added to each Tydbit

## The Encoding Example - Q

| Tydbits       |
|:-------------:|
| `01010001`    |
| `01 01 00 01` |
Table: The character **`Q`** has the ASCII code `0x51` or `0b01010001`

| Trybble | Tydbits | Encoding  | Wire  |
|:-------:|:-------:|:---------:|:-----:|
| `000`   | `01`    | `o o *`   | `1`   |
| `100`   | `01`    | `* o *`   | `5`   |
| `000`   | `00`    | `o o o`   | `0`   |
| `100`   | `01`    | `* o *`   | `5`   |
Table: Alternating Trybbles are added to each Tydbit

## The Encoded Result

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

## The Encoded Result - trybbled

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
Table: Encoding "CQ" as a sequence of flashes

## The Payload Payoff

| Segment |Size   | Example             | Note
|---------|:-----:|---------------------|----
| Magic   | 4     | `43 51 43 51`       | `CQCQ`[^cqcq]
| Size    | 2     | `05 00`             | 5 bytes
| CRC32   | 4     | `EC E8 35 DE`       | checksum of payload
| Payload | 1-255 | ...                 | . . . . .

-----------------------------------
`1 4 0 7 1 5 0 5 1 4 0 7 1 5 0 5`
`0 4 1 5 0 4 0 4`
`3 6 3 4 3 6 2 4 0 7 1 5 3 5 3 6`
`1 4 1 6 1 6 3 6 1 6 3 7 1 7 0 6 1 6 1 4`
-----------------------------------

[^cqcq]: `CQ` is used by Morse operators to mean "Attention":
`-.-. --.-`

## The Implant

```Shell
usage: kbdexfil.exe [-h] [-v] [-t timer] <file>
    -h          this help
    -v          verbose output
    -t timer    milliseconds between flashes (1-999)
                default 20
    file        filename
```

<!-- *************************************************************** -->

# The Pi

## The Raspberry Pi Zero [^rpi0]

![](images/rpi0w.jpg)

## The Meaning of "OTG"

- The Raspberry Pi Zero has 2 USB microB ports
  + one is dedicated power (the one on the right in the picture)
  + the other is OTG, or *On-The-Go*
    + OTG can also take power
- OTG devices can be either:
  + the master device as the host, or
  + the slave device as the peripheral
- OTG allows the Raspberry Pi Zero to act as a peripheral.
  + Like a USB keyboard.
  + Or a host.  But not *both* at the same time.
  + Emulate more than one peripheral device at a time
    + DIY Bash Bunny.

## The Gadget Filesystem

- GadgetFS is a kernel module used to emulate USB devices
```Shell
cd /sys/kernel/config/usb_gadget/g1
echo 0x413c > idVendor    # Dell Computer Corp.
echo 0x2106 > idProduct   # Dell QuietKey Keyboard
echo 0x0101 > bcdDevice   # v1.0.0
echo 0x0200 > bcdUSB      # USB2
echo "fedcba9876543210"     > strings/0x409/serialnumber
echo "Xell Computer Corp."  > strings/0x409/manufacturer
echo "Dxll QuietKey Keyboard" > strings/0x409/product
```

## The Emulation

- This is how the Pi appears to Windows.
![what windows sees](images/ControlPanel-Devices-C.png){ height=66% }


<!-- *************************************************************** -->

# The Demo

## The Obvious Questions {.allowframebreaks .allowdisplaybreaks}

1.  How can this be detected? I hope this is your first question.
    + Generic malware detection
      - Binary signature
      - Odd calls to `keybd_event()`
      - PowerShell and VB
    + Odd USB behaviour
      - voltages
      - RPi boot sequence, esp if powered over OTG
      - initialisation handshake? (worth pondering)

2.  I want one.  How do I make one, and where can I get the code?
    + Ask me offline[^walrus]

3.  What next? (or, How can I help?)
    + File-less PowerShell implant (cut'n'paste buffer exfil)
    + Device layer implant (broaden the bandwidth)
    + Build a better chassis (camouflage, key-logger, Teensy)

4.  What is the throughput?
    + $bps = 1000/4t$ ($t$ is milliseconds, and $4$ tydbits to the byte)
    + A $t$ value of 50 should give approx $5bps$

5.  Which LED is the trybble, and which are the tydbits?
    + Scroll Lock is the trybble (`0b100`)
    + Caps Lock is the hi-bit of the tydbit (`0b10`)
    * Num Lock is the lo-bit of the tydbit (`0b01`)

8.  How long until external keyboards are banned?  At least our mice are safe.
    They're safe, aren't they?  Surely it's not possible to leak data through a mouse?!
    + *Mice and Covert Channels* [^arxiv-mice]

9.  We were promised a "Magic Carpet".
    + Yes, you were


<!-- *************************************************************** -->

# The Magic Carpet

## The Standard

***The Universal Serial Bus HID Usage Tables*** (2004)[^usb-hud]

. . .

+ Produced by the USB Implementers' Forums, who create the standards for all things USB.
+ This document defines all the on-the-wire details of the USB protocol
  for *Human Interface Devices*.
  + It is the source for a bunch of the obtuse hex values used by `gadgetfs`
    to emulate the keyboard.

. . .

- Tucked away in section 5 ...

## Yes, there is a Magic Carpet!

```
5 Simulation Controls Page (0x02)
Usage ID   Usage Name
0B         Magic Carpet Simulation Device

5.7  Miscellaneous Simulation Devices
Magic Carpet Simulation Device

CA - Allows a device to be generally classified as one
that uses the standard control of a magic carpet.
This control is a bar, grasped by both hands, that
controls the Yaw, Pitch and Roll of the carpet.
```

## It goes on

```
The bar, at which the pilot sits, may be pushed forward or pulled back
to cause the carpet to dive or rise, respectively. In the zero position, the carpet is
in level flight. Pushing forward on the bar causes the carpet to nose down
and generates negative values. Pulling back on the bar causes the carpet to nose
up and generates positive values.

Turning the bar turns the carpet. In the zero position, the carpet travels
straight ahead. Pulling back on the right side turns the carpet to the right
and generates positive values. Pulling back on the left side turns the carpet to the
left and generates negative values.

Rotating the bar rolls the carpet. In the zero position, the carpet travels level.
Rotating the bar in a clockwise direction rolls the carpet to the right and
generates positive values. Rotating the bar in the counterclockwise direction
rolls the carpet to the left and generates negative values.
```

## Does it go far enough?

- Alas, there is no clear way to talk to the magic carpet controller from the host.
  + A very cursory search suggested no one sells them, either.

. . .

- Plenty more LED indicators to play with in Section 11:
  + Kana and other non-US-ASCII keyboard indicators
    + Post-2004 keyboards
  + Audio indicators and telephony devices

. . .

- And, there are other interesting devices in the standard, too
  + Game controllers
  + Home security controls
  + Medical instruments
  + and, so, so much more

<!-- *************************************************************** -->

# The End



## The Coda {.allowframebreaks}

Title

: Lock and LED

Author

: Rodger Allen

Email

: rodger.allen@protonmail.com

Licence

: CC BY-SA (2020), BSD3 (2018-2020)

Thanks

: DS & SH & CH

<!--
```
=========1=========2=========3=========4=========5======
```
-->

![](images/keyboard-assembly.jpg)


[^usb-hud]: https://usb.org/hid
[^usb-pwn]: https://techblog.vsza.hu/posts/Leaking_data_using_DIY_USB_HID_device.html
[^teensy]: https://www.pjrc.com/teensy/
[^air-gap]: https://cyber.bgu.ac.il/air-gap/
[^morse-tree]: https://apfelmus.nfshost.com/articles/fun-with-morse-code.html
[^walrus]: https://github.com/jqwalrustitty/
[^rpi0]: https://www.raspberrypi.org/products/raspberry-pi-zero-w/
[^pocorgtfo19]: https://www.alchemistowl.org/pocorgtfo/pocorgtfo19.pdf
[^arxiv-mice]: https://arxiv.org/abs/1911.01349
[^ctrl-alt-led]: https://arxiv.org/abs/1907.05851

<!-- =============================================================== -->
<!--
## The Junk at the end of the draft that needs deleting

```
=========1=========2=========3=========4=========5======
=========1 -V aspectratio:54 3=========4=========5======X
=========1 -V aspectratio:43 3=========4=========5=======X
=========1 -V aspectratio:141 =========4=========5=========6=====X

pandoc -s -t beamer \
    -V theme:Szeged \
    -V colortheme:crane \
    -V fonttheme:structurebold \
    -V aspectratio:43 \
    -o slides.pdf slides.md
```
->
<!-- *************************************************************** -->
<!--
-->
<!--
```
            __________________________________
   NUM ____/
                               _______________
  CAPS _______________________/
                ______            ______
SCROLL ________/      \__________/      \_____

                  01                11
```
-->
<!-- *************************************************************** -->
<!--
vi: et ts=2 ai ts=72
-->
