all: komodo

WARNINGS = -Wall
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2

komodo: Makefile komodo.c
	gcc komodo.c -o komodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`

quasimodo: Makefile quasimodo.c
	gcc quasimodo.c -o quasimodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`
	./quasimodo
	rm quasimodo

quasimodo-debug: Makefile quasimodo.c
	gcc quasimodo.c -g -o quasimodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`
	gdb ./quasimodo
	rm quasimodo

server: Makefile server-komodo.c
	gcc server-komodo.c -o server-komodo
	scp ./server-komodo arina@10.15.12.245:~/
	rm server-komodo

debug: Makefile komodo.c
	gcc komodo.c -g -o komodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`
	gdb ./komodo
	rm komodo

clean:
	rm komodo

run:
	gcc komodo.c -o komodo `pkg-config --cflags --libs gstreamer-video-1.0 gtk+-3.0 gstreamer-1.0`
	./komodo
	rm komodo
