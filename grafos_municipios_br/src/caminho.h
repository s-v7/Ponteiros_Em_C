/*
 * caminho.h - Pathfinding reutilizavel sobre o Grafo (Dijkstra 1->todos).
 * Author: s-v7 | License: MIT
 */
#ifndef CAMINHO_H
#define CAMINHO_H
#include "grafo.h"

/* Dijkstra de fonte unica. Preenche dist[n] e parent[n] (buffers do chamador).
 * dist[v] = INF (1e18) se inalcancavel; parent[src] = -1. */
void dijkstra_full(const Grafo *g, int src, double *dist, int *parent);

#endif /* CAMINHO_H */
