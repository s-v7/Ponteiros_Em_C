CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
LDFLAGS = -lm
COMMON  = src/grafo.c src/heap.c
HDR     = src/grafo.h src/heap.h src/caminho.h

all: astar_br tsp_br

astar_br: src/astar_br.c $(COMMON) $(HDR)
	$(CC) $(CFLAGS) src/astar_br.c $(COMMON) -o $@ $(LDFLAGS)

tsp_br: src/tsp_br.c $(COMMON) src/caminho.c $(HDR)
	$(CC) $(CFLAGS) src/tsp_br.c $(COMMON) src/caminho.c -o $@ $(LDFLAGS)

debug: src/tsp_br.c $(COMMON) src/caminho.c $(HDR)
	$(CC) -O1 -g -fsanitize=address,undefined $(CFLAGS) src/tsp_br.c $(COMMON) src/caminho.c -o tsp_br_dbg $(LDFLAGS)

clean:
	rm -f astar_br tsp_br tsp_br_dbg astar_br_dbg

.PHONY: all clean debug
