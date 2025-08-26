SRCS := system.c shell.c upload.c file.c hwtest.c camera.c temp.c handler.c router.c main.c
OBJS := $(SRCS:.c=.o)
CPP_SRCS := camera/capture_raw.cpp camera/dng_writer.cpp camera/frame_plane_mapper.cpp
CPP_OBJS := $(CPP_SRCS:.cpp=.o)

SYSTEMD_CFLAGS := $(shell pkg-config --cflags libsystemd)
SYSTEMD_LIBS := $(shell pkg-config --libs libsystemd)
GLIB2_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB2_LIBS := $(shell pkg-config --libs glib-2.0)
LIBCAMERA_LIBS := $(shell pkg-config --libs libcamera)
LIBCAMERA_CFLAGS := $(shell pkg-config --cflags libcamera)

CFLAGS := -Wall -Wextra
CXXFLAGS := -Wall -Wextra $(LIBCAMERA_CFLAGS)

all: cspd

cspd: $(OBJS) $(CPP_OBJS)
	$(CXX) -o $@ $(OBJS) $(CPP_OBJS) -l csp $(SYSTEMD_LIBS) $(GLIB2_LIBS) $(LIBCAMERA_LIBS) -ltiff

%.o: %.c
	$(CC) $(CFLAGS) $(SYSTEMD_CFLAGS) $(GLIB2_CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS): cspd.h

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 cspd $(DESTDIR)$(BINDIR)

clean:
	rm -rf cspd *.o
