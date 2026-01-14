# Pointers for Memory and Vector in C++

## Overview

This project demonstrates how to use pointers for memory manipulation, dynamic memory allocation, and working with arrays in C++. It includes examples of:

- **Pointer arithmetic**
- **Dynamic memory allocation** (`new` and `delete`)
- **Array manipulation using pointers**

The program is modularized for better understanding and serves as a learning resource for developers exploring C++ pointers and memory management.

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
## Graph Algorithms – Shortest Path (Dijkstra)

This project now also includes an implementation of the **Dijkstra algorithm** using a matrix of adjacency in C.  
The goal is to demonstrate how classical graph algorithms are applied to real-world problems such as:

- Logistics and route optimization  
- Maps and navigation systems  
- Games pathfinding  
- Computer networks routing  
- Urban planning simulations  
- Basic AI navigation engines  

Each vertex represents a real entity (for example, locations, cities, routers, or game positions), and each edge weight represents a cost such as:

- Distance  
- Time  
- Fuel consumption  
- Monetary cost  
- Network latency  

### Example of Real-World Mapping

```c
char *nomes[] = {
    "Deposito",
    "Filial Norte",
    "Filial Sul",
    "Hub Regional",
    "Cliente"
};
```
---

## Requirements

- **C++ Compiler** (e.g., `g++` for Linux/Ubuntu or MinGW for Windows).
- **Basic knowledge of C++ programming, particularly pointers and arrays.**

## How to Compile and Run

### Clone the Repository
```bash
git clone https://github.com/s-v7/Ponteiros_Em_C.git
cd Ponteiros_Em_C

```

### Compile the Program
```bash
g++ pointers.cpp -o output

gcc dijkstra.c -O2 -Wall -o dijkstra_teste

```
---
### Run the Executable
```bash
./output

./dijkstra_teste Deposito Cliente

```

## Sample Output

### Initializing Array with User Input
```plaintext
Enter a value for position [0]: 5
Enter a value for position [1]: 10
Enter a value for position [2]: 15
```
---
### Displaying Array Addresses and Values
```plaintext
Address: 0x7ffee3a8b8d0 -> Value: 5
Address: 0x7ffee3a8b8d4 -> Value: 10
Address: 0x7ffee3a8b8d8 -> Value: 15
```
---
### Calculating Sum of Array Elements
```plaintext
Sum of array elements: 150
```
---
### Demonstrating Dynamic Memory Allocation
```plaintext
Enter the size of the dynamic array: 5
Dynamic Array [0]: 0
Dynamic Array [1]: 2
Dynamic Array [2]: 4
Dynamic Array [3]: 6
Dynamic Array [4]: 8
Dynamic memory deallocated.
```
---
### Which produces an output like:
```bash
Lowest Deposit Cost for Customer: 5
Path: Warehouse -> Northern Branch -> Regional Hub -> Customer
```
### Output:
```bash
Minimum distance from Warehouse to Customer: 5 km
Route: Warehouse -> Northern Branch -> Regional Hub -> Customer
```
---
## Contributing
Contributions are welcome! If you find any issues or have suggestions for improvements, feel free to open an issue or submit a pull request.

### Steps to Contribute

1. **Fork this repository.**
2. **Create a new branch for your changes:**
   ```bash
   git checkout -b feature-name
   ```
3. **Commit your changes:**
   ```bash
   git commit -m "Add feature description"
   ```
4. **Push to your branch:**
   ```bash
   git push origin feature-name
   ```
5. **Open a pull request.**

## License

This project is licensed under the **MIT License**.

## Contact

For questions or suggestions, feel free to reach out:

- **Author:** Silas Vasconcelos Cruz
- **GitHub:** [s-v7](https://github.com/s-v7)

## Tags

`pointers` Low-level memory manipulation (pointers)` `arrays` `C++` `memory-management` `educational`

