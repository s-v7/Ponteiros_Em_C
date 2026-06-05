/* =====================================================================
 * semiring_path.c - The "algebraic path problem": ONE triple loop, many
 * algorithms, changing only the semiring (add, mul, zero, one).
 *
 * Generalizes warshall.c (boolean transitive closure) into the form
 *     M[i][j] = M[i][j] (+) ( M[i][k] (x) M[k][j] )
 * where (+) and (x) are the operations of a semiring:
 *
 *   semiring            (+)   (x)   zero   one   solves
 *   ------------------  ----  ----  -----  ----  -----------------------
 *   boolean             OR    AND   0      1     reachability (Warshall)
 *   tropical            min   +     +INF   0     shortest path (Floyd)
 *   bottleneck (widest) max   min   -INF   +INF  widest path
 *   counting            +     *     0      1     number of paths (DAG)
 *
 * Author: s-v7
 * License: MIT
 * Compile: gcc -O2 -Wall -Wextra semiring_path.c -o semiring_path
 * ===================================================================== */


#include <stdio.h>
#include <math.h>

#define MAXN 16

typedef double Elem;

typedef struct {
    const  char *nome;
    Elem (*add)(Elem, Elem); /* (+) : escolhe/combina caminhos */
    Elem (*mul)(Elem, Elem); /* (*) : compoe um caminho		*/
    Elem zero;		    /*  identidade de (+), anaquila (*) */
    Elem one;		   /* identidade de (*)			*/
} Semiring;

/* ---- operacoes dos quatro semianeis ---- */
static Elem b_add(Elem a, Elem b) { return (a != 0.0 ||  b != 0.0) ? 1.0 : 0.0; } /* OR */
static Elem b_mul(Elem a, Elem b) { return (a != 0.0 &&  b != 0.0) ? 1.0 : 0.0; } /* AND */

static Elem t_add(Elem a, Elem b) { return a < b ? a : b; }			 /* min */
static Elem t_mul(Elem a, Elem b) { return a + b; }				 /* + */

static Elem w_add(Elem a, Elem b) { return a > b ? a : b; }			 /* max */
static Elem w_mul(Elem a, Elem b) { return a < b ? a : b; } 			 /* min */

static Elem c_add(Elem a, Elem b) { return a + b; } 				 /* + */
static Elem c_mul(Elem a, Elem b) { return a * b; } 				 /* * */

static const Semiring BOOLEANO = {"booleano (OR,AND)", b_add, b_mul, 0.0,	1.0		};
static const Semiring TROPICAL = {"tropical (min,+)", t_add, t_mul, INFINITY,	0.0		};
static const Semiring GARGALO  = {"gargalo (max,,in)", w_add, w_mul, -INFINITY, INFINITY	};
static const Semiring CONTAGEM = {"contagem (+,*)",   c_add, c_mul, 0.0,	1.0		};

/* =========================================================================
 * O coracao: identico ao warshall.c, so que (+) e (x) vem do semianel.
 * ========================================================================= */
static void closure(int n, Elem M[][MAXN], const Semiring *smrg){
   for(int k = 0; k < n; k++)
      for(int i = 0; i < n; i++)
         for(int j = 0; j < n; j++)
		 M[i][j] = smrg->add(M[i][j], smrg->mul(M[i][k], M[k][j]));
}

/* impressao sensivel ao semianel (INF, inteiros, etc.) */
static void imprime(int n, Elem M[][MAXN], const Semiring *s) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			Elem v = M[i][j];
			if (v == INFINITY)       printf("%6s ", "INF");
			else if (v == -INFINITY) printf("%6s ", "-INF");
			else if (s == &BOOLEANO) printf("%6d ", (int)v);
			else if (s == &CONTAGEM) printf("%6.0f ", v);
			else                     printf("%6.1f ", v);
		}
		printf("\n");
	}
}

static void roda(const char *titulo, int n, Elem M[][MAXN], const Semiring *s) {
	printf("\n=== %s | semianel: %s ===\n", titulo, s->nome);
	printf("inicial:\n"); imprime(n, M, s);
	closure(n, M, s);
	printf("resultado:\n"); imprime(n, M, s);
}

int main(void) {
	int n = 5;

	/* (1) Mesma matriz do warshall.c -> deve reproduzir o fechamento transitivo */
	Elem B[MAXN][MAXN] = {
		{0,1,0,0,0},
		{0,0,1,0,0},
		{1,0,0,1,0},
		{0,0,0,0,0},
		{1,0,1,0,0}
	};
	roda("Alcancabilidade (grafo do warshall.c)", n, B, &BOOLEANO);

	/* (2) e (3): grafo dirigido com pesos.  zero = sem aresta; diagonal = one.
	 * Arestas: 0->1(7) 1->2(2) 2->0(3) 2->3(4) 4->0(1) 4->2(5) */
	Elem W[MAXN][MAXN];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			W[i][j] = (i == j) ? 0.0 : INFINITY;   /* one tropical = 0 na diagonal */
	W[0][1] = 7; W[1][2] = 2; W[2][0] = 3; W[2][3] = 4; W[4][0] = 1; W[4][2] = 5;
	roda("Caminho minimo (all-pairs)", n, W, &TROPICAL);

	/* gargalo: mesma topologia como "capacidades"; diagonal = one = +INF */
	Elem C[MAXN][MAXN];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			C[i][j] = (i == j) ? INFINITY : -INFINITY; /* one gargalo = +INF */
	C[0][1] = 7; C[1][2] = 2; C[2][0] = 3; C[2][3] = 4; C[4][0] = 1; C[4][2] = 5;
	roda("Caminho mais largo / gargalo (all-pairs)", n, C, &GARGALO);

	/* (4) contagem de caminhos: precisa de DAG (ciclo -> infinitos passeios).
	 * DAG topologicamente ordenado: 0->1 0->2 1->2 1->3 2->3 */
	int m = 4;
	Elem D[MAXN][MAXN] = {0};
	D[0][1] = 1; D[0][2] = 1; D[1][2] = 1; D[1][3] = 1; D[2][3] = 1;
	roda("Numero de caminhos (DAG)", m, D, &CONTAGEM);
	printf("(esperado D[0][3] = 3: 0-1-3, 0-2-3, 0-1-2-3)\n");

	return 0;
}


