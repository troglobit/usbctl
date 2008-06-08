# Builds usbctl and libs.
# Heavy use of implicit rules.
# Copyright (C) 2005 Joachim Nilsson <jocke()vmlinux!org>
# Free to use under the terms of the GNU General Public License.

AR      = @ar
ARFLAGS = crus

CC      = @gcc-2.95
CLAGS   = -g -fPIC
CPPFLAGS= -Wall -I.
LDFLAGS = -static -L.
LDLIBS  = -lnsl -lm -lc -lusb -lusbctl

RM      = @rm -f

APPS    = usbctl
LIBOBJS = usbmisc.o usbext.o
LIB     = libusbctl
LIBS    = $(addprefix $(LIB), .so .a)
JUNK    = *~ semantic.cache $(APPS) $(LIBS)

all: $(LIBS)($(LIBOBJS)) $(APPS)
	@upx -qqq $(APPS)

$(LIB).so: $(LIBOBJS)
	$(CC) -shared $^ -o $@

clean:
	$(RM) $(JUNK)
