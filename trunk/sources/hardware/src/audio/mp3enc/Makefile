#
# Shine_Fixed GENERIC PLATFORM GCC port Makefile
#

XPREFIX=
#XPREFIX= /opt/buildroot-gcc342/bin/mipsel-linux-
CXX=$(XPREFIX)g++
CC=$(XPREFIX)gcc
STRIP=$(XPREFIX)strip
AR=$(XPREFIX)ar
#CFLAGS=-fno-exceptions -Wall -O2 -fsigned-char -m32
CFLAGS=-fno-exceptions -Wall -O2 -fsigned-char 
INCLUDE=-I.
LIBS=-lm
LIBDIRS=


APPNAME=MP3fxpEnc
STNAME=libmp3enc.la
SONAME=libmp3enc.so.1
LIBNAME=$(SONAME).0.0

C_SRCS=$(shell ls *.c)
OBJFILES=$(C_SRCS:.c=.o)

#don''t need wav or main in library
LIBOBJS=$(shell echo $(OBJFILES) | sed -e "s/Main.o//" -e "s/wave.o//")

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

default: $(OBJFILES) $(APPNAME) library

library: $(LIBOJBS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(SONAME) -o $(LIBNAME) $(LIBOBJS) $(LIBDIRS) -lc -lm
	$(AR) rcs $(STNAME) $(LIBOBJS)

$(APPNAME): $(OBJFILES) library
	$(CC) $(CFLAGS) $(INCLUDE) -o $(APPNAME) $(OBJFILES) $(LIBDIRS) $(LIBS) $(STNAME)

release:
	$(STRIP) -s $(APPNAME)
	$(STRIP) -s $(LIBNAME)

cleanobj:
	rm -f $(OBJFILES)

clean: cleanobj
	rm -f *.la *.so* $(APPNAME)
