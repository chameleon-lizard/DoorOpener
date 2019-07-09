all: komodo

WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2

komodo: Makefile komodo.c
	gcc komodo.c -o komodo $$(pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0)

debug: Makefile komodo.c
	gcc komodo.c -g -o komodo $$(pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0)
	gdb ./komodo

clean:
	rm -f komodo

run:
	./komodo
