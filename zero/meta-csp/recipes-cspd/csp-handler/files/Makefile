SRCS := camera.c temp.c handler.c router.c main.c
OBJS := $(SRCS:.c=.o)
SYSTEMD_CFLAGS := $(shell pkg-config --cflags libsystemd)
SYSTEMD_LIBS := $(shell pkg-config --libs libsystemd)


all: cspd

cspd: $(OBJS)
	$(CC) -o $@ $(OBJS) -l csp $(SYSTEMD_LIBS)

%.o: %.c
	$(CC) -Wall -Wextra $(CFLAGS) $(SYSTEMD_CFLAGS) -c $< -o $@

$(OBJS): cspd.h

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 cspd $(DESTDIR)$(BINDIR)

clean:
	rm -rf cspd *.o
