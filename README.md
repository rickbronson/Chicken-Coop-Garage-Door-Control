  Chicken Coop Garage Door Opener/Closer
==========================================

  Here we document a controller for opening and closing a chicken coop
door using an old garage door opener that has been modified.

1. The Problem
--------------

  Here in Oregon there are many night predators that would love to
	turn your chicken into a nice meal.  The trick is to keep the
	predators out of your chicken coop.  Chickens naturally go into the
	coop when it gets dark, we just need a way to shut the door sometime
	after the sun sets and open it around sunrise.

2. Ideas
--------------

  A great solution would be to have a WiFi enabled module that could
	get the current time of day and sunrise/sunset time off the web but
	for this particular application there wasn't a nearby router.

3. Design
--------------
 
  So what was done was to take a very low power microcontroller (about
	20 microamps in sleep mode) with a super capacitor (4 Farads) to
	live through power outages (up to about 4 days).  The time of day is
	programmed in when the microcontroller is flashed and we keep
	accurate time by a 60 Hz line interrupt.  The current sunrise/sunset
	times for one year for our area are stored in a table in flash
	memory.

4. Parts
--------------
  I had many of the parts but here are some that I ordered:

http://www.aliexpress.com/item/Super-capacitor-farad-capacitor-type-double-layer-capacitor-5-5V-4F-V-type/1558646499.html
http://www.aliexpress.com/item/1pcs-FT232RL-FTDI-USB-3-3V-5-5V-to-TTL-Serial-Adapter-Module-for-Arduino-Mini/2019421866.html
http://www.aliexpress.com/item/ARM-STM8S103F3P6-STM8-Minimum-System-Development-Board-Module-For-Arduino/32307411825.html
http://www.aliexpress.com/item/mini-ST-LINK-V2-ST-LINK-STLINK-STM8-STM32-emulator-download-super-protection/1551631840.html

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

![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/images/garagedooropener.png "garagedooropener")
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/images/mainboard.png "mainboard")
![alt text](https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control/images/programming.png "programming")
