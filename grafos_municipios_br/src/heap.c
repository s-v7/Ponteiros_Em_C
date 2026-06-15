/*
 * heap.c - Min-heap binario (ver heap.h).
 * Author: s-v7 | License: MIT
 */
#include "heap.h"
#include <stdlib.h>
#include <stdio.h>

#define PAI(i) (((i) - 1) / 2)
#define ESQ(i) (2 * (i) + 1)
#define DIR(i) (2 * (i) + 2)

void heap_init(MinHeap *h, int cap, const double *key) {
	h->data = malloc((size_t)cap * sizeof(int));
	h->pos  = malloc((size_t)cap * sizeof(int));
	if (!h->data || !h->pos) { perror("malloc heap"); exit(EXIT_FAILURE); }
	h->size = 0;
	h->cap  = cap;
	h->key  = key;
	for (int v = 0; v < cap; v++) h->pos[v] = -1;
}

void heap_free(MinHeap *h) {
	free(h->data); free(h->pos);
	h->data = h->pos = NULL; h->size = h->cap = 0;
}

static void troca(MinHeap *h, int i, int j) {
	int t = h->data[i];
	h->data[i] = h->data[j];
	h->data[j] = t;
	h->pos[h->data[i]] = i;
	h->pos[h->data[j]] = j;
}

static void sift_up(MinHeap *h, int i) {
	const double *k = h->key;
	while (i > 0 && k[h->data[i]] < k[h->data[PAI(i)]]) {
		troca(h, i, PAI(i));
		i = PAI(i);
	}
}

static void sift_down(MinHeap *h, int i) {
	const double *k = h->key;
	for (;;) {
		int menor = i, l = ESQ(i), r = DIR(i);
		if (l < h->size && k[h->data[l]] < k[h->data[menor]]) menor = l;
		if (r < h->size && k[h->data[r]] < k[h->data[menor]]) menor = r;
		if (menor == i) break;
		troca(h, i, menor);
		i = menor;
	}
}

void heap_build_all(MinHeap *h, int n) {
	h->size = n;
	for (int v = 0; v < n; v++) { h->data[v] = v; h->pos[v] = v; }
	for (int i = n / 2 - 1; i >= 0; i--) sift_down(h, i);
}

int heap_empty(const MinHeap *h) { return h->size == 0; }

int heap_extract_min(MinHeap *h) {
	int raiz = h->data[0];
	h->pos[raiz] = -1;
	h->size--;
	if (h->size > 0) {
		h->data[0] = h->data[h->size];
		h->pos[h->data[0]] = 0;
		sift_down(h, 0);
	}
	return raiz;
}

void heap_decrease_key(MinHeap *h, int v) {
	int i = h->pos[v];
	if (i >= 0) sift_up(h, i);
}

int heap_contains(const MinHeap *h, int v) { return h->pos[v] >= 0; }
