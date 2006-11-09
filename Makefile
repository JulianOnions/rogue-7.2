XSTR =	xstr
CC =	cc
STRIP =	strip

HDRS=	rogue.h mach_dep.h
OBJS=	vers.o global.o armor.o chase.o command.o daemon.o daemons.o\
	disply.o encumb.o fight.o init.o io.o list.o main.o misc.o\
	monsters.o move.o new_level.o options.o pack.o passages.o\
	potions.o pstats.o rings.o rip.o rooms.o save.o scrolls.o\
	sticks.o things.o trader.o weapons.o wizard.o strings.o
CFILES=	vers.c global.c armor.c chase.c command.c daemon.c daemons.c\
	disply.c encumb.c fight.c init.c io.c list.c main.c misc.c\
	monsters.c move.c new_level.c options.c pack.c passages.c\
	potions.c pstats.c rings.c rip.c rooms.c save.c scrolls.c\
	sticks.c things.c trader.c weapons.c wizard.c

VERSION=	-DBSD4_2
CFLAGS=		-O -c
LDFLAGS=	
INSDIR = /usr/sheriff/jpo/games

rogue7.2: $(HDRS) $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -lcurses -ltermlib -o rogue7.2
	
install: rogue7.2
	cp rogue7.2 $(INSDIR)/rogue7.2
	$(STRIP) $(INSDIR)/rogue7.2
	chown bin $(INSDIR)/rogue7.2
	chgrp bin $(INSDIR)/rogue7.2
	chmod 4111 $(INSDIR)/rogue7.2

main.o rip.o io.o command.o wizard.o: mach_dep.h

global.o init.o wizard.o things.o: rogue.h

vers.o:	vers.c
	$(CC) $(CFLAGS) $(VERSION) vers.c

strings.o: strings
	$(XSTR)
	$(CC) -S xs.c
	ed - < :rofix xs.s
	$(AS) -o strings.o xs.s
	rm xs.s xs.c

.c.o:
	$(CC) -E $(VERSION) $*.c | $(XSTR) -c -
	$(CC) $(CFLAGS) x.c
	mv x.o $*.o
	rm x.c

listing:
	pr $(HDRS) $(CFILES) | lpr
