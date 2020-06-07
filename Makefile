# GPL License - see http://opensource.org/licenses/gpl-license.php
# Copyright 2005 *nixCoders team - don't forget to credits us

CC = gcc
LD = gcc
CFLAGS = -g -Wall -fPIC -O3 -march=i686 -fno-strict-aliasing
LDFLAGS = -ldl -shared

OBJS = cg_functions.o engine.o hook.o q_math.o tools.o visuals.o $(shell grep -q -e ETH_PRIVATE\ 1 eth.h && echo private.o)
PROG = libETH.so

ETH_VERSION = $(shell grep -e define\ ETH_VERSION eth.h | cut -f2 -d\")
ETH_DISTFILE = eth.h cg_functions.h cg_functions.c engine.c hook.c tools.c visuals.c get-sdk.sh CREDITS INSTALL LICENSE eth.pk3 Makefile README run.sh
SDK_FOLDER = sdk
DIST_FOLDER = eth-$(ETH_VERSION)

# Main rule
all: $(PROG)

# Link
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

cg_functions.o: cg_functions.c eth.h
	$(CC) $(CFLAGS) -c cg_functions.c
engine.o: engine.c eth.h
	$(CC) $(CFLAGS) -c engine.c
hook.o: hook.c eth.h
	$(CC) $(CFLAGS) -c hook.c
q_math.o: sdk/src/game/q_math.c
	$(CC) $(CFLAGS) -c sdk/src/game/q_math.c
private.o: private.c eth.h
	$(CC) $(CFLAGS) -c private.c
tools.o: tools.c eth.h
	$(CC) $(CFLAGS) -c tools.c
visuals.o: visuals.c eth.h
	$(CC) $(CFLAGS) -c visuals.c

clean:
	rm -rf $(PROG) $(OBJS)

#pk3: eth.pk3
	#rm -f eth.pk3 && cd pk3 && zip -9r ../eth.pk3 * // todo

dist:
	mkdir $(DIST_FOLDER)
	rm -rf $(SDK_FOLDER)
	./get-sdk.sh
	cp -R $(SDK_FOLDER) $(ETH_DISTFILE) $(shell grep -q -e ETH_PRIVATE\ 1 eth.h && echo private.c private.h) $(DIST_FOLDER)
	tar czf ../eth-$(ETH_VERSION)$(shell grep -q -e ETH_PRIVATE\ 1 eth.h && echo -private).tar.gz $(DIST_FOLDER)
	rm -rf $(DIST_FOLDER)
