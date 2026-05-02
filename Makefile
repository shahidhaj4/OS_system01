CC = gcc
CFLAGS = -Wall -Wextra -std=c11

COMMON = graph.c file_reader.c dijkstra.c library.c
GUI = Main.c Dijkstra_draw.c drawing.c

milestone1:
	$(CC) $(CFLAGS) -o dijkstra $(COMMON) -lm

milestone2:
	$(CC) $(CFLAGS) -o sim $(COMMON) Main.c Dijkstra_draw.c -lraylib -lm -lX11 -lpthread -ldl -lrt

milestone3:
	$(CC) $(CFLAGS) -o sim $(COMMON) $(GUI) -lraylib -lm -lX11 -lpthread -ldl -lrt

clean:
	rm -f dijkstra sim *.o dijkstra sim *.o