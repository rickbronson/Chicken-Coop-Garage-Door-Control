PROJ=stm8
export OPT_HOME := $(shell pwd)
include $(OPT_HOME)/${PROJ}.mk
# set up for parallel make's if make is new enough
export MK_PAR=$(shell if make -v | grep "3.80" > /dev/null ; then echo -n; else echo -n "-j9"; fi)
TMPMNT=/tmp/mnt
$(shell mkdir -p $(TMPMNT))
#APPDIRNAME=ble_app_proximity
#APPNAME=ble_app_proximity
APPDIRNAME=adc-example-with-softdevice
APPNAME=uart
LIB=$(OPT_HOME)/STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_test/SDCC/$(DEVICE)

DEVICE=STM8S103

APP=$(APPDIR)/_build/$(APPNAME)_s110_xxaa
VERS=3
OUT_DIR= $(shell	if [ -d "$(HOME)/in" ] ; then \
		echo -n $(HOME)/in; else echo -n /tmp;	fi)

CFLAGS=-I$(OPT_HOME)/STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver/inc -I$(OPT_HOME)/STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_Examples/CLK/CLK_ClockSelection -DSTM8S103

.PHONY: sdcc-examples-stm8

all: uart

lib:
#	cd STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_Template/SDCC; make clean; test -d $(DEVICE) || mkdir $(DEVICE)
#	cd STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_Template/SDCC; make DEVICE=$(DEVICE) TARGET=./$(DEVICE)/$(DEVICE).hex
	cd STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_test/SDCC; make clean; test -d $(DEVICE) || mkdir $(DEVICE)
	cd STM8S_StdPeriph_Lib/Project/STM8S_StdPeriph_test/SDCC; make DEVICE=$(DEVICE) TARGET=./$(DEVICE)/$(DEVICE).hex

uart:
	cd scripts; ./parse.sun.pl
	$(CC) -mstm8 -lstm8 --opt-code-size -DUSE_STM8_128_EVAL $(CFLAGS) -DSKIP_TRAPS=1 -c main.c -o main.rel
	$(CC) -mstm8 -lstm8 --opt-code-size -DUSE_STM8_128_EVAL --out-fmt-ihx -o $@.ihx main.rel
	$(OBJCOPY)  -Iihex -Obinary $@.ihx $@.bin
	if ls /dev/stlinkv2_* > /dev/null; then stm8flash -cstlinkv2 -pstm8s103 -w $@.ihx; fi

debug:
	make clean
	cd $(APPDIR); make -f $(APPNAME).Makefile DEBUG_APP=1

dis:
	$(OBJDUMP) -S $(APP).out > dis.lis

clean:
	cd $(APPDIR); make -f $(APPNAME).Makefile clean

github-init:
	git init
	git add ./awu.h ./docs/hardware/800px-STM8S103F3P6.png ./docs/hardware/board.png ./docs/hardware/CD00161709.pdf ./docs/hardware/CD00190271.pdf ./docs/hardware/CD00199324.pdf ./docs/hardware/CD00226640.pdf ./docs/hardware/chip-pinout.png ./docs/hardware/ds1117.pdf ./docs/hardware/hookup1.png ./docs/hardware/stlinkv2.png ./images/garagedooropener.png ./images/coop.jpg ./images/Door-switch.png ./images/mainboard.png ./images/programming.png ./images/schematic.png ./main.c ./main-low-power-test.c ./main-power-debug.c ./Makefile ./notes.txt ./README.md ./stm8.mk ./stsw-stm8069.zip ./STM8S_StdPeriph_Lib.patch ./time.h ./gitignore ./scripts/coop-door-helper.pl ./scripts/parse.sun.pl ./scripts/sun.txt 49-stlinkv2.rules
	git commit -m "first commit"
	git remote add origin https://github.com/rickbronson/Chicken-Coop-Garage-Door-Control.git
	git push -u origin master

github-update:
	git commit -a -m 'update README'
	git push -u origin master

st-patch:
	rm -f STM8S_StdPeriph_Lib.patch; for file in `find -L STM8S_StdPeriph_Lib -name "*.~1~"` ; do root_name=`echo $$file | sed -e "s|\(.*\).~1~|\1|"`;  echo "* diffing $$root_name{.~1~,}"; diff -uNr $$root_name.~1~ $$root_name >> STM8S_StdPeriph_Lib.patch; ls > /dev/null; done

