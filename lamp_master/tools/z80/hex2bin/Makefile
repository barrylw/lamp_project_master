<<<<<<< HEAD
#
# Makefile for hexameter
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.3 2009/12/15 07:13:14 matsutsuka Exp $
#

#CFLAGS		= -std=c99 -g -mno-cygwin -Wall
CFLAGS		= -g -Wall
SOURCEDIR	= src
SOURCES		= hexameter.c ihx2bin.c
OBJECTDIR	= bin
OBJECTS		= ${addprefix $(OBJECTDIR)/,$(SOURCES:.c=.o)}

vpath %.c $(SOURCEDIR)

hexameter: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf bin
	rm -f *~ *.stackdump
	rm -f *~ hexameter hexameter.exe

$(OBJECTDIR):
	mkdir $@

$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

=======
#
# Makefile for hexameter
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.3 2009/12/15 07:13:14 matsutsuka Exp $
#

#CFLAGS		= -std=c99 -g -mno-cygwin -Wall
CFLAGS		= -g -Wall
SOURCEDIR	= src
SOURCES		= hexameter.c ihx2bin.c
OBJECTDIR	= bin
OBJECTS		= ${addprefix $(OBJECTDIR)/,$(SOURCES:.c=.o)}

vpath %.c $(SOURCEDIR)

hexameter: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf bin
	rm -f *~ *.stackdump
	rm -f *~ hexameter hexameter.exe

$(OBJECTDIR):
	mkdir $@

$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24
