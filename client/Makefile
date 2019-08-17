main: main.o startsocket.o tcp_recv_thread.o tcp_send_thread.o udp_recv_thread.o udp_send_thread.o sha256.o
	gcc -g main.o startsocket.o tcp_recv_thread.o tcp_send_thread.o udp_recv_thread.o udp_send_thread.o sha256.o -lpthread -o main

main.o: main.c protocol.h startsocket.h serverthread.h main.h
	gcc -g -c main.c 

startsocket.o: startsocket.c startsocket.h
	gcc -g -c startsocket.c
	
tcp_recv_thread.o: tcp_recv_thread.c
	gcc -g -c tcp_recv_thread.c
	
tcp_send_thread.o: tcp_send_thread.c
	gcc -g -c tcp_send_thread.c
	
udp_recv_thread.o: udp_recv_thread.c
	gcc -g -c udp_recv_thread.c

udp_send_thread.o: udp_send_thread.c main.h
	gcc -g -c udp_send_thread.c

sha256.o: sha256.c sha256.h
	gcc -g -c sha256.c

clean:
	@echo "cleanning project"
	-rm main *.o
	@echo "clean completed"
.PHONY: clean

