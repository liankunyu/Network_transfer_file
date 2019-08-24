
LIBS = -lpthread
OBJS = main.o explore.o tcp.o udp.o sha256.o startsocket.o
CFLAGS = -g -Wall

server: ${OBJS}
	gcc -o $@ ${CFLAGS} ${OBJS} ${LIBS}

clean:
	@echo "cleanning project"
	-rm main *.o
	@echo "clean completed"
.PHONY: clean
