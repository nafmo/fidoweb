# Makefile for OS/2, EMX and IBM nmake
#
# $Id$

CFLAGS=-Wall -Ismapi -Lsmapi -O3 -fomit-frame-pointer -fstrength-reduce -fno-rtti -fno-exceptions -static
# -Zcrtdll

all: reader.exe arealist.exe messages.exe new.exe reply.exe
        cp reader.exe d:\apache_1.3.6\htdocs\webbbbs
        cp arealist.exe d:\apache_1.3.6\htdocs\webbbbs
        cp messages.exe d:\apache_1.3.6\htdocs\webbbbs
        cp new.exe d:\apache_1.3.6\htdocs\webbbbs
        cp reply.exe d:\apache_1.3.6\htdocs\webbbbs

reader.exe: reader.o charset.o convert.o select.o getarg.o lastread.o htmlerror.o
        gcc $(CFLAGS) -o reader.exe reader.o charset.o convert.o getarg.o select.o lastread.o htmlerror.o reader.def -lsmapiemo -lstdcpp

reader.o: reader.cpp charset.h select.h convert.h config.h lastread.h
        gcc $(CFLAGS) -c reader.cpp

charset.o: charset.cpp config.h
        gcc $(CFLAGS) -c charset.cpp

convert.o: convert.cpp config.h
        gcc $(CFLAGS) -c convert.cpp

getarg.o: getarg.cpp config.h
        gcc $(CFLAGS) -c getarg.cpp

select.o: select.cpp select.h config.h
        gcc $(CFLAGS) -c select.cpp

arealist.exe: arealist.o getarg.o htmlerror.o config.h
        gcc $(CFLAGS) -o arealist.exe arealist.o getarg.o htmlerror.o reader.def -lstdcpp

arealist.o: arealist.cpp config.h
        gcc $(CFLAGS) -c arealist.cpp

messages.exe: messages.o select.o charset.o convert.o getarg.o lastread.o htmlerror.o
        gcc $(CFLAGS) -o messages.exe messages.o select.o charset.o convert.o getarg.o lastread.o htmlerror.o reader.def -lsmapiemo -lstdcpp

messages.o: messages.cpp config.h lastread.h
        gcc $(CFLAGS) -c messages.cpp

new.exe: new.o getarg.o htmlerror.o
        gcc $(CFLAGS) -o new.exe new.o getarg.o htmlerror.o reader.def -lstdcpp

new.o: new.cpp config.h
        gcc $(CFLAGS) -c new.cpp

lastread.o: lastread.cpp lastread.h config.h
        gcc $(CFLAGS) -c lastread.cpp

htmlerror.o: htmlerror.cpp htmlerror.h
        gcc $(CFLAGS) -c htmlerror.cpp

reply.exe: reply.o replycgi.o convert.o config.h htmlerror.o
        gcc $(CFLAGS) -o reply.exe reply.o replycgi.o convert.o htmlerror.o reader.def -lstdcpp

reply.o: reply.cpp datatyp.h pkthead.h config.h
        gcc $(CFLAGS) -c reply.cpp

replycgi.o: replycgi.cpp config.h
        gcc $(CFLAGS) -c replycgi.cpp
