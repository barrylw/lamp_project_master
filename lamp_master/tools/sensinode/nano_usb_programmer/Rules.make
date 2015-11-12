<<<<<<< HEAD
ifeq "$(PLATFORM)" "linux"
CC=gcc
CFLAGS= -I. -I./ftdi_linux -L. -L./ftdi_linux -DPLATFORM_LINUX
LDFLAGS= -lftd2xx -Wl,-rpath /usr/local/lib
SOURCES=main.c prog.c cdi.c ihex.c
SUFFIX=
else
CC=gcc
CFLAGS=-I. -I./ftdi_win32 -L. -L./ftdi_win32 -DPLATFORM_WINDOWS
CFLAGS+= -mwin32 
LDFLAGS=ftdi_win32/ftd2xx.lib -lkernel32
SOURCES=main.c prog.c cdi.c ihex.c
SUFFIX=.exe
endif
=======
ifeq "$(PLATFORM)" "linux"
CC=gcc
CFLAGS= -I. -I./ftdi_linux -L. -L./ftdi_linux -DPLATFORM_LINUX
LDFLAGS= -lftd2xx -Wl,-rpath /usr/local/lib
SOURCES=main.c prog.c cdi.c ihex.c
SUFFIX=
else
CC=gcc
CFLAGS=-I. -I./ftdi_win32 -L. -L./ftdi_win32 -DPLATFORM_WINDOWS
CFLAGS+= -mwin32 
LDFLAGS=ftdi_win32/ftd2xx.lib -lkernel32
SOURCES=main.c prog.c cdi.c ihex.c
SUFFIX=.exe
endif
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24
