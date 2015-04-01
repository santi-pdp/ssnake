CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lncurses -lpthread -lm
SOURCES=snake.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=snake

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf *o $(EXECUTABLE)
