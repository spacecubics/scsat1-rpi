SRCS=camera.c temp.c handler.c router.c main.c
OBJS=$(SRCS:.c=.o)

all: $(OBJS)
	${CC} ${CFLAGS} -o csp_handler $(OBJS) -l csp

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): cspd.h

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 0755 csp_handler ${DESTDIR}${BINDIR}

clean:
	rm -rf csp_handler *.o
