PREFIX?=/usr/local
INSTALL_BIN?=${PREFIX}/bin
INSTALL_MAN?=${PREFIX}/man/man1

INSTALL?=`which install`
GROFF?=`which groff`

# Use these CFLAGS for debugging
ifdef NDEBUG
  CFLAGS=-ansi -O2 -pedantic -Wall -D_BSD_SOURCE -DNDEBUG
else
  CFLAGS=-ansi -g -pedantic -Wall -D_BSD_SOURCE
endif

.PHONY: all clean lint

TARGET=sfv
OBJECTS=sfvlib.o sfv.o crc.o
MAN=sfv.1

all: ${TARGET}

install: ${TARGET}
	${INSTALL} -g bin -o root -m 755 ${TARGET} ${INSTALL_BIN}
	${INSTALL} -g bin -o root -m 644 ${MAN} ${INSTALL_MAN}

clean:
	rm -f ${TARGET}
	rm -f ${OBJECTS}
	rm -f llib-lsfv.ln

obj: ${OBJECTS}

html:
	${GROFF} -Thtml -mandoc ${MAN} > sfv.html

lint:
	lint *.c -C${TARGET} -H -I. -I/usr/include

${TARGET}: ${OBJECTS}
	${CC} ${CFLAGS} $+ -o $@
#	strip $@

.c.o:
	${CC} ${CFLAGS} -c $+ -o $@
