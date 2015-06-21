#############################
# Makefile 
# Author: Lester
#############################

VERSION    = 0.0
PROGRAM    = saf7741_test

# Cross-compile
PREFIX     = /qsi_rootfs/root/
CC_PATH    = /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/
CROSS      = arm-linux-

# Binary target/source
DESTDIR    = ../../output
EXTRA_SRC  = 
DSP_SRC    = ../


# Compiler
HOST       = $(CC_PATH)$(CROSS)
CC         = $(HOST)gcc
DEFINES    = -D_GNU_SOURCE
CFLAGS     = -g -O2 -Wall -Werror -std=gnu99 -march=armv7-a -mfpu=neon $(DEFINES)
INCPATH    = -I$(DSP_SRC)/inc -I$(PREFIX)/usr/include -I$(PREFIX)/usr/local/include 
LINK       = $(HOST)gcc
LIBPATH    = -L$(PREFIX)/usr/lib -L$(PREFIX)/usr/local/lib
RPATH      =
RPATH_LINK = $(PREFIX)/usr/lib
LFLAGS     = -rdynamic -Wl,-rpath-link=$(RPATH_LINK)
LIBS       = $(LIBPATH) -lpthread
AR         = $(HOST)ar
OBJECTS    = $(shell ls ./*.c     | sed 's/\.c/.o/g')\
             $(shell ls $(DSP_SRC)/LIB/*.c     | sed 's/\.c/.o/g')\
             $(shell ls $(DSP_SRC)/HAL/*.c     | sed 's/\.c/.o/g')\
             $(shell ls $(DSP_SRC)/HAL/*/*.c     | sed 's/\.c/.o/g')
    		 
# All Target
all: $(PROGRAM) install

$(PROGRAM): $(OBJECTS)
	@echo -e '\e[1;36mMake $@... \e[0m'
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(LIBS)  

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCPATH) $< -o $@
			
install:
	@test -d $(DESTDIR) || mkdir -p $(DESTDIR)
#	@cp $(PROGRAM) $(PREFIX)/root/
	@mv $(PROGRAM) $(DESTDIR)/
	@sync
	@echo -e '\e[1;33mOutput : $(DESTDIR)/$(PROGRAM)\e[0m'
	
clean:
	@echo -e '\e[1;31mClear $(PROGRAM)...\e[0m'
	@rm -f $(OBJECTS)

