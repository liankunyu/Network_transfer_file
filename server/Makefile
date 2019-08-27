DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))

TARGET = server

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc
LIBS = -lpthread
CFLAGS = -g -Wall -I${DIR_INC}

${BIN_TARGET}:$(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find ${DIR_OBJ} -name *.o | xargs rm -rf
	find ${DIR_BIN} -name server | xargs rm -rf
.PHONY: clean
