/*
 * markov.h - Discrete-time Markov chain (interface).
 *
 * Part of the Ponteiros_Em_C showcase: evolution of a state distribution
 * under a row-stochastic transition matrix (each row sums to 1).
 * One step is the vector-by-matrix product: next[j] = sum_i state[i] * P[i][j],
 * that is, multiplication in the probability semiring (+, *).
 *
 * Author: s-v7
 * License: MIT
 */
#ifndef MARKOV_H
#define MARKOV_H

#define MAXN 10 /* number maximo de states */
/* Advances one step: next = state * P
 * (next[j] = sum_i state[i] * P[i][j])
 * P must be row-stochastic; state and next are vectors of size n. */
void next_state(int n, double P[MAXN][MAXN], double state[], double next[]);

/* Prints the current state distribution (S0,... S{n-1}). */
void print_state(int n, double state[]);

#endif /* MARKOV_H */
