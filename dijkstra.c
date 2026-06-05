#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 100
#define INF  1000000000

char *nomes[] = {
	"Deposito",
	"Filial Norte",
	"Filial Sul",
	"Hub Regional",
	"Cliente"
};

int indice_por_nome(char *nome, int n) {
	for(int i = 0; i < n; i++) {
		if(strcmp(nome, nomes[i]) == 0)
			return i;
	}
	return -1;
}

/* =========================================================================
 * Lista de adjacencia (ponteiros)
 * Cada vertice aponta para uma lista encadeada de arestas de saida.
 * Trocar a matriz por lista e o que permite varrer apenas as arestas reais,
 * baixando o custo de O(V²) para O(E log V) junto com o heap.
 * ========================================================================= */
typedef struct Edge {
    int		to;		/* vertice destino		*/
    int 	weight;		/* peso da aresta (>= 0)	*/
    struct Edge	*next;		/* proxima aresta de saida	*/
} Edge;

/* Insere a aresta dirigida u->v com peso w no inicio da lista de u. */
void add_edge(Edge *adj_lst[], int u, int v, int w) {
     Edge *rst = malloc(sizeof(*rst));
     if(!rst) { perror("malloc"); exit(EXIT_FAILURE); }
     rst->to = v;
     rst->weight = w;
     rst->next = adj_lst[u];
     adj_lst[u] = rst;
}

void free_adj(Edge *adj_lst_free[], int n){
    for(int x = 0; x < n; x++){
	    Edge *st = adj_lst_free[x];
	    while(st){
		    Edge *prox = st->next;
		    free(st);
		    st = prox;
	    }
	    adj_lst_free[x] = NULL;
    }
}

/* =========================================================================
 * Min-heap binario em vetor (aritmetica de indice: pai/filhos)
 * Guarda indices de vertices, ordenados pela chave dist[].
 * pos[v] = posicao de v no heap, ou -1 se v ja saiu / nunca entrou.
 * Suporta decrease_key em O(log V).
 * ========================================================================= */
typedef struct {
    int data[MAXN];  /* data[i] = id do vertice na posicao i  */
    int pos[MAXN];   /* pos[v]  = indice de v dentro de data[] */
    int size;
} MinHeap;

#define PAI(p) (((p) - 1) / 2)
#define ESQ(p) (2 * (p) + 1)
#define DIR(p) (2 * (p) + 2)

static void heap_snap(MinHeap *h, int i, int j){
    int tmp = h->data[i];
    h->data[i] = h->data[j];
    h->data[j] = tmp;
    h->pos[h->data[i]] = i;
    h->pos[h->data[j]] = j;
}

static void sift_up(MinHeap *h, int i, const int dist[]){
    while(i > 0 && dist[h->data[i]] < dist[h->data[PAI(i)]]) {
	    heap_snap(h, i, PAI(i));
	    i = PAI(i);            /* sobe um nivel: sem isto, so troca uma vez */
    }
}

static void sift_down(MinHeap *h, int i, const int dist[]){
    for(;;){
	    int menor = i, l = ESQ(i), r = DIR(i);
	    if(l < h->size && dist[h->data[l]] < dist[h->data[menor]]) menor = l;
	    if(r < h->size && dist[h->data[r]] < dist[h->data[menor]]) menor = r;
	    if(menor == i) break;
	    heap_snap(h, i, menor);   /* nome correto da funcao de troca */
	    i = menor;
    }
}

/* Constroi o heap com todos os n vertices e heapifica em O(n).
 * Heapificar de fato e essencial: com dist[src]=0 e o resto INF, a ordem de
 * identidade so seria um heap valido se src==0. O sift_down bottom-up garante
 * a propriedade de heap para qualquer distribuicao inicial de chaves. */
static void heap_build(MinHeap *h, int n, const int dist[]){
    h->size = n;
    for(int v = 0; v < n; v++){
	    h->data[v] = v;
	    h->pos[v] = v;
    }
    for(int w = n / 2 - 1; w >= 0; w--){
	    sift_down(h, w, dist);    /* usa w, o indice do laco */
    }
}

static int heap_empty(const MinHeap *h){
    return h->size == 0;
}

/* Remove e retorna o vertice de menor dist. */
static int heap_extract_min(MinHeap *h, const int dist[]){
    int raiz = h->data[0];
    h->pos[raiz] = -1;
    h->size--;
    if(h->size > 0) {
	    h->data[0] = h->data[h->size];
	    h->pos[h->data[0]] = 0;
	    sift_down(h, 0, dist);
    }
    return raiz;
}

/* Avisa o heap que dist[v] diminuiu; reordena subindo v. */
static void heap_decrease_key(MinHeap *h, int v, const int dist[]){
     int i = h->pos[v];
     if(i >= 0) sift_up(h, i, dist);
}

static int heap_contem(const MinHeap *h, int v){
    return h->pos[v] >= 0;
}

/* =========================================================================
 * Dijkstra com lista de adjacencia + min-heap: O(E log V)
 * Preenche dist[] e parent[]. Pesos >= 0.
 * ========================================================================= */
void dijkstra(int n, Edge *adj_lst[], int src, int dist[], int parent[]) {
    MinHeap h;

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[src] = 0;

    heap_build(&h, n, dist);

    while (!heap_empty(&h)) {
        int u = heap_extract_min(&h, dist);
        if (dist[u] == INF) break;      /* ninguem mais alcancavel */

        for (Edge *e = adj_lst[u]; e != NULL; e = e->next) {
            int v = e->to;
            int w = e->weight;
            if (heap_contem(&h, v) && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                heap_decrease_key(&h, v, dist);
            }
        }
    }
}

void imprime_path(int parent[], int v) {
    if (v == -1) return;
    if (parent[v] != -1) {
        imprime_path(parent, parent[v]);
        printf(" -> %s", nomes[v]);
    } else {
        printf("%s", nomes[v]);
    }
}

int main(int argc, char *argv[]) {
    int n = 5;
    Edge *adj_lst[MAXN] = {0};

    /* Mesmo grafo dirigido do original:
     * 0->1(2), 0->2(5), 1->2(1), 1->3(2), 2->3(3), 3->4(1), 2->4(10) */
    add_edge(adj_lst, 0, 1, 2);  add_edge(adj_lst, 0, 2, 5);
    add_edge(adj_lst, 1, 2, 1);  add_edge(adj_lst, 1, 3, 2);
    add_edge(adj_lst, 2, 3, 3);  add_edge(adj_lst, 3, 4, 1);  add_edge(adj_lst, 2, 4, 10);

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

    int dist[MAXN], parent[MAXN];
    dijkstra(n, adj_lst, src, dist, parent);

    if (dist[dst] >= INF) {
        printf("Nao existe caminho de %s para %s.\n", nomes[src], nomes[dst]);
    } else {
        printf("Distancia minima de %s para %s: %d\n", nomes[src], nomes[dst], dist[dst]);
        printf("Caminho: ");
        imprime_path(parent, dst);
        printf("\n");
    }

    free_adj(adj_lst, n);
    return 0;
}
