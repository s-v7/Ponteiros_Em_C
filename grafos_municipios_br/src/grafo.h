/*
 * grafo.h - Grafo de municipios carregado de CSV (formato geobr).
 *
 *   nos_br.csv     : code_muni,nome,uf,lat,lon
 *   arestas_br.csv : origem,destino            (pares de code_muni)
 *
 * Arestas de fronteira sao simetricas -> cada par vira u->v e v->u.
 * O peso e calculado aqui por haversine (mesma metrica da heuristica do A*).
 *
 * Author: s-v7 | License: MIT
 */
#ifndef GRAFO_H
#define GRAFO_H

typedef struct Edge {
	int           to;
	double        w;
	struct Edge  *next;
} Edge;

typedef struct {
	int      n;        /* numero de nos                          */
	char   **cod;      /* code_muni (string de 7 digitos)        */
	char   **nome;     /* nome do municipio                      */
	char   **uf;       /* sigla da UF                            */
	double  *lat;
	double  *lon;
	Edge   **adj;      /* listas de adjacencia                   */
} Grafo;

Grafo *grafo_load(const char *nos_csv, const char *arestas_csv);
void   grafo_free(Grafo *g);

double haversine(const Grafo *g, int a, int b);   /* km entre dois nos */

int grafo_idx_por_codigo(const Grafo *g, const char *cod);
int grafo_idx_por_nome_uf(const Grafo *g, const char *nome, const char *uf);

/* Resolve um token de CLI:
 *   - so digitos        -> code_muni
 *   - "Nome/UF"         -> nome qualificado por estado
 *   - "Nome"            -> nome; se houver homonimos em UFs diferentes,
 *                          retorna AMBIGUO (-2) e imprime os candidatos.
 * Retorna indice >=0, -1 (nao encontrado) ou -2 (ambiguo). */
#define RESOLVE_NAO_ENCONTRADO (-1)
#define RESOLVE_AMBIGUO        (-2)
int grafo_resolver(const Grafo *g, const char *token);

#endif /* GRAFO_H */
