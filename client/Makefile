
LIBS = -lpthread
OBJS = main.o startsocket.o tcp_recv_thread.o tcp_send_thread.o udp_recv_thread.o udp_send_thread.o sha256.o
CFLAGS = -g -Wall

client: ${OBJS}
	gcc -o $@ ${CFLAGS} ${OBJS} ${LIBS}


clean:
	@echo "cleanning project"
	-rm main *.o
	@echo "clean completed"
.PHONY: clean

