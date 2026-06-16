CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE

COMMON = graph.c file_reader.c dijkstra.c
GUI = sim_main.c drawing.c Dijkstra_draw.c sync.c

milestone1:
	$(CC) $(CFLAGS) -o dijkstra milestone1_main.c $(COMMON) -lm

milestone2:
	$(CC) $(CFLAGS) -DMILESTONE=2 -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone3:
	$(CC) $(CFLAGS) -DMILESTONE=3 -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone4:
	$(CC) $(CFLAGS) -DMILESTONE=4 -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone5:
	$(CC) $(CFLAGS) -DMILESTONE=5 -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone6:
	$(CC) $(CFLAGS) -DMILESTONE=6 -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

clean:
	rm -f dijkstra sim *.o
