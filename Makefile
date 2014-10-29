.PHONY: all clean

TARGET=lecteur
SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

all: $(TARGET)

clean:
	rm -rf *.o $(TARGET)

CC=gcc
CFLAGS=-Wall -g -pedantic -std=c99 `pkg-config gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10 --cflags`
LDLIBS=-lpthread `pkg-config gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10 --libs`
$(TARGET):
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)
