/*
 * heap.h --- Min-Heap binário reutilizavél, chave double externa
 *
 * Guarda indices de vertices ordenados por uma chave externa key[v]
 * (dist[] no Dijkstra, f[]=g+h no A*). Aritmética de índice pai/filhos;
 * pos[v] mapeia vertice -> posição, permitindo decrease key em O(log n);
 * Arraysd dimencionados em runtime (sem teto fixo) -> escala nacional.
 *
 * Author: s-v7 | License: MIT
 *
 */

#ifndef HEAP_H
#define	HEAP_H

typedef struct {
     int		*data;	/* data[i] = id do vertice na posição i */
     int 		*pos;	/* post[v] = índice de v em data, ou -1 */
     int 		size;
     int		cap;
     const double 	*key;
} MinHeap;

/* Initializa para até `cap` vertices, comparando pela chave externa `key`. */
void heap_init(MinHeap *h_init, int cap, const double *k);
void heap_free(MinHeap *h_free);

/* Carrega os vertices até `cap` vertices, comparando pela chave externa `key`. */
void heap_build_all(MinHeap *h_build_all, int n);

int heap_empty(const MinHeap *h_empty);
int heap_extract_min(MinHeap *h_extract_min);	/* remove e retorna o de menor key */
void heap_decrease_key(MinHeap *h_decrease_key, int v); /* avisa que key[v] diminuiu	*/
int heap_contains(const MinHeap *h_contains, int v);

#endif /*HEAP_H */

