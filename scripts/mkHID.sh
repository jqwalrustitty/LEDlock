#!/bin/bash
# derived from https://github.com/girst/hardpass-sendHID
#
# TODO:
# * multibyte LED bitmasks???
# * to use parameters captured from real keyboards
#

modprobe libcomposite

cd /sys/kernel/config/usb_gadget/
mkdir -p g1
cd g1

echo 0x413c     > idVendor                      # Dell Computer Corp.
echo 0x2106     > idProduct                     # Dell QuietKey Keyboard
echo 0x0101     > bcdDevice                     # v1.0.0
echo 0x0200     > bcdUSB                        # USB2

mkdir -p strings/0x409
echo "fedcba9876543210"         > strings/0x409/serialnumber
echo "Xell Computer Corp."      > strings/0x409/manufacturer 
echo "Dxll QuietKey Keyboard"   > strings/0x409/product

N="usb0"
mkdir -p functions/hid.$N
echo 1          > functions/hid.$N/protocol       # Keyboard
echo 1          > functions/hid.$N/subclass       # Boot Interface
echo 8          > functions/hid.$N/report_length

# --------------------------------------

# XXX: 7-bit LEDs + 1-bit padding
echo -ne \\05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x01\\x95\\x07\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x07\\x91\\x02\\x95\\x01\\x75\\x01\\x91\\x01\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 >  functions/hid.usb0/report_desc

# XXX: 77-bit LEDS (0x4d) - ALL of them  + 3-bit padding
#echo -ne \\05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x01\\x95\\x4d\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x4d\\x92\\x02\\x95\\x03\\x75\\x01\\x91\\x01\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 >  functions/hid.usb0/report_desc

# Generic keyboard report (5-bit LEDS + 3-bit padding)
#echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > functions/hid.usb0/report_desc

# Dell keyboard report (3-bit LEDS + 5-bit padding)
#echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x01\\x95\\x03\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x03\\x91\\x02\\x95\\x01\\x75\\x01\\x91\\x01\\x95\\x06\\x75\\x08\\x15\\x00\\x26\\xff\\x00\\x05\\x07\\x19\\x00\\x2a\\xff\\x00\\x81\\x00\\xc0 > functions/hid.usb0/report_desc

# --------------------------------------

C=1
mkdir -p configs/c.$C/strings/0x409
echo "Config $C"                > configs/c.$C/strings/0x409/configuration 
echo 50                         > configs/c.$C/MaxPower 
#echo "0xa0"                     > configs/c.1/bmAttributes

ln -s functions/hid.$N configs/c.$C/
ls /sys/class/udc > UDC

exit

# ------------------------------------------------------------------------------
# vi: et ts=4 sw=4 ai
