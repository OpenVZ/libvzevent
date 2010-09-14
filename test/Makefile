CC = gcc

CFLAGS = -Wall -O0 -g
INC = -I/usr/include -I../include
LIBS = ../src/libvzevent.a

TARGET=test
OBJS = test.o

all: ${TARGET}

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

test: ${OBJS}
	$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $^ ${LIBS} -o $@ 

clean:
	rm -f ${BJS} ${TARGET}

.PHONY: all clean
