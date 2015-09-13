  Chicken Coop Garage Door Opener/Closer
==========================================

  Here we document a controller for opening and closing a chicken coop
door using an old garage door opener that has been modified.

1. The Problem
--------------

  Here in Oregon there are many night predators that would love to a
	chicken into a nice meal.  The trick is to keep the predators out of
	your chicken coop.  Chickens naturally go into the coop when it gets
	dark, we just need a way to shut the door sometime after the sun
	sets and open it around sunrise.

2. Ideas
--------------

  A great solution would be to have a WiFi enabled module that could
	get the current time of day and sunrise/sunset time off the web but
	for this particular application there wasn't a nearby router.

3. Harware Design
--------------
 
  So what was done was to take a very low power microcontroller (about
	16 microamps in sleep mode) with a super capacitor (4 Farads) to
	live through power outages (up to at least 5 days).  The time of day
	is programmed in when the microcontroller is flashed and we keep
	accurate time by a 60 Hz line interrupt.  The current sunrise/sunset
	times for one year for our area are stored in a table in flash
	memory.  The jumper is removed when progamming then inserted when
	done but then quickly disconnect the programmer as it's 3v3 and the
	supercap can be up to 5v1.  The door sensor is a simple switch that
	is engaged when the door is completely down.

  The changes made to the STM8 board for the lowest power were:

A.  The power LED was disabled by removing R3 in
docs/hardware/800px-STM8S103F3P6.png

B. The 3 leads from U2 were cut.

C. The UART was disabled in the software when not debugging.

3. Software Design
--------------

  A simple loop in main.c keeps track of the current time from
	progamming and adjusts the sunrise/sunset times as each day passes.
	The LED is flashed at certain rates to indicate normal operation or
	any errors.  The time from sunrise/sunset to door open/close is
	currently set to 30 minutes.  When power is okay, the 60 Hz GPIO
	interrupt runs to keep accurate time.  When a power failure occurs
	the AWU interrupt starts running and it keeps track of time.  The
	accuracy of the internal STM8 crystal is not very good so I tweaked
	it to be closer to reality, your STM8 might be different than mine.
	The perl script parse.sun.pl populates time.h with the current time
	and the sunrise/sunset times for the area you are in using sun.txt
	which is gotten from [Sun Times](http://aa.usno.navy.mil/data/docs/RS_OneYear.php)

5. Parts
--------------
  I had many of the parts but here are some that I ordered:

[Super-capacitor](http://www.aliexpress.com/item/Super-capacitor-farad-capacitor-type-double-layer-capacitor-5-5V-4F-V-type/1558646499.html)

[FTDI-USB](http://www.aliexpress.com/item/1pcs-FT232RL-FTDI-USB-3-3V-5-5V-to-TTL-Serial-Adapter-Module-for-Arduino-Mini/2019421866.html)

[STM8S103F3P6](http://www.aliexpress.com/item/ARM-STM8S103F3P6-STM8-Minimum-System-Development-Board-Module-For-Arduino/32307411825.html)

[SST-LINK-V2](http://www.aliexpress.com/item/mini-ST-LINK-V2-ST-LINK-STLINK-STM8-STM32-emulator-download-super-protection/1551631840.html)

5. Tools
--------------

  Debian Linux was used to build using sdcc.  You will need to get the
	latest sdcc (I used sdcc-3.4.0-i386-unknown-linux2.5.tar.bz2), see
	notes.txt for more info.  Parts of the STM8S_StdPeriph_Lib are used,
	see stsw-stm8069.zip and STM8S_StdPeriph_Lib.patch.  You will also
	need: stm8flash (in /opt/stm8flash
	[https://github.com/vdudouyt/stm8flash.git]) sdcc-3.4.0 in
	/opt/sdcc-3.4.0) I also needed to do:

```
sudo cp 49-stlinkv2.rules /etc/udev/rules.d
sudo apt-get install libusb-1.0-0-dev minicom pkg-config libdate-manip-perl;sudo udevadm control --reload-rules; sudo usermod -a -G dialout $USER; su - $USER; minicom
```

5. Images
--------------

The modified garage door opener.
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/garagedooropener.png "garagedooropener")
The mainboard with the STM8 daughter board.
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/mainboard.png "mainboard")
Everything hooked up ready for programming and serial debug.
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/programming.png "programming")
Schematic
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/schematic.png "schematic")
Coop Door
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/coop.jpg "Coop")
Door Sensor Switch
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/blob/master/images/./images/Door-switch.png "Coop")
