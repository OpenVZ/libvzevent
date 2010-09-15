CC = gcc
LIBVER = 1

LIBDIR ?= /usr/lib
INCLUDEDIR = /usr/include/vz

CFLAGS = -Wall -O0 -fPIC -g
INC = -I/usr/include -I../include
LIB_vzevent = libvzevent.so.${LIBVER}
TARGET = $(LIB_vzevent) libvzevent.a

TARGET = libvzevent.a $(LIB_vzevent)
OBJS = vzevent.o vzevent_error.o

all: ${TARGET}

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(LIB_vzevent): ${OBJS}
	$(CC) $(CFLAGS) $(INC) $(LDFLAGS) -shared $^ -o $@ 

libvzevent.a: ${OBJS}
	ar rucs $@ $^
	ranlib $@

install: $(LIB_vzevent)
	install -d $(DESTDIR)/$(LIBDIR)
	install -d $(DESTDIR)/$(INCLUDEDIR)
	install -m 644 ../include/vzevent.h $(DESTDIR)/$(INCLUDEDIR)
	install -s -m 644 $(LIB_vzevent) $(DESTDIR)/$(LIBDIR)
	install -m 644 libvzevent.a $(DESTDIR)/$(LIBDIR)
	ln -sf $(LIB_vzevent) libvzevent.so
	cp -af libvzevent.so $(DESTDIR)/$(LIBDIR)/

clean:
	rm -f ${OBJS} ${TARGET}

.PHONY: all clean
