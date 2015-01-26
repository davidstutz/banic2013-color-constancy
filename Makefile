PROJECT=cs
SOURCE=main.cpp

CC=g++
OPENCV:=`pkg-config --cflags --libs opencv`
CFLAGS=-Wall -g -O3 $(OPENCV)
LDFLAGS=

all: $(PROJECT)

$(PROJECT): main.cpp
	$(CC) main.cpp $(CFLAGS) -o $(PROJECT)

.cpp.o:
	$(CC) -c -O3 $<


clean:
	-rm -f $(PROJECT) main.o *.core

