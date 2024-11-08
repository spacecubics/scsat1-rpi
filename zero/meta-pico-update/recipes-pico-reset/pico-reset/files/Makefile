SRCS=main.c
OBJS=$(SRCS:.c=.o)

all: $(OBJS)
	${CC} ${CFLAGS} -o pico_reset $(OBJS) -l gpiod

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 0755 pico_reset ${DESTDIR}${BINDIR}

clean:
	rm -rf pico_reset *.o
