CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE

COMMON = graph.c file_reader.c dijkstra.c
DRAWING = drawing.c Dijkstra_draw.c
SYNC = sync.c queue.c scheduler.c

LIBS = -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone1:
	$(CC) $(CFLAGS) -o dijkstra main_milestone1.c $(COMMON) -lm

milestone2:
	$(CC) $(CFLAGS) -o sim main_milestone2.c $(COMMON) $(DRAWING) -DMILESTONE=2 $(LIBS)

milestone3:
	$(CC) $(CFLAGS) -o sim main_milestone3.c $(COMMON) $(DRAWING) -DMILESTONE=3 $(LIBS)

milestone4:
	$(CC) $(CFLAGS) -o sim main_milestone4.c $(COMMON) $(DRAWING) -DMILESTONE=4 $(LIBS)

milestone5:
	$(CC) $(CFLAGS) -o sim main_milestone5.c $(COMMON) $(DRAWING) -DMILESTONE=5 $(LIBS)

milestone6:
	$(CC) $(CFLAGS) -o sim main_milestone6.c $(COMMON) $(DRAWING) $(SYNC) -DMILESTONE=6 $(LIBS)

milestone7:
	$(CC) $(CFLAGS) -o sim main_milestone7.c $(COMMON) $(DRAWING) $(SYNC) -DMILESTONE=7 $(LIBS)

clean:
	rm -f dijkstra sim *.o