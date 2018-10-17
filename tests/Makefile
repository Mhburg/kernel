CC = $(NDK_PATH)/clang
LD = $(NDK_PATH)/clang

########## YOUR MAKEFILE BELOW ##########
INCLUDE = ./include
LDLIBS = ./lib
CFLAGS = -Wall -Werror -g -O2 -I$(INCLUDE)
LDFLAGS = -L$(LDLIBS)

all: faceup facedown

faceup: faceup.o
	$(LD) $^ -o $@ $(LDFLAGS)

facedown: facedown.o
	$(LD) $^ -o $@ $(LDFLAGS)

faceup.o: faceup.c faceup.h
	$(CC) -c $(CFLAGS) $<

facedown.o: facedown.c facedown.h
	$(CC) -c $(CFLAGS) $<


.PHONY: clean

clean:
	rm -f *.o faceup facedown