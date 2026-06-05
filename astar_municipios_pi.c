/*
 * astar_municipios.c - A* with haversine heuristic over the municipalities of PI.
 *
 * Loads:
 *   nos.csv     -> codigo_ibge,name,lat,lon   (from queries/nos_municipios_pi.sql)
 *   arestas.csv -> source,target              (from queries/arestas_municipios_pi.sql)
 *
 * The weight of each edge does NOT come from the database: it is calculated here
 * using the same haversine function that generates the heuristic. This way, edge
 * weights and heuristic use the identical metric -> h(v) <= real cost by construction
 * -> A* is admissible/optimal.
 *
 * Border edges are symmetric: each pair (a,b) from the CSV becomes a->b and b->a.
 *
 * Compile: gcc -O2 -Wall -Wextra astar_municipios.c -o astar_municipios -lm
 * Run:     ./astar_municipios <source> <target> [nos.csv] [arestas.csv]
 *          source/target accept either codigo_ibge OR exact name.
 *
 * Author: Silas Vasconcelos Cruz (s-v7)
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXN   6000          /* teto de nos (PI tem 224; folga p/ outros UFs) */
#define INF    1e18
#define PI     3.14159265358979323846
#define NAMELEN 128
#define CODLEN  16

/* ----- nos ----- */
static char   cod[MAXN][CODLEN];
static char   nome[MAXN][NAMELEN];
static double lat[MAXN], lon[MAXN];
static int    n_nos = 0;

/* ----- arestas (lista de adjacencia) ----- */
typedef struct Edge {
	int		to;
	double		weight;
	struct Edge	*next;
} Edge;
static Edge *adj[MAXN];

static void add_edge(int u, int v, double w) {
	Edge *e = malloc(sizeof(*e));
	if(!e) { perror("malloc"); exit(EXIT_FAILURE); }
	e->to = v; e->weight = w; e->next = adj[u]; adj[u] = e;
}

static void free_adj(void) {
	for(int i = 0; i < n_nos; i++) {
		Edge *e = adj[i];
		while(e) { Edge *p = e->next; free(e); e = p; }
		adj[i] = NULL;
	}
}

/* ----- haversine (km): peso de aresta E heuristica ----- */
static double haversine(int a, int b) {
	const double R = 6371.0;
	double la1 = lat[a] * PI / 180.0, la2 = lat[b] * PI / 180.0;
	double dla = (lat[b] - lat[a]) * PI / 180.0;
	double dlo = (lon[b] - lon[a]) * PI / 180.0;
	double s = sin(dla/2)*sin(dla/2) + cos(la1)*cos(la2)*sin(dlo/2)*sin(dlo/2);
	return R * 2.0 * atan2(sqrt(s), sqrt(1.0 - s));
}

/* =========================================================================
 * Parser CSV minimo (RFC-4180): campos separados por virgula, opcionalmente
 * entre aspas duplas; "" dentro de campo aspado = aspa literal.
 * Preenche out[] com ponteiros para campos; retorna o numero de campos.
 * Modifica 'linha' in-place.
 * ========================================================================= */
static int csv_split(char *linha, char *out[], int max) {
	int nf = 0;
	char *p = linha;
	while(nf < max) {
		char *campo = p, *w = p;
		if(*p == '"') {                 /* campo entre aspas */
			p++; w = p; campo = p;
			char *dst = p;
			while(*p) {
				if(*p == '"') {
					if(*(p+1) == '"') { *dst++ = '"'; p += 2; }
					else { p++; break; }
				} else *dst++ = *p++;
			}
			*dst = '\0';
			while(*p && *p != ',') p++;  /* pula ate a virgula */
		} else {
			while(*p && *p != ',') p++;
			(void)w;
		}
		out[nf++] = campo;
		if(*p == ',') { *p = '\0'; p++; }
		else { *p = '\0'; break; }
	}
	return nf;
}

/* remove \r e \n do fim */
static void chomp(char *s) {
	size_t l = strlen(s);
	while(l && (s[l-1] == '\n' || s[l-1] == '\r')) s[--l] = '\0';
}

static int idx_por_codigo(const char *c) {
	for(int i = 0; i < n_nos; i++) if(strcmp(cod[i], c) == 0) return i;
	return -1;
}
static int idx_por_nome(const char *nm) {
	for(int i = 0; i < n_nos; i++) if(strcmp(nome[i], nm) == 0) return i;
	return -1;
}
/* aceita codigo OU nome */
static int resolver(const char *s) {
	int i = idx_por_codigo(s);
	return (i >= 0) ? i : idx_por_nome(s);
}

static void carregar_nos(const char *path) {
	FILE *f = fopen(path, "r");
	if(!f) { fprintf(stderr, "erro ao abrir %s\n", path); exit(EXIT_FAILURE); }
	char linha[1024];
	int primeira = 1;
	while(fgets(linha, sizeof(linha), f)) {
		chomp(linha);
		if(linha[0] == '\0') continue;
		if(primeira) { primeira = 0; continue; }  /* pula header */
		char *campo[8];
		int nf = csv_split(linha, campo, 8);
		if(nf < 4) continue;
		if(n_nos >= MAXN) { fprintf(stderr, "MAXN excedido\n"); break; }
		snprintf(cod[n_nos],  CODLEN,  "%s", campo[0]);
		snprintf(nome[n_nos], NAMELEN, "%s", campo[1]);
		lat[n_nos] = atof(campo[2]);
		lon[n_nos] = atof(campo[3]);
		n_nos++;
	}
	fclose(f);
}

static void carregar_arestas(const char *path) {
	FILE *f = fopen(path, "r");
	if(!f) { fprintf(stderr, "erro ao abrir %s\n", path); exit(EXIT_FAILURE); }
	char linha[1024];
	int primeira = 1, ignoradas = 0, total = 0;
	while(fgets(linha, sizeof(linha), f)) {
		chomp(linha);
		if(linha[0] == '\0') continue;
		if(primeira) { primeira = 0; continue; }
		char *campo[4];
		int nf = csv_split(linha, campo, 4);
		if(nf < 2) continue;
		int u = idx_por_codigo(campo[0]);
		int v = idx_por_codigo(campo[1]);
		if(u < 0 || v < 0) { ignoradas++; continue; }  /* codigo fora dos nos */
		double w = haversine(u, v);
		add_edge(u, v, w);     /* fronteira e simetrica: */
		add_edge(v, u, w);     /* adiciona os dois sentidos */
		total++;
	}
	fclose(f);
	if(ignoradas)
		fprintf(stderr, "aviso: %d arestas ignoradas (codigo sem no)\n", ignoradas);
	(void)total;
}

/* ----- min-heap binario com chave double f[] ----- */
typedef struct { int data[MAXN]; int pos[MAXN]; int size; } MinHeap;
#define PAI(p) (((p)-1)/2)
#define ESQ(p) (2*(p)+1)
#define DIR(p) (2*(p)+2)

static void heap_snap(MinHeap *h, int i, int j) {
	int t = h->data[i]; h->data[i] = h->data[j]; h->data[j] = t;
	h->pos[h->data[i]] = i; h->pos[h->data[j]] = j;
}
static void sift_up(MinHeap *h, int i, const double k[]) {
	while(i > 0 && k[h->data[i]] < k[h->data[PAI(i)]]) { heap_snap(h, i, PAI(i)); i = PAI(i); }
}
static void sift_down(MinHeap *h, int i, const double k[]) {
	for(;;) {
		int m = i, l = ESQ(i), r = DIR(i);
		if(l < h->size && k[h->data[l]] < k[h->data[m]]) m = l;
		if(r < h->size && k[h->data[r]] < k[h->data[m]]) m = r;
		if(m == i) break;
		heap_snap(h, i, m); i = m;
	}
}
static void heap_build(MinHeap *h, int n, const double k[]) {
	h->size = n;
	for(int v = 0; v < n; v++) { h->data[v] = v; h->pos[v] = v; }
	for(int w = n/2 - 1; w >= 0; w--) sift_down(h, w, k);
}
static int heap_empty(const MinHeap *h) { return h->size == 0; }
static int heap_extract_min(MinHeap *h, const double k[]) {
	int raiz = h->data[0]; h->pos[raiz] = -1; h->size--;
	if(h->size > 0) { h->data[0] = h->data[h->size]; h->pos[h->data[0]] = 0; sift_down(h, 0, k); }
	return raiz;
}
static void heap_decrease_key(MinHeap *h, int v, const double k[]) {
	int i = h->pos[v]; if(i >= 0) sift_up(h, i, k);
}
static int heap_contem(const MinHeap *h, int v) { return h->pos[v] >= 0; }

/* A* (usar_h=1) ou Dijkstra (usar_h=0). Retorna nos expandidos. */
static int buscar(int src, int goal, int usar_h, double g[], int parent[]) {
	double f[MAXN];
	MinHeap h;
	int expandidos = 0;
	for(int i = 0; i < n_nos; i++) { g[i] = INF; parent[i] = -1; f[i] = INF; }
	g[src] = 0.0;
	f[src] = usar_h ? haversine(src, goal) : 0.0;
	heap_build(&h, n_nos, f);
	while(!heap_empty(&h)) {
		int u = heap_extract_min(&h, f);
		if(g[u] >= INF) break;
		expandidos++;
		if(u == goal) break;
		for(Edge *e = adj[u]; e; e = e->next) {
			int v = e->to;
			double ng = g[u] + e->weight;
			if(heap_contem(&h, v) && ng < g[v]) {
				g[v] = ng; parent[v] = u;
				f[v] = usar_h ? ng + haversine(v, goal) : ng;
				heap_decrease_key(&h, v, f);
			}
		}
	}
	return expandidos;
}

static void imprime_path(int parent[], int v) {
	if(v == -1) return;
	if(parent[v] != -1) { imprime_path(parent, parent[v]); printf(" -> %s", nome[v]); }
	else printf("%s", nome[v]);
}

int main(int argc, char *argv[]) {
	const char *fnos = (argc >= 4) ? argv[3] : "nos.csv";
	const char *fare = (argc >= 5) ? argv[4] : "arestas.csv";

	if(argc < 3) {
		printf("Uso: %s <origem> <destino> [nos.csv] [arestas.csv]\n", argv[0]);
		printf("     origem/destino: codigo_ibge ou nome exato do municipio\n");
		return 1;
	}

	carregar_nos(fnos);
	carregar_arestas(fare);
	if(n_nos == 0) { fprintf(stderr, "nenhum no carregado\n"); return 1; }

	int src = resolver(argv[1]);
	int dst = resolver(argv[2]);
	if(src < 0 || dst < 0) {
		printf("Origem ou Destino invalido (use codigo_ibge ou nome exato).\n");
		free_adj();
		return -1;
	}

	double gA[MAXN], gD[MAXN];
	int parA[MAXN], parD[MAXN];
	int expA = buscar(src, dst, 1, gA, parA);
	int expD = buscar(src, dst, 0, gD, parD);

	if(gA[dst] >= INF) {
		printf("Nao existe caminho de %s para %s.\n", nome[src], nome[dst]);
	} else {
		printf("Rota minima de %s para %s: %.2f km\n", nome[src], nome[dst], gA[dst]);
		printf("Caminho: ");
		imprime_path(parA, dst);
		printf("\n");
		printf("Nos: %d municipios carregados | expansao A* = %d | Dijkstra = %d\n",
		       n_nos, expA, expD);
	}

	free_adj();
	return 0;
}
