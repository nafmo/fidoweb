# Makefile for OS/2, EMX and IBM nmake
#
# $Id$

CFLAGS=-Wall -Zomf -Ismapi -Lsmapi -O3 -fomit-frame-pointer -fstrength-reduce -fno-rtti -fno-exceptions -static
# -Zcrtdll

all: reader.exe arealist.exe messages.exe new.exe reply.exe
        cp reader.exe d:\apache_1.3.6\htdocs\webbbbs
        cp arealist.exe d:\apache_1.3.6\htdocs\webbbbs
        cp messages.exe d:\apache_1.3.6\htdocs\webbbbs
        cp new.exe d:\apache_1.3.6\htdocs\webbbbs
        cp reply.exe d:\apache_1.3.6\htdocs\webbbbs

reader.exe: reader.obj charset.obj convert.obj select.obj getarg.obj lastread.obj
        gcc $(CFLAGS) -o reader.exe reader.obj charset.obj convert.obj getarg.obj select.obj lastread.obj reader.def -lsmapiemo -lstdcpp

reader.obj: reader.cpp charset.h select.h convert.h config.h lastread.h
        gcc $(CFLAGS) -c reader.cpp

charset.obj: charset.c config.h
        gcc $(CFLAGS) -c charset.c

convert.obj: convert.cpp config.h
        gcc $(CFLAGS) -c convert.cpp

convert.o: convert.cpp config.h
        gcc -DONLYTRANS -O3 -c convert.cpp

getarg.obj: getarg.cpp config.h
        gcc $(CFLAGS) -c getarg.cpp

select.obj: select.cpp select.h config.h
        gcc $(CFLAGS) -c select.cpp

arealist.exe: arealist.obj getarg.obj config.h
        gcc $(CFLAGS) -o arealist.exe arealist.obj getarg.obj reader.def -lstdcpp

arealist.obj: arealist.cpp config.h
        gcc $(CFLAGS) -c arealist.cpp

messages.exe: messages.obj select.obj charset.obj convert.obj getarg.obj lastread.obj
        gcc $(CFLAGS) -o messages.exe messages.obj select.obj charset.obj convert.obj getarg.obj lastread.obj reader.def -lsmapiemo -lstdcpp

messages.obj: messages.cpp config.h lastread.h
        gcc $(CFLAGS) -c messages.cpp

new.exe: new.obj getarg.obj
        gcc $(CFLAGS) -o new.exe new.obj getarg.obj reader.def -lstdcpp

new.obj: new.cpp config.h
        gcc $(CFLAGS) -c new.cpp

lastread.obj: lastread.cpp lastread.h config.h
        gcc $(CFLAGS) -c lastread.cpp

reply.exe: reply.o replycgi.o convert.o config.h
        gcc -O3 -o reply.exe reply.o replycgi.o convert.o reader.def -lstdcpp

reply.o: reply.cpp datatyp.h pkthead.h config.h
        gcc -O3 -c reply.cpp

replycgi.o: replycgi.cpp config.h
        gcc -O3 -c replycgi.cpp
