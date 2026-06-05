#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXN 100
#define INF  1e18
#define PI   3.14159265358979323846

char *nomes[] = {
	"Deposito",
	"Filial Norte",
	"Filial Sul",
	"Hub Regional",
	"Cliente"
};

/* Coordenadas (lat, lon) aproximadas em Teresina-PI.
 * Sao elas que dao ao grafo a estrutura geometrica: a partir daqui o produto
 * interno de R^2 induz a norma, a norma a metrica, e a metrica a heuristica. */
double coord[][2] = {
	{-5.0892, -42.8019},  /* Deposito     */
	{-5.0400, -42.7900},  /* Filial Norte */
	{-5.1500, -42.7800},  /* Filial Sul   */
	{-5.0900, -42.7600},  /* Hub Regional */
	{-5.0600, -42.7300},  /* Cliente      */
};

int indice_por_nome(char *nome, int n) {
	for(int i = 0; i < n; i++) {
		if(strcmp(nome, nomes[i]) == 0)
			return i;
	}
	return -1;
}

/* Distancia de grande-circulo (km) entre dois vertices, via haversine.
 * E uma metrica: satisfaz a desigualdade triangular, o que torna a
 * heuristica admissivel (nunca superestima o custo real ate o destino). */
static double haversine(int a, int b) {
	const double R = 6371.0;
	double lat1 = coord[a][0] * PI / 180.0;
	double lat2 = coord[b][0] * PI / 180.0;
	double dlat = (coord[b][0] - coord[a][0]) * PI / 180.0;
	double dlon = (coord[b][1] - coord[a][1]) * PI / 180.0;
	double s = sin(dlat / 2) * sin(dlat / 2) +
	           cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
	return R * 2.0 * atan2(sqrt(s), sqrt(1.0 - s));
}

/* ----- lista de adjacencia (ponteiros), peso real em km ----- */
typedef struct Edge {
	int		to;
	double		weight;
	struct Edge	*next;
} Edge;

void add_edge(Edge *adj_lst[], int u, int v, double w) {
	Edge *rst = malloc(sizeof(*rst));
	if(!rst) { perror("malloc"); exit(EXIT_FAILURE); }
	rst->to = v;
	rst->weight = w;
	rst->next = adj_lst[u];
	adj_lst[u] = rst;
}

void free_adj(Edge *adj_lst_free[], int n) {
	for(int x = 0; x < n; x++) {
		Edge *st = adj_lst_free[x];
		while(st) {
			Edge *prox = st->next;
			free(st);
			st = prox;
		}
		adj_lst_free[x] = NULL;
	}
}

/* ----- min-heap binario, agora com chave double f[] (= g + h) ----- */
typedef struct {
	int data[MAXN];
	int pos[MAXN];
	int size;
} MinHeap;

#define PAI(p) (((p) - 1) / 2)
#define ESQ(p) (2 * (p) + 1)
#define DIR(p) (2 * (p) + 2)

static void heap_snap(MinHeap *h, int i, int j) {
	int tmp = h->data[i];
	h->data[i] = h->data[j];
	h->data[j] = tmp;
	h->pos[h->data[i]] = i;
	h->pos[h->data[j]] = j;
}

static void sift_up(MinHeap *h, int i, const double key[]) {
	while(i > 0 && key[h->data[i]] < key[h->data[PAI(i)]]) {
		heap_snap(h, i, PAI(i));
		i = PAI(i);
	}
}

static void sift_down(MinHeap *h, int i, const double key[]) {
	for(;;) {
		int menor = i, l = ESQ(i), r = DIR(i);
		if(l < h->size && key[h->data[l]] < key[h->data[menor]]) menor = l;
		if(r < h->size && key[h->data[r]] < key[h->data[menor]]) menor = r;
		if(menor == i) break;
		heap_snap(h, i, menor);
		i = menor;
	}
}

static void heap_build(MinHeap *h, int n, const double key[]) {
	h->size = n;
	for(int v = 0; v < n; v++) {
		h->data[v] = v;
		h->pos[v] = v;
	}
	for(int w = n / 2 - 1; w >= 0; w--)
		sift_down(h, w, key);
}

static int heap_empty(const MinHeap *h) { return h->size == 0; }

static int heap_extract_min(MinHeap *h, const double key[]) {
	int raiz = h->data[0];
	h->pos[raiz] = -1;
	h->size--;
	if(h->size > 0) {
		h->data[0] = h->data[h->size];
		h->pos[h->data[0]] = 0;
		sift_down(h, 0, key);
	}
	return raiz;
}

static void heap_decrease_key(MinHeap *h, int v, const double key[]) {
	int i = h->pos[v];
	if(i >= 0) sift_up(h, i, key);
}

static int heap_contem(const MinHeap *h, int v) { return h->pos[v] >= 0; }

/* =========================================================================
 * Busca com heap.  usar_heuristica = 1 -> A* (f = g + h)
 *                  usar_heuristica = 0 -> Dijkstra puro (f = g, h = 0)
 * Mesmo codigo: o Dijkstra e o caso particular do A* com h == 0.
 * Preenche g[] (custo real) e parent[]; retorna nº de nos expandidos.
 * ========================================================================= */
int buscar(int n, Edge *adj_lst[], int src, int goal,
           int usar_heuristica, double g[], int parent[]) {
	double f[MAXN];
	MinHeap h;
	int expandidos = 0;

	for(int i = 0; i < n; i++) {
		g[i] = INF;
		parent[i] = -1;
		f[i] = INF;
	}
	g[src] = 0.0;
	f[src] = usar_heuristica ? haversine(src, goal) : 0.0;

	heap_build(&h, n, f);

	while(!heap_empty(&h)) {
		int u = heap_extract_min(&h, f);
		if(g[u] >= INF) break;        /* inalcancavel */
		expandidos++;
		if(u == goal) break;          /* heuristica consistente: pode parar */

		for(Edge *e = adj_lst[u]; e != NULL; e = e->next) {
			int v = e->to;
			double ng = g[u] + e->weight;
			if(heap_contem(&h, v) && ng < g[v]) {
				g[v] = ng;
				parent[v] = u;
				f[v] = usar_heuristica ? ng + haversine(v, goal) : ng;
				heap_decrease_key(&h, v, f);
			}
		}
	}
	return expandidos;
}

void imprime_path(int parent[], int v) {
	if(v == -1) return;
	if(parent[v] != -1) {
		imprime_path(parent, parent[v]);
		printf(" -> %s", nomes[v]);
	} else {
		printf("%s", nomes[v]);
	}
}

int main(int argc, char *argv[]) {
	int n = 5;
	Edge *adj_lst[MAXN] = {0};

	/* Mesma topologia dirigida do exemplo do Dijkstra, mas os pesos agora
	 * sao distancias reais (km) entre os pontos -> heuristica admissivel. */
	int arestas[][2] = { {0,1},{0,2},{1,2},{1,3},{2,3},{3,4},{2,4} };
	int n_arestas = sizeof(arestas) / sizeof(arestas[0]);
	for(int i = 0; i < n_arestas; i++) {
		int u = arestas[i][0], v = arestas[i][1];
		add_edge(adj_lst, u, v, haversine(u, v));
	}

	if(argc < 3) {
		printf("Uso: %s <origem> <destino>\n", argv[0]);
		free_adj(adj_lst, n);
		return 1;
	}

	int src = indice_por_nome(argv[1], n);
	int dst = indice_por_nome(argv[2], n);
	if(src == -1 || dst == -1) {
		printf("Origem ou Destino invalido.\n");
		free_adj(adj_lst, n);
		return -1;
	}

	double gA[MAXN], gD[MAXN];
	int parA[MAXN], parD[MAXN];
	int expA = buscar(n, adj_lst, src, dst, 1, gA, parA);  /* A*       */
	int expD = buscar(n, adj_lst, src, dst, 0, gD, parD);  /* Dijkstra */

	if(gA[dst] >= INF) {
		printf("Nao existe caminho de %s para %s.\n", nomes[src], nomes[dst]);
	} else {
		printf("Distancia minima de %s para %s: %.2f km\n",
		       nomes[src], nomes[dst], gA[dst]);
		printf("Caminho: ");
		imprime_path(parA, dst);
		printf("\n");
		printf("Expansao de nos: A* = %d | Dijkstra = %d "
		       "(mesmo otimo, A* explora menos)\n", expA, expD);
	}

	free_adj(adj_lst, n);
	return 0;
}
