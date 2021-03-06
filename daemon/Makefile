#############################
# Makefile qsi-audio
# Author: Lester
#############################

VERSION    = 1.7
PROGRAM    = lst-audio

# Cross-compile : set PREFIX and PATH
PREFIX     = /qsi_rootfs/root/
CC_PATH    = /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/
CROSS      = arm-linux-

# Binary target/source
DEST_PATH  = ../output
EXTRA_SRC  = 
DSP_SRC    = ../dsp

# Compiler
HOST       = $(CC_PATH)$(CROSS)
CC         = $(HOST)gcc
DEFINES    = -DQ_ASSERT -D_GNU_SOURCE -DQAUDIO_VERSION='"$(VERSION)"'
CFLAGS     = -g -O2 -Wall -Werror -std=gnu99 -march=armv7-a -mfpu=neon $(DEFINES)
INCPATH    = -I$(PREFIX)/usr/include -I$(PREFIX)/usr/local/include -I./inc -I$(DSP_SRC)/inc
LINK       = $(HOST)gcc
LIBPATH    = -L$(PREFIX)/usr/lib -L$(PREFIX)/usr/local/lib -L$(PREFIX)/lib
RPATH      =
RPATH_LINK = $(PREFIX)/usr/lib
LFLAGS     = -rdynamic -Wl,-rpath-link=$(RPATH_LINK)
LIBS       = $(LIBPATH) -lpthread -lasound -lQSI-IPCLib
AR         = $(HOST)ar
OBJECTS    = $(shell ls ./src/*.c | sed 's/\.c/.o/g')\
             $(shell ls ./ext/*.c | sed 's/\.c/.o/g')\
    	     $(shell ls $(DSP_SRC)/LIB/*.c | sed 's/\.c/.o/g')\
             $(shell ls $(DSP_SRC)/HAL/*.c | sed 's/\.c/.o/g')\
    	     $(shell ls $(DSP_SRC)/HAL/*/*.c | sed 's/\.c/.o/g')
 
# All Target
all: $(PROGRAM) install

$(PROGRAM): $(OBJECTS)
	@echo -e '\e[1;36mMake $@... \e[0m'
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(LIBS)  

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCPATH) $< -o $@
		
install:
	@test -d $(DEST_PATH) || mkdir -p $(DEST_PATH)
	@mv $(PROGRAM) $(DEST_PATH)
	@sync
	@echo -e '\e[1;33mOutput : $(DEST_PATH)/$(PROGRAM)\e[0m'

clean:
	@echo -e '\e[1;31mClean $(PROGRAM)...\e[0m'
	@rm -f $(OBJECTS)


