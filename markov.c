/*
 * markov.c - Discrete-time Markov chain.
 * Iterates a state distribution under the transition matrix P for 10 steps,
 * converging to the stationary distribution. See markov.h for the interface.
 *
 * Author: s-v7 | License: MIT
 * Compile: gcc -O2 -Wall -Wextra markov.c -o markov
 */
#include <stdio.h>
#include "markov.h"


void next_state(int n, double P[MAXN][MAXN], double state[], double next[]) {
	for(int j = 0; j < n; j++){
		next[j] = 0.0;
		for(int i = 0; i < n; i++){
			next[j] += state[i] * P[i][j];
		}
	}
}

void print_state(int n, double state[]){
	for(int i = 0; i < n; i++)
		printf("S%d: %.3f ", i, state[i]);
	printf("\n");
}

int main() {
	int n = 3;
        // Transition matrix (each row sums to 1)
	double P[MAXN][MAXN] = {
		{0.1, 0.6, 0.3},
		{0.4, 0.4, 0.2},
		{0.2, 0.3, 0.5},
	};

	//Initial state (sums to 1)
	double state[MAXN] = {1.0, 0.0, 0.0}; // Starts entirely in state s0,
	double next[MAXN];

	printf("State init:\n");
	for(int step = 1; step <= 10; step++) {
		next_state(n, P, state, next);
		for(int i = 0; i < n; i++)
			state[i] = next[i];
		printf("Passo %d:\n", step);
		print_state(n, state);
	}
	return 0;
}


