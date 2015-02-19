CC = gcc
LIBVER ?= 1
LIBVER_MINOR ?= 0

ARCH=$(shell uname -i)
ifeq "${ARCH}" "x86_64"
LIBDIR ?= /usr/lib64
else
LIBDIR ?= /usr/lib
endif

INCLUDEDIR = /usr/include/vz
VZEVENT_DIR = /var/run/vzevents

CFLAGS = -Wall -O0 -fPIC -g
INC = -I/usr/include -I../include
LIB_vzevent_major = libvzevent.so.${LIBVER}
LIB_vzevent = ${LIB_vzevent_major}.${LIBVER_MINOR}
TARGET = $(LIB_vzevent) libvzevent.a

TARGET = libvzevent.a $(LIB_vzevent)
OBJS = vzevent.o vzevent_error.o

all: ${TARGET}

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(LIB_vzevent): ${OBJS}
	$(CC) $(CFLAGS) $(INC) $(LDFLAGS) -shared -lpthread \
		-Wl,-soname=$(LIB_vzevent_major) $^ -o $@ 

libvzevent.a: ${OBJS}
	ar rucs $@ $^
	ranlib $@

install: $(LIB_vzevent)
	install -d $(DESTDIR)/$(LIBDIR)
	install -d $(DESTDIR)/$(INCLUDEDIR)
	install -d $(DESTDIR)/$(VZEVENT_DIR)
	install -m 644 ../include/vzevent.h $(DESTDIR)/$(INCLUDEDIR)
	install -s -m 755 $(LIB_vzevent) $(DESTDIR)/$(LIBDIR)
	install -m 644 libvzevent.a $(DESTDIR)/$(LIBDIR)
	ln -sf $(LIB_vzevent) libvzevent.so
	ln -sf $(LIB_vzevent) libvzevent.so.${LIBVER}
	cp -af libvzevent.so $(DESTDIR)/$(LIBDIR)/
	cp -af libvzevent.so.${LIBVER} $(DESTDIR)/$(LIBDIR)/

clean:
	rm -f ${OBJS} ${TARGET}

.PHONY: all clean
