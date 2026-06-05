/* Warshall(boolean n x n matrix M)
 * Initially, M = adjacency matrix of a directed graph
 * G with no parallel arcs
 *
 * Author: s-v7
 */

#include <stdio.h>
#include <stdbool.h>

#define N 5

void imprimirMatriz(bool M[N][N]);
void imprimirEstado(bool M[N][N], int tempo, int k);

void imprimirMatriz(bool M[N][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%d ", M[i][j]);
        }
        printf("\n");
    }
}

void imprimirEstado(bool M[N][N], int tempo, int k) {
    printf("\nT%d", tempo);

    if (k >= 0) {
        printf(" - apos considerar k = %d\n", k);
    } else {
        printf(" - matriz inicial\n");
    }

    imprimirMatriz(M);
}

void warshall(bool M[N][N]) {
    int k, i, j;

    imprimirEstado(M, 0, -1);

    for (k = 0; k < N; k++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                M[i][j] = M[i][j] || (M[i][k] && M[k][j]);
            }
        }

        imprimirEstado(M, k + 1, k);
    }
}

int main() {
    bool M[N][N] = {
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 1, 0, 0}
    };

    warshall(M);

    printf("\nFechamento transitivo final:\n");
    imprimirMatriz(M);

    return 0;
}
