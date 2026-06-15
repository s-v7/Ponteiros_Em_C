/*
 * tsp_br.c - ENTREGA 2: otimizador de rota (TSP) sobre os municipios do Brasil.
 *
 * Recebe K paradas (>=3) e devolve a melhor ordem de visita. O custo entre
 * paradas e a DISTANCIA DE REDE (Dijkstra sobre o grafo de fronteiras), nao a
 * linha reta. Heuristica: nearest-neighbor + 2-opt + Or-opt. A origem (1a
 * parada) fica fixa. Ciclo fechado por padrao; --aberta nao retorna a origem.
 *
 * Uso:
 *   ./tsp_br [--json] [--aberta] [--nos F] [--arestas F] <p1> <p2> <p3> ...
 *   paradas: code_muni (7 digitos) ou "Nome/UF"
 * Ex.:
 *   ./tsp_br "Teresina/PI" "Picos/PI" "Floriano/PI" "Parnaiba/PI"
 *   ./tsp_br --aberta --json 2211001 2208007 2204006 2207702
 *
 * Compile: make
 * Author: s-v7 | License: MIT
 */
#include "grafo.h"
#include "heap.h"
#include "caminho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 1e18

/* ---------------- custo do tour ---------------- */
static double tour_cost(const int *order, int K, double **D, int fechado) {
	double c = 0.0;
	for (int i = 0; i < K - 1; i++) c += D[order[i]][order[i + 1]];
	if (fechado) c += D[order[K - 1]][order[0]];
	return c;
}

/* ---------------- nearest-neighbor (origem fixa em order[0]=0) -------------- */
static void nn_constroi(int *order, int K, double **D) {
	int *usado = calloc((size_t)K, sizeof(int));
	order[0] = 0; usado[0] = 1;
	for (int p = 1; p < K; p++) {
		int atual = order[p - 1], melhor = -1;
		double md = INF;
		for (int j = 0; j < K; j++)
			if (!usado[j] && D[atual][j] < md) { md = D[atual][j]; melhor = j; }
		order[p] = melhor; usado[melhor] = 1;
	}
	free(usado);
}

/* ---------------- 2-opt (mantem order[0] fixo) ---------------- */
static void two_opt(int *order, int K, double **D, int fechado) {
	int *cand = malloc((size_t)K * sizeof(int));
	int melhorou = 1;
	while (melhorou) {
		melhorou = 0;
		double atual = tour_cost(order, K, D, fechado);
		for (int i = 1; i < K - 1; i++) {
			for (int j = i + 1; j < K; j++) {
				memcpy(cand, order, (size_t)K * sizeof(int));
				for (int a = i, b = j; a < b; a++, b--) {   /* reverte [i..j] */
					int t = cand[a]; cand[a] = cand[b]; cand[b] = t;
				}
				double nc = tour_cost(cand, K, D, fechado);
				if (nc < atual - 1e-9) {
					memcpy(order, cand, (size_t)K * sizeof(int));
					atual = nc; melhorou = 1;
				}
			}
		}
	}
	free(cand);
}

/* ---------------- Or-opt: move segmento de tam 1..3 (mantem order[0] fixo) -- */
static void or_opt(int *order, int K, double **D, int fechado) {
	int *cand = malloc((size_t)K * sizeof(int));
	int *seg  = malloc((size_t)K * sizeof(int));
	int melhorou = 1;
	while (melhorou) {
		melhorou = 0;
		double atual = tour_cost(order, K, D, fechado);
		for (int L = 1; L <= 3 && L < K - 1; L++) {
			for (int i = 1; i + L <= K; i++) {              /* segmento [i..i+L-1] */
				memcpy(seg, order + i, (size_t)L * sizeof(int));
				/* resto sem o segmento */
				int m = 0;
				int *rest = malloc((size_t)(K - L) * sizeof(int));
				for (int k = 0; k < K; k++)
					if (k < i || k >= i + L) rest[m++] = order[k];
				/* tenta reinserir em cada gap (>=1 para nao mexer na origem) */
				for (int q = 1; q <= K - L; q++) {
					int c = 0;
					for (int k = 0; k < q; k++)        cand[c++] = rest[k];
					for (int k = 0; k < L; k++)        cand[c++] = seg[k];
					for (int k = q; k < K - L; k++)    cand[c++] = rest[k];
					double nc = tour_cost(cand, K, D, fechado);
					if (nc < atual - 1e-9) {
						memcpy(order, cand, (size_t)K * sizeof(int));
						atual = nc; melhorou = 1;
					}
				}
				free(rest);
			}
		}
	}
	free(cand); free(seg);
}

/* ---------------- reconstrucao de caminho (parent de Dijkstra) ------------- */
/* anexa em out (realloc) os nos do caminho stops[a]..stops[b], usando par[a].
 * se incluir_inicio=0, pula o primeiro no (evita duplicar a parada partilhada) */
static void anexa_caminho(int **out, int *n_out, int *cap_out,
                          int **par, const int *stops, int a, int b, int incluir_inicio) {
	int destino = stops[b], origem = stops[a];
	/* coleta destino..origem */
	int tmp_cap = 64, tn = 0;
	int *tmp = malloc((size_t)tmp_cap * sizeof(int));
	for (int x = destino; x != -1; x = par[a][x]) {
		if (tn >= tmp_cap) { tmp_cap *= 2; tmp = realloc(tmp, (size_t)tmp_cap * sizeof(int)); }
		tmp[tn++] = x;
		if (x == origem) break;
	}
	/* tmp esta de destino->origem; anexa em ordem origem->destino */
	for (int k = tn - 1; k >= 0; k--) {
		if (!incluir_inicio && k == tn - 1) continue;   /* pula origem repetida */
		if (*n_out >= *cap_out) { *cap_out *= 2; *out = realloc(*out, (size_t)(*cap_out) * sizeof(int)); }
		(*out)[(*n_out)++] = tmp[k];
	}
	free(tmp);
}

static int conta_saltos(int **par, const int *stops, int a, int b) {
	int saltos = 0;
	for (int x = stops[b]; x != -1 && x != stops[a]; x = par[a][x]) saltos++;
	return saltos;
}

/* ---------------- JSON ---------------- */
static void json_str(const char *s) {
	putchar('"');
	for (const char *p = s; *p; p++) { if (*p == '"' || *p == '\\') putchar('\\'); putchar(*p); }
	putchar('"');
}

int main(int argc, char *argv[]) {
	int json = 0, aberta = 0;
	const char *fnos = "data/nos_br.csv", *fare = "data/arestas_br.csv";
	const char *toks[256]; int nt = 0;

	for (int i = 1; i < argc; i++) {
		if      (!strcmp(argv[i], "--json"))    json = 1;
		else if (!strcmp(argv[i], "--aberta"))  aberta = 1;
		else if (!strcmp(argv[i], "--nos")     && i + 1 < argc) fnos = argv[++i];
		else if (!strcmp(argv[i], "--arestas") && i + 1 < argc) fare = argv[++i];
		else if (nt < 256) toks[nt++] = argv[i];
	}
	int fechado = !aberta;

	if (nt < 3) {
		fprintf(stderr, "Uso: %s [--json] [--aberta] [--nos F] [--arestas F] <p1> <p2> <p3> ...\n", argv[0]);
		fprintf(stderr, "     >= 3 paradas (code_muni ou \"Nome/UF\"). Para 2 pontos use astar_br.\n");
		return 1;
	}

	Grafo *g = grafo_load(fnos, fare);
	fprintf(stderr, "Grafo: %d municipios carregados.\n", g->n);

	int K = nt;
	int *stops = malloc((size_t)K * sizeof(int));
	for (int i = 0; i < K; i++) {
		stops[i] = grafo_resolver(g, toks[i]);
		if (stops[i] < 0) {
			fprintf(stderr, "Parada invalida: \"%s\"\n", toks[i]);
			if (json) printf("{\"erro\":\"parada_invalida\",\"parada\":"), json_str(toks[i]), printf("}\n");
			grafo_free(g); return -1;
		}
	}

	/* matriz de distancias de rede + parents por parada (p/ reconstrucao) */
	double **D = malloc((size_t)K * sizeof(double *));
	int **par = malloc((size_t)K * sizeof(int *));
	double *dist_tmp = malloc((size_t)g->n * sizeof(double));
	for (int i = 0; i < K; i++) {
		D[i]   = malloc((size_t)K * sizeof(double));
		par[i] = malloc((size_t)g->n * sizeof(int));
		dijkstra_full(g, stops[i], dist_tmp, par[i]);
		for (int j = 0; j < K; j++) D[i][j] = dist_tmp[stops[j]];
	}
	free(dist_tmp);

	/* checa conectividade entre as paradas */
	for (int i = 0; i < K; i++)
		for (int j = 0; j < K; j++)
			if (D[i][j] >= INF) {
				fprintf(stderr, "Sem caminho entre %s/%s e %s/%s.\n",
				        g->nome[stops[i]], g->uf[stops[i]], g->nome[stops[j]], g->uf[stops[j]]);
				if (json) printf("{\"erro\":\"paradas_desconectadas\"}\n");
				grafo_free(g); return -1;
			}

	/* heuristica: NN -> 2-opt -> Or-opt */
	int *order = malloc((size_t)K * sizeof(int));
	nn_constroi(order, K, D);
	double custo_nn = tour_cost(order, K, D, fechado);
	two_opt(order, K, D, fechado);
	or_opt(order, K, D, fechado);
	two_opt(order, K, D, fechado);   /* 2a passada: Or-opt pode reabrir 2-opt */
	double custo_fim = tour_cost(order, K, D, fechado);

	/* caminho completo (municipios) concatenando as pernas */
	int cap = 256, n_path = 0;
	int *path = malloc((size_t)cap * sizeof(int));
	int n_pernas = fechado ? K : K - 1;
	for (int p = 0; p < n_pernas; p++) {
		int a = order[p], b = order[(p + 1) % K];
		anexa_caminho(&path, &n_path, &cap, par, stops, a, b, p == 0);
	}

	double reducao = custo_nn > 0 ? (custo_nn - custo_fim) / custo_nn * 100.0 : 0.0;

	if (json) {
		printf("{\"tipo\":\"tsp\",\"fechado\":%s,", fechado ? "true" : "false");
		printf("\"km_total\":%.1f,\"km_inicial_nn\":%.1f,\"reducao_percent\":%.1f,", custo_fim, custo_nn, reducao);
		printf("\"paradas\":[");
		for (int p = 0; p < K; p++) {
			int s = stops[order[p]];
			if (p) printf(",");
			printf("{\"code\":"); json_str(g->cod[s]);
			printf(",\"nome\":"); json_str(g->nome[s]);
			printf(",\"uf\":");   json_str(g->uf[s]); printf("}");
		}
		printf("],\"legs\":[");
		for (int p = 0; p < n_pernas; p++) {
			int a = order[p], b = order[(p + 1) % K];
			char de[256], pa[256];
			snprintf(de, sizeof(de), "%s/%s", g->nome[stops[a]], g->uf[stops[a]]);
			snprintf(pa, sizeof(pa), "%s/%s", g->nome[stops[b]], g->uf[stops[b]]);
			if (p) printf(",");
			printf("{\"de\":"); json_str(de);
			printf(",\"para\":"); json_str(pa);
			printf(",\"km\":%.1f,\"saltos\":%d}", D[a][b], conta_saltos(par, stops, a, b));
		}
		printf("],\"caminho_completo\":[");
		for (int i = 0; i < n_path; i++) {
			char buf[256];
			snprintf(buf, sizeof(buf), "%s/%s", g->nome[path[i]], g->uf[path[i]]);
			if (i) printf(",");
			json_str(buf);
		}
		printf("],\"classificacao\":\"publica\"}\n");
	} else {
		printf("Otimizacao de rota (%s) sobre %d paradas:\n", fechado ? "ciclo fechado" : "rota aberta", K);
		printf("Ordem: ");
		for (int p = 0; p < K; p++) {
			int s = stops[order[p]];
			printf("%s/%s%s", g->nome[s], g->uf[s], p < K - 1 ? " -> " : "");
		}
		if (fechado) printf(" -> (volta) %s/%s", g->nome[stops[order[0]]], g->uf[stops[order[0]]]);
		printf("\n");
		printf("Distancia: NN inicial = %.1f km -> otimizado = %.1f km (reducao %.1f%%)\n",
		       custo_nn, custo_fim, reducao);
		printf("Municipios no trajeto completo: %d\n", n_path);
	}

	for (int i = 0; i < K; i++) { free(D[i]); free(par[i]); }
	free(D); free(par); free(order); free(stops); free(path);
	grafo_free(g);
	return 0;
}
