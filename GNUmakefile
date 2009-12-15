PREFIX?=/usr/local
INSTALL_BIN?=${PREFIX}/bin
INSTALL_MAN?=${PREFIX}/man/man1

INSTALL?=`which install`
GROFF?=`which groff`

ifdef NDEBUG
  CFLAGS=-ansi -O2 -pedantic -Wall -D_BSD_SOURCE -DNDEBUG
else
  CFLAGS=-ansi -g -pedantic -Wall -D_BSD_SOURCE
endif

.PHONY: all clean lint

TARGET=sfv
OBJECTS=sfvlib.o sfv.o crc.o
MAN=sfv.1
TEST_TARGET=crctest
TEST_OBJECTS=crctest.o crc.o

all: ${TARGET}

install: ${TARGET}
	${INSTALL} -g bin -o root -m 755 ${TARGET} ${INSTALL_BIN}
	${INSTALL} -g bin -o root -m 644 ${MAN} ${INSTALL_MAN}

clean:
	rm -f ${TARGET}
	rm -f ${OBJECTS}
	rm -f ${TEST_TARGET}
	rm -f ${TEST_OBJECTS}
	rm -f llib-lsfv.ln

obj: ${OBJECTS}

html:
	${GROFF} -Thtml -mandoc ${MAN} > sfv.html

lint:
	lint *.c -C${TARGET} -H -I. -I/usr/include

test: ${TEST_TARGET}
	./crctest

${TEST_TARGET}: ${TEST_OBJECTS}
	${CC} ${CFLAGS} $+ -o $@

${TARGET}: ${OBJECTS}
	${CC} ${CFLAGS} $+ -o $@

.c.o:
	${CC} ${CFLAGS} -c $< -o $@
