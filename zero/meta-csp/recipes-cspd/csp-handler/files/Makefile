SRCS := system.c shell.c upload.c file.c hwtest.c camera.c temp.c handler.c router.c main.c
OBJS := $(SRCS:.c=.o)
SYSTEMD_CFLAGS := $(shell pkg-config --cflags libsystemd)
SYSTEMD_LIBS := $(shell pkg-config --libs libsystemd)
GLIB2_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB2_LIBS := $(shell pkg-config --libs glib-2.0)


all: cspd

cspd: $(OBJS)
	$(CC) -o $@ $(OBJS) -l csp $(SYSTEMD_LIBS) $(GLIB2_LIBS)

%.o: %.c
	$(CC) -Wall -Wextra $(CFLAGS) $(SYSTEMD_CFLAGS) $(GLIB2_CFLAGS) -c $< -o $@

$(OBJS): cspd.h

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 cspd $(DESTDIR)$(BINDIR)

clean:
	rm -rf cspd *.o
