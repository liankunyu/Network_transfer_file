main: main.o explore.o tcp.o udp.o sha256.o startsocket.o
	gcc -Wall main.o explore.o tcp.o udp.o sha256.o startsocket.o -lpthread -g -o main

main.o: main.c explore.h tcp.h udp.h sha256.h
	gcc -g -c main.c
udp.o: udp.c udp.h sha256.h protocol.h startsocket.h
	gcc -g -c udp.c
tcp.o: tcp.c tcp.h sha256.h protocol.h startsocket.h
	gcc -g -c tcp.c
explore.o: explore.c explore.h protocol.h
	gcc -g -c explore.c
startsocket.o: startsocket.c startsocket.h
	gcc -g -c startsocket.c
sha256.o: sha256.c sha256.h
	gcc -g -c sha256.c
clean:
	@echo "cleanning project"
	-rm main *.o
	@echo "clean completed"
.PHONY: clean
