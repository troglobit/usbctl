CC=@gcc
CLAGS=-g
LDFLAGS=-static -L.
LDLIBS=-lnsl -lm -lc -lusb

APPS = usbctl
OBJS = $(addsuffix .o, $(notdir $(APPS)))

all: $(APPS)
	@upx -qqq $<

clean:
	@$(RM) $(APPS) $(OBJS)
