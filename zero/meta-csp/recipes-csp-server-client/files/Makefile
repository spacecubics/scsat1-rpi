all:
	${CC} csp_server_client.c csp_server_client_posix.c -o csp_server_client -l csp -pthread

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 0755 csp_server_client ${DESTDIR}${BINDIR}

clean:
	rm -rf csp_server_client
