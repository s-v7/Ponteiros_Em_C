/*
 * astar_br.c - ENTREGA 1: A* ponto-a-ponto sobre os municipios do Brasil.
 *
 * Le data/nos_br.csv + data/arestas_br.csv (formato geobr), monta o grafo e
 * roteia com A* (heuristica haversine, admissivel) entre dois municipios.
 * Aceita origem/destino por code_muni OU "Nome/UF".
 *
 * Saida:
 *   - modo humano (default): texto legivel em stdout
 *   - modo --json: contrato JSON em stdout (logs vao p/ stderr -> pipe limpo).
 *     Ex.: ./astar_br --json 2211001 4314902 | node integ/openai.js
 *
 * Compile: make
 * Author: s-v7 | License: MIT
 */
#include "grafo.h"
#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 1e18

static int buscar(const Grafo *g, int src, int goal, int usar_h,
                  double *gcost, int *parent) {
	int n = g->n;
	double *f = malloc((size_t)n * sizeof(double));
	if (!f) { perror("malloc"); exit(EXIT_FAILURE); }

	for (int i = 0; i < n; i++) { gcost[i] = INF; parent[i] = -1; f[i] = INF; }
	gcost[src] = 0.0;
	f[src] = usar_h ? haversine(g, src, goal) : 0.0;

	MinHeap h;
	heap_init(&h, n, f);
	heap_build_all(&h, n);

	int expandidos = 0;
	while (!heap_empty(&h)) {
		int u = heap_extract_min(&h);
		if (gcost[u] >= INF) break;
		expandidos++;
		if (u == goal) break;
		for (Edge *e = g->adj[u]; e; e = e->next) {
			int v = e->to;
			double ng = gcost[u] + e->w;
			if (heap_contains(&h, v) && ng < gcost[v]) {
				gcost[v] = ng;
				parent[v] = u;
				f[v] = usar_h ? ng + haversine(g, v, goal) : ng;
				heap_decrease_key(&h, v);
			}
		}
	}
	heap_free(&h);
	free(f);
	return expandidos;
}

/* reconstroi o caminho src..v em out[] (ordem origem->destino); retorna o tamanho */
static int build_path(const int *parent, int v, int *out, int cap) {
	int tmp[8192], topo = 0;
	for (int x = v; x != -1 && topo < (int)(sizeof(tmp)/sizeof(tmp[0])); x = parent[x])
		tmp[topo++] = x;
	int m = 0;
	for (int i = topo - 1; i >= 0 && m < cap; i--) out[m++] = tmp[i];
	return m;
}

static void imprime_path(const Grafo *g, const int *parent, int v) {
	int *seq = malloc((size_t)g->n * sizeof(int));
	if (!seq) { perror("malloc"); exit(EXIT_FAILURE); }
	int m = build_path(parent, v, seq, g->n);
	for (int i = 0; i < m; i++)
		printf("%s/%s%s", g->nome[seq[i]], g->uf[seq[i]], i < m - 1 ? " -> " : "");
	printf("\n");
	free(seq);
}

/* imprime uma string como literal JSON (escapa " e \) */
static void json_str(const char *s) {
	putchar('"');
	for (const char *p = s; *p; p++) {
		if (*p == '"' || *p == '\\') putchar('\\');
		putchar(*p);
	}
	putchar('"');
}

/* emite o contrato JSON da rota em stdout */
static void emite_json(const Grafo *g, int src, int dst,
                       const double *gcost, const int *parent, int expA, int expD) {
	printf("{");
	printf("\"origem\":{\"code\":"); json_str(g->cod[src]);
	printf(",\"nome\":"); json_str(g->nome[src]);
	printf(",\"uf\":"); json_str(g->uf[src]); printf("},");
	printf("\"destino\":{\"code\":"); json_str(g->cod[dst]);
	printf(",\"nome\":"); json_str(g->nome[dst]);
	printf(",\"uf\":"); json_str(g->uf[dst]); printf("},");

	if (gcost[dst] >= INF) {
		printf("\"caminho\":[],\"km_total\":null,\"saltos\":0,\"estados\":[],");
		printf("\"expansao\":{\"astar\":%d,\"dijkstra\":%d},", expA, expD);
		printf("\"erro\":\"sem_caminho\",\"classificacao\":\"publica\"}");
		printf("\n");
		return;
	}

	int *seq = malloc((size_t)g->n * sizeof(int));
	if (!seq) { perror("malloc"); exit(EXIT_FAILURE); }
	int m = build_path(parent, dst, seq, g->n);

	printf("\"km_total\":%.1f,\"saltos\":%d,", gcost[dst], m - 1);

	/* estados unicos na ordem de aparicao */
	printf("\"estados\":[");
	int primeiro = 1;
	char vistos[64][8]; int nv = 0;
	for (int i = 0; i < m; i++) {
		const char *uf = g->uf[seq[i]];
		int achou = 0;
		for (int k = 0; k < nv; k++) if (strcmp(vistos[k], uf) == 0) { achou = 1; break; }
		if (achou) continue;
		if (nv < 64) snprintf(vistos[nv++], 8, "%s", uf);
		if (!primeiro) printf(",");
		json_str(uf); primeiro = 0;
	}
	printf("],");

	printf("\"expansao\":{\"astar\":%d,\"dijkstra\":%d},", expA, expD);

	printf("\"caminho\":[");
	for (int i = 0; i < m; i++) {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s/%s", g->nome[seq[i]], g->uf[seq[i]]);
		if (i) printf(",");
		json_str(buf);
	}
	printf("],");
	printf("\"classificacao\":\"publica\"}");
	printf("\n");
	free(seq);
}

static int resolver_ou_erro(const Grafo *g, const char *token, const char *papel) {
	int idx = grafo_resolver(g, token);
	if (idx == RESOLVE_NAO_ENCONTRADO)
		fprintf(stderr, "%s invalido: \"%s\" nao encontrado.\n", papel, token);
	return idx;
}

int main(int argc, char *argv[]) {
	int json = 0;
	const char *pos[4]; int np = 0;          /* args posicionais (sem flags) */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--json") == 0) json = 1;
		else if (np < 4) pos[np++] = argv[i];
	}

	const char *fnos = (np >= 3) ? pos[2] : "data/nos_br.csv";
	const char *fare = (np >= 4) ? pos[3] : "data/arestas_br.csv";

	if (np < 2) {
		fprintf(stderr, "Uso: %s [--json] <origem> <destino> [nos.csv] [arestas.csv]\n", argv[0]);
		fprintf(stderr, "     origem/destino: code_muni (7 digitos) ou \"Nome/UF\"\n");
		return 1;
	}

	Grafo *g = grafo_load(fnos, fare);
	fprintf(stderr, "Grafo: %d municipios carregados.\n", g->n);

	int src = resolver_ou_erro(g, pos[0], "Origem");
	int dst = resolver_ou_erro(g, pos[1], "Destino");
	if (src < 0 || dst < 0) {
		if (json) printf("{\"erro\":\"origem_ou_destino_invalido\"}\n");
		grafo_free(g);
		return -1;
	}

	double *gA = malloc((size_t)g->n * sizeof(double));
	double *gD = malloc((size_t)g->n * sizeof(double));
	int *parA = malloc((size_t)g->n * sizeof(int));
	int *parD = malloc((size_t)g->n * sizeof(int));
	if (!gA || !gD || !parA || !parD) { perror("malloc"); return EXIT_FAILURE; }

	int expA = buscar(g, src, dst, 1, gA, parA);
	int expD = buscar(g, src, dst, 0, gD, parD);

	if (json) {
		emite_json(g, src, dst, gA, parA, expA, expD);
	} else if (gA[dst] >= INF) {
		printf("Nao existe caminho de %s/%s para %s/%s.\n",
		       g->nome[src], g->uf[src], g->nome[dst], g->uf[dst]);
		printf("(possivel no isolado, ex.: ilha sem fronteira terrestre)\n");
	} else {
		printf("Rota minima de %s/%s para %s/%s: %.1f km\n",
		       g->nome[src], g->uf[src], g->nome[dst], g->uf[dst], gA[dst]);
		printf("Caminho: ");
		imprime_path(g, parA, dst);
		printf("Nos: %d municipios | expansao A* = %d | Dijkstra = %d\n",
		       g->n, expA, expD);
	}

	free(gA); free(gD); free(parA); free(parD);
	grafo_free(g);
	return 0;
}
