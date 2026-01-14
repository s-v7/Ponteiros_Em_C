#include <stdio.h>
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

// Retorna o índice do vértice com menor dist não visitado
int extrai_min(int n, int dist[], int vis[]) {
    int idx = -1, best = INF;
    for (int i = 0; i < n; i++) {
        if (!vis[i] && dist[i] < best) {
            best = dist[i];
            idx = i;
        }
    }
    return idx;
}

// Dijkstra O(n²) para matriz de adjacência
// graph[i][j] = peso (>=0) da aresta i->j; use 0 para ausência de aresta (i!=j)
void dijkstra(int n, int graph[MAXN][MAXN], int src, int dist[], int parent[]) {
    int vis[MAXN] = {0};

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[src] = 0;

    for (int k = 0; k < n; k++) {
        int u = extrai_min(n, dist, vis);
        if (u == -1) break;      // ninguém mais alcançável
        vis[u] = 1;

        for (int v = 0; v < n; v++) {
            int w = graph[u][v];
            if (!vis[v] && w > 0 && dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
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
    int G[MAXN][MAXN] = {0};

    // Exemplo dirigido:
    // 0->1(2), 0->2(5), 1->2(1), 1->3(2), 2->3(3), 3->4(1), 2->4(10)
    G[0][1] = 2;  G[0][2] = 5;
    G[1][2] = 1;  G[1][3] = 2;
    G[2][3] = 3;  G[3][4] = 1;  G[2][4] = 10;

    if(argc < 3) {
	    printf("Uso: %s <origem> <destino>\n", argv[0]);
	    return 1;
    }

    int src = indice_por_nome(argv[1], n);
    int dst = indice_por_nome(argv[2], n);
    
    if(src == -1 || dst == -1) {
	    printf("Origem ou Destino invalido.\n");
	    return -1;
    }

    int dist[MAXN], parent[MAXN];
    dijkstra(n, G, src, dist, parent);

    if (dist[dst] >= INF) {
        printf("Nao existe caminho de %s para %s.\n", nomes[src], nomes[dst]);
    } else {
        printf("Distancia minima de %s para %s: %d\n", nomes[src], nomes[dst], dist[dst]);
        printf("Caminho: ");
        imprime_path(parent, dst);
        printf("\n");
    }
    return 0;
}

