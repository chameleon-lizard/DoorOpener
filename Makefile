all: komodo

WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2

komodo: Makefile komodo.c
	gcc komodo.c -o komodo $$(pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0)

quasimodo: Makefile quasimodo.c
	gcc quasimodo.c -o quasimodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`
	./quasimodo
	rm quasimodo

debug: Makefile komodo.c
	gcc komodo.c -g -o komodo $$(pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0)
	gdb ./komodo

clean:
	rm komodo

run:
	gcc komodo.c -o komodo $$(pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0)
	./komodo
	rm komodo
