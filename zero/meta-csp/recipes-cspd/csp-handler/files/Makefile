SRCS := camera.c temp.c handler.c router.c main.c
OBJS := $(SRCS:.c=.o)

all: cspd

cspd: $(OBJS)
	${CC} ${CFLAGS} -o $@ $(OBJS) -l csp

%.o: %.c
	$(CC) -Wall -Wextra $(CFLAGS) -c $< -o $@

$(OBJS): cspd.h

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 0755 cspd ${DESTDIR}${BINDIR}

clean:
	rm -rf cspd *.o
