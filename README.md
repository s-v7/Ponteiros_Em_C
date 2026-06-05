# Pointers for Memory and Vector in C/C++

## Overview

This project started as a demonstration of pointers for memory manipulation, dynamic allocation, and arrays in C++. It has since grown into a small showcase of classic computer-science foundations implemented from scratch in C/C++ — graph search (Dijkstra, A\*), the algebraic path problem over semirings, cryptographic hashing and a minimal blockchain, probabilistic systems, and a real-world route optimizer — all resting on hands-on pointer and memory work.

It includes examples of:

- **Pointer arithmetic**
- **Dynamic memory allocation** (`new` and `delete`)
- **Array manipulation using pointers**

The code is modularized as a learning resource for developers exploring low-level C/C++ and how classical algorithms map to real problems.

## Features

### Static Array Manipulation
- **Initialize an array with user inputs using pointers.**
- **Display array values and their memory addresses.**

### Dynamic Memory Allocation
- **Demonstrates the use of `new` and `delete` to allocate and deallocate memory dynamically.**

### Sum of Array Elements
- **Calculate the sum of elements in an array using pointer arithmetic.**

### Modular Code
- **Functions like `initializeArray`, `displayArray`, and `dynamicMemoryExample` separate the logic for easier understanding.**

---

## Pathfinding

### Dijkstra — Shortest Path, O(E log V) (`dijkstra.c`)

Single-source shortest path using an **adjacency list (linked edges)** plus a **binary min-heap** with `decrease-key`, bringing the cost from the naive O(V²) down to **O(E log V)**. The heap is an array with index arithmetic for parent/children (`pai=(i-1)/2`, `esq=2i+1`, `dir=2i+2`), and `pos[v]` maps each vertex to its slot so keys can be decreased in O(log V).

Classic applications: logistics and route optimization, navigation, network routing, urban-planning simulations, game pathfinding.

```c
char *nomes[] = { "Deposito", "Filial Norte", "Filial Sul", "Hub Regional", "Cliente" };
```

```bash
gcc -O2 -Wall -Wextra dijkstra.c -o dijkstra_teste
./dijkstra_teste Deposito Cliente
# Distancia minima de Deposito para Cliente: 5
# Caminho: Deposito -> Filial Norte -> Hub Regional -> Cliente
```

### A\* with Haversine Heuristic (`astar.c`)

A\* is Dijkstra with a sense of direction. By giving each vertex geographic coordinates (lat/lon), the **inner product of ℝ² induces a norm, the norm a metric, and the metric an admissible heuristic** `h(v)` = great-circle (haversine) distance to the goal. Because the straight-line distance never overestimates the real path (triangle inequality), A\* is admissible/consistent and returns the **same optimal path as Dijkstra while expanding far fewer nodes**. It reuses the very same binary heap, keyed on `f = g + h`.

```bash
gcc -O2 -Wall -Wextra astar.c -o astar -lm
./astar Deposito Cliente
```

### A\* over the real municipalities of Piauí (`astar_municipios_pi.c`)

The toy graph becomes a real route-planning engine. Each of the 224 municipalities of Piauí is a node (coordinates via PostGIS `ST_PointOnSurface`), and edges are shared-border adjacencies. Nodes and edges are exported to CSV by the queries in `queries/`, and the program loads them, computes edge weights with the **same haversine function used by the heuristic** (single source of truth), and routes by `codigo_ibge` or municipality name.

The value of the heuristic shows clearly in node expansion (real runs):

| Route                          | A\* expanded | Dijkstra expanded |
|--------------------------------|:------------:|:-----------------:|
| Teresina → Bom Jesus           |      45      |        209        |
| Teresina → Simplício Mendes    |      34      |        138        |
| Picos → Simplício Mendes       |       7      |         49        |

```bash
# export from PostGIS (see queries/)
psql -h localhost -U postgres -d postgres -f queries/nos_municipios_pi.sql
psql -h localhost -U postgres -d postgres -f queries/arestas_municipios_pi.sql

gcc -O2 -Wall -Wextra astar_municipios_pi.c -o astar_municipios -lm
./astar_municipios "Teresina" "Bom Jesus"
```

> Note: weights are accumulated haversine distance between municipality seats (geographic hops), not road kilometers. Swapping in an OSM/DNIT road network turns the weight into real driving distance without changing the algorithm.

### Fiscal Route Optimizer (`fiscal_otimizador.c`)

Takes real CREA-PI fiscal routes (`fiscal_roteiros` ⋈ `fiscal_roteiro_municipio`) exported to CSV and reorders each route's stops to minimize total travel, via a TSP heuristic: **nearest-neighbor + 2-opt** over haversine. It compares the current order against the optimized one, reports the percentage gained, and emits the `UPDATE`s to fill the `ordem_otimizada` column. Stops with missing coordinates are warned and skipped.

```bash
gcc -O2 -Wall -Wextra fiscal_otimizador.c -o fiscal_otimizador -lm
./fiscal_otimizador ALL          # processes every route, writes ordem_otimizada.sql
./fiscal_otimizador PIC-01       # a single route by codigo
```

---

## Graphs and the Algebraic Path Problem

### Transitive Closure — Warshall (`warshall.c`)

Boolean reachability via the classic triple loop `M[i][j] = M[i][j] || (M[i][k] && M[k][j])`. This is the (∨, ∧) Boolean semiring: it answers "can i reach j?" for a directed graph.

### One Algorithm, Many Algorithms — Semirings (`semiring_path.c`)

The Warshall loop generalizes to the **algebraic path problem**: the same triple loop
`M[i][j] = M[i][j] ⊕ (M[i][k] ⊗ M[k][j])` solves an entire family of problems by swapping the semiring `(⊕, ⊗, 0̄, 1̄)`:

| Semiring (⊕, ⊗)    | 0̄ / 1̄        | Computes                         |
|--------------------|---------------|----------------------------------|
| Boolean (OR, AND)  | 0 / 1         | Reachability (Warshall)          |
| Tropical (min, +)  | +∞ / 0        | Shortest path (Floyd)            |
| Widest (max, min)  | −∞ / +∞       | Bottleneck / widest path         |
| Counting (+, ×)    | 0 / 1         | Number of paths (DAG)            |

The Boolean case reproduces `warshall.c` exactly; the tropical case is validated against an independent Floyd-Warshall on hundreds of random graphs.

```bash
gcc -O2 -Wall -Wextra warshall.c -o wharshall && ./wharshall
gcc -O2 -Wall -Wextra semiring_path.c -o semiring_path && ./semiring_path
```

---

## Probabilistic Systems — Markov Chains (`markov.c`)

A discrete-time Markov chain. Given a row-stochastic transition matrix `P` (each row sums to 1) and an initial state distribution, one step advances the distribution by a vector-by-matrix product:

```
next[j] = Σ_i  state[i] · P[i][j]
```

Iterating this forward (here, 3 states over 10 steps) shows the distribution converging toward the chain's stationary distribution. A single step is exactly matrix multiplication in the probability semiring (+, ×) — the same algebra generalized in `semiring_path.c`.

```bash
gcc -O2 -Wall -Wextra markov.c -o markov
./markov
# Passo 1:  S0: 0.100 S1: 0.600 S2: 0.300
# Passo 2:  S0: 0.310 S1: 0.390 S2: 0.300
# ...
# Passo 10: S0: 0.258 S1: 0.419 S2: 0.323   (≈ distribuição estacionária)
```

---

## Blockchain & Hashing (SHA-256)

This section showcases the most elegant idea in the whole repository: **a blockchain is just a linked list whose link is two things at once.**

```c
struct Block {
    struct Block *prev;            /* an ordinary C pointer  -> walk the chain        */
    char          prev_hash[65];   /* a cryptographic pointer -> make it tamper-evident */
    /* ... */
};
```

Change a single byte in any past block and every subsequent hash stops matching, so the chain refuses to validate. That single property is what separates a blockchain from a plain linked list — and it is built entirely on pointers.

### SHA-256 from scratch (`sha256.c` / `sha256.h`)

A clean-room implementation of SHA-256 following **FIPS PUB 180-4**, with **no external libraries** — only pointer and bitwise manipulation. It exposes a streaming API (`sha256_init` / `sha256_update` / `sha256_final`) plus one-shot helpers (`sha256_bytes`, `sha256_hex`), verified against the official test vectors:

```plaintext
SHA-256("")    = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
SHA-256("abc") = ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
```

### Minimal blockchain (`blockchain.c`)

Each block carries an index, timestamp, payload, nonce, the previous block's hash, and its own hash. New blocks are mined with a simple **proof-of-work** (a hash with N leading zero hex digits), and the chain can be validated end-to-end and shown to reject any tampering.

```plaintext
Validating original chain... VALID
Tampering with block #2 (rewriting its data)...
  ! block #2: stored hash does not match its contents
Re-validating tampered chain... INVALID
```

---

## Requirements

- **C compiler** (`gcc`) and **C++ compiler** (`g++`).
- Math library (`-lm`) for the A\* / optimizer programs.
- *Optional* (geographic parts): PostgreSQL + PostGIS and `psql`, to export `nos.csv` / `arestas.csv` / `roteiros.csv`.

## How to Compile and Run

```bash
git clone https://github.com/s-v7/Ponteiros_Em_C.git
cd Ponteiros_Em_C

# pointers / arrays
g++ -O2 -Wall -Wextra Pointers_C++.cpp -o pointers

# pathfinding
gcc -O2 -Wall -Wextra dijkstra.c            -o dijkstra_teste
gcc -O2 -Wall -Wextra astar.c               -o astar              -lm
gcc -O2 -Wall -Wextra astar_municipios_pi.c -o astar_municipios   -lm
gcc -O2 -Wall -Wextra fiscal_otimizador.c   -o fiscal_otimizador  -lm

# graphs / semirings
gcc -O2 -Wall -Wextra warshall.c            -o wharshall
gcc -O2 -Wall -Wextra semiring_path.c       -o semiring_path

# probabilistic
gcc -O2 -Wall -Wextra markov.c              -o markov

# crypto / blockchain
gcc -O2 -Wall -Wextra sha256.c blockchain.c -o blockchain_demo
```

A helper script, `verify_test.sh`, compiles every source (treating warnings as failures) and runs smoke tests before committing.

## Sample Output

```plaintext
Address: 0x7ffee3a8b8d0 -> Value: 5      # pointers: address + value
Sum of array elements: 150

Distancia minima de Deposito para Cliente: 5
Caminho: Deposito -> Filial Norte -> Hub Regional -> Cliente
```

---

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, feel free to open an issue or submit a pull request.

1. **Fork this repository.**
2. **Create a branch:** `git checkout -b feature-name`
3. **Commit your changes:** `git commit -m "Add feature description"`
4. **Push:** `git push origin feature-name`
5. **Open a pull request.**

## License

This project is licensed under the **MIT License**.

## Contact

- **Author:** Silas Vasconcelos Cruz
- **GitHub:** [s-v7](https://github.com/s-v7)

## Tags

`pointers` `low-level` `memory-management` `arrays` `C` `C++` `algorithms` `graphs` `dijkstra` `astar` `pathfinding` `heuristics` `floyd-warshall` `semiring` `algebraic-path-problem` `markov` `blockchain` `sha-256` `hashing` `cryptography` `postgis` `geospatial` `route-optimization` `educational`
