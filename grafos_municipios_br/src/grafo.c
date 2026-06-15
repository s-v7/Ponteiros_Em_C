/*
 * grafo.c - Grafo de municipios (ver grafo.h).
 * Author: s-v7 | License: MIT
 */
#include "grafo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846

/* ---------- CSV minimo (RFC-4180), modifica a linha in-place ---------- */
static int csv_split(char *linha, char *out[], int max) {
	int nf = 0;
	char *p = linha;
	while (nf < max) {
		char *campo = p;
		if (*p == '"') {
			p++; campo = p; char *dst = p;
			while (*p) {
				if (*p == '"') {
					if (*(p + 1) == '"') { *dst++ = '"'; p += 2; }
					else { p++; break; }
				} else *dst++ = *p++;
			}
			*dst = '\0';
			while (*p && *p != ',') p++;
		} else {
			while (*p && *p != ',') p++;
		}
		out[nf++] = campo;
		if (*p == ',') { *p = '\0'; p++; }
		else { *p = '\0'; break; }
	}
	return nf;
}

static void chomp(char *s) {
	size_t l = strlen(s);
	while (l && (s[l - 1] == '\n' || s[l - 1] == '\r')) s[--l] = '\0';
}

static char *dup_str(const char *s) {
	char *d = malloc(strlen(s) + 1);
	if (!d) { perror("malloc"); exit(EXIT_FAILURE); }
	strcpy(d, s);
	return d;
}

/* conta linhas de dados (ignora header e linhas vazias) */
static int conta_linhas(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) { fprintf(stderr, "erro ao abrir %s\n", path); exit(EXIT_FAILURE); }
	char buf[1024];
	int n = 0, primeira = 1;
	while (fgets(buf, sizeof(buf), f)) {
		chomp(buf);
		if (buf[0] == '\0') continue;
		if (primeira) { primeira = 0; continue; }
		n++;
	}
	fclose(f);
	return n;
}

static void add_edge(Grafo *g, int u, int v, double w) {
	Edge *e = malloc(sizeof(*e));
	if (!e) { perror("malloc"); exit(EXIT_FAILURE); }
	e->to = v; e->w = w; e->next = g->adj[u]; g->adj[u] = e;
}

Grafo *grafo_load(const char *nos_csv, const char *arestas_csv) {
	Grafo *g = calloc(1, sizeof(*g));
	if (!g) { perror("calloc"); exit(EXIT_FAILURE); }

	int cap = conta_linhas(nos_csv);
	g->cod  = malloc((size_t)cap * sizeof(char *));
	g->nome = malloc((size_t)cap * sizeof(char *));
	g->uf   = malloc((size_t)cap * sizeof(char *));
	g->lat  = malloc((size_t)cap * sizeof(double));
	g->lon  = malloc((size_t)cap * sizeof(double));
	g->adj  = calloc((size_t)cap, sizeof(Edge *));
	if (!g->cod || !g->nome || !g->uf || !g->lat || !g->lon || !g->adj) {
		perror("malloc grafo"); exit(EXIT_FAILURE);
	}

	/* ----- nos ----- */
	FILE *f = fopen(nos_csv, "r");
	if (!f) { fprintf(stderr, "erro ao abrir %s\n", nos_csv); exit(EXIT_FAILURE); }
	char buf[1024];
	int primeira = 1;
	while (fgets(buf, sizeof(buf), f)) {
		chomp(buf);
		if (buf[0] == '\0') continue;
		if (primeira) { primeira = 0; continue; }
		char *c[8];
		int nf = csv_split(buf, c, 8);
		if (nf < 5) continue;
		int i = g->n;
		g->cod[i]  = dup_str(c[0]);
		g->nome[i] = dup_str(c[1]);
		g->uf[i]   = dup_str(c[2]);
		g->lat[i]  = atof(c[3]);
		g->lon[i]  = atof(c[4]);
		g->n++;
	}
	fclose(f);

	/* ----- arestas ----- */
	f = fopen(arestas_csv, "r");
	if (!f) { fprintf(stderr, "erro ao abrir %s\n", arestas_csv); exit(EXIT_FAILURE); }
	primeira = 1;
	int ignoradas = 0;
	while (fgets(buf, sizeof(buf), f)) {
		chomp(buf);
		if (buf[0] == '\0') continue;
		if (primeira) { primeira = 0; continue; }
		char *c[4];
		int nf = csv_split(buf, c, 4);
		if (nf < 2) continue;
		int u = grafo_idx_por_codigo(g, c[0]);
		int v = grafo_idx_por_codigo(g, c[1]);
		if (u < 0 || v < 0) { ignoradas++; continue; }
		double w = haversine(g, u, v);
		add_edge(g, u, v, w);
		add_edge(g, v, u, w);
	}
	fclose(f);
	if (ignoradas)
		fprintf(stderr, "aviso: %d aresta(s) ignorada(s) (codigo sem no)\n", ignoradas);

	return g;
}

void grafo_free(Grafo *g) {
	if (!g) return;
	for (int i = 0; i < g->n; i++) {
		free(g->cod[i]); free(g->nome[i]); free(g->uf[i]);
		Edge *e = g->adj[i];
		while (e) { Edge *p = e->next; free(e); e = p; }
	}
	free(g->cod); free(g->nome); free(g->uf);
	free(g->lat); free(g->lon); free(g->adj);
	free(g);
}

double haversine(const Grafo *g, int a, int b) {
	const double R = 6371.0;
	double la1 = g->lat[a] * PI / 180.0, la2 = g->lat[b] * PI / 180.0;
	double dla = (g->lat[b] - g->lat[a]) * PI / 180.0;
	double dlo = (g->lon[b] - g->lon[a]) * PI / 180.0;
	double s = sin(dla / 2) * sin(dla / 2) + cos(la1) * cos(la2) * sin(dlo / 2) * sin(dlo / 2);
	return R * 2.0 * atan2(sqrt(s), sqrt(1.0 - s));
}

int grafo_idx_por_codigo(const Grafo *g, const char *cod) {
	for (int i = 0; i < g->n; i++)
		if (strcmp(g->cod[i], cod) == 0) return i;
	return -1;
}

int grafo_idx_por_nome_uf(const Grafo *g, const char *nome, const char *uf) {
	for (int i = 0; i < g->n; i++)
		if (strcmp(g->nome[i], nome) == 0 && strcmp(g->uf[i], uf) == 0) return i;
	return -1;
}

static int so_digitos(const char *s) {
	if (!*s) return 0;
	for (const char *p = s; *p; p++) if (*p < '0' || *p > '9') return 0;
	return 1;
}

int grafo_resolver(const Grafo *g, const char *token) {
	if (so_digitos(token))
		return grafo_idx_por_codigo(g, token);

	/* "Nome/UF" */
	const char *barra = strrchr(token, '/');
	if (barra) {
		char nome[256], uf[8];
		size_t ln = (size_t)(barra - token);
		if (ln >= sizeof(nome)) ln = sizeof(nome) - 1;
		memcpy(nome, token, ln); nome[ln] = '\0';
		snprintf(uf, sizeof(uf), "%s", barra + 1);
		return grafo_idx_por_nome_uf(g, nome, uf);
	}

	/* "Nome" sozinho: detecta homonimos entre UFs diferentes */
	int achado = -1, qtd = 0;
	for (int i = 0; i < g->n; i++) {
		if (strcmp(g->nome[i], token) == 0) {
			if (qtd == 0) achado = i;
			qtd++;
		}
	}
	if (qtd == 0) return RESOLVE_NAO_ENCONTRADO;
	if (qtd == 1) return achado;

	fprintf(stderr, "Nome ambiguo: \"%s\" existe em %d municipios. Qualifique com /UF:\n", token, qtd);
	for (int i = 0; i < g->n; i++)
		if (strcmp(g->nome[i], token) == 0)
			fprintf(stderr, "   %s/%s (%s)\n", g->nome[i], g->uf[i], g->cod[i]);
	return RESOLVE_AMBIGUO;
}
