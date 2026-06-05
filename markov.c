
#include <stdio.h>

#define MAXN 10

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
	// Matriz de transition (cada line soma 1)
	double P[MAXN][MAXN] = {
		{0.1, 0.6, 0.3},
		{0.4, 0.4, 0.2},
		{0.2, 0.3, 0.5},
	};

	//Estado init (também soma 1)
	double state[MAXN] = {1.0, 0.0, 0.0}; // Começa totalmente no estado s0
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


