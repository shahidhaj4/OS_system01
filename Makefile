CC = gcc
CFLAGS = -Wall -Wextra -std=c11

.PHONY: milestone2 clean

milestone2:
	$(CC) $(CFLAGS) -o sim Main.c graph.c file_reader.c dijkstra.c Dijkstra_draw.c library.c -lraylib -lm -lX11 -lpthread -ldl -lrt

clean:
	rm -f sim dijkstra *.o
