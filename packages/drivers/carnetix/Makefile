
CC = gcc
LD = gcc
AR = ar

LDFLAGS= -lusb -Lbin -g -L/opt/local/lib
CFLAGS= -g -Iinclude -I/opt/local/include

all: bin/libctx.a ctxutil consolemon

clean:
	rm -rf bin *~ ctx consolemon

# Building ctx library 
bin:
	mkdir -p bin

bin/ctxapi.o: src/ctxapi.c include/ctxapi.h bin
	$(CC) $(CFLAGS) -c src/ctxapi.c -o bin/ctxapi.o

bin/libctx.a: bin/ctxapi.o
	$(AR) rcs bin/libctx.a bin/ctxapi.o 
	ranlib bin/libctx.a

# Build Test/Demo Programs
test/bin:
	mkdir -p test/bin

# Test Program
bin/ctx.o: test/src/ctx.c include/ctxapi.h
	$(CC) $(CFLAGS) -c test/src/ctx.c -o bin/ctx.o 

ctxutil: bin/ctx.o bin/ctxapi.o
	$(CC) -lusb bin/ctxapi.o  bin/ctx.o -o ctxutil

# Demo Program
bin/consolemon.o: test/src/consolemon.c include/ctxapi.h bin/ctxapi.o
	$(CC) $(CFLAGS) -c test/src/consolemon.c -o bin/consolemon.o


consolemon: bin/ctxapi.o bin/consolemon.o
	$(CC) -lusb bin/consolemon.o bin/ctxapi.o -o consolemon 
