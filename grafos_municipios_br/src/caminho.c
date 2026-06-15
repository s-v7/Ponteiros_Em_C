/*
 * caminho.c - Dijkstra 1->todos (ver caminho.h). Reusa o min-heap.
 * Author: s-v7 | License: MIT
 */
#include "caminho.h"
#include "heap.h"
#include <stdlib.h>

#define INF 1e18

void dijkstra_full(const Grafo *g, int src, double *dist, int *parent) {
	int n = g->n;
	for (int i = 0; i < n; i++) { dist[i] = INF; parent[i] = -1; }
	dist[src] = 0.0;

	MinHeap h;
	heap_init(&h, n, dist);
	heap_build_all(&h, n);

	while (!heap_empty(&h)) {
		int u = heap_extract_min(&h);
		if (dist[u] >= INF) break;
		for (Edge *e = g->adj[u]; e; e = e->next) {
			int v = e->to;
			double nd = dist[u] + e->w;
			if (heap_contains(&h, v) && nd < dist[v]) {
				dist[v] = nd;
				parent[v] = u;
				heap_decrease_key(&h, v);
			}
		}
	}
	heap_free(&h);
}
