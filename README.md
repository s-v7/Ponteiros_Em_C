# Pointers for Memory and Vector

## Overview

This C++ script demonstrates the use of pointers for memory management and vectors. It initializes two integer arrays (`u` and `v`) and uses pointers to interact with the memory locations of these arrays.

## Author

- **Author:** Silas Vasconcelos Cruz -> {s-v7}
- **Language:** C++
- **Date:** [Insert Date]

## Description

The script includes the following key components:

### Integer Arrays

```cpp
int u[10];
int v[10];
```

## Integer Pointers
```cpp
int *pu, *pv;
```
- **Two integer pointers, pu and pv, are declared to point to the memory locations of u and v arrays, respectively**

## Input and Initialization
```cpp
for(i = 0; i < 10; i++){
    *(pu+i) = 0;
    cout << "Enter Values: [" << i << "]" << '\n';
    cin >> i;
    *(pu+i) = i;
}
```
- **A loop is used to initialize and input values into the u array using pointers.**

## Displaying Addresses and Values
```cpp
for(int i : u){
    cout << pu+i << "->" << *(pu+i) << '\n' << '\n';
}
```
- **Another loop is used to display the memory addresses and values stored in the u array.**

## Usage
- **Compile and run the script to observe the interaction with memory using pointers.**
```cpp
 g++ your_script.cpp -o your_executable
./your_executable
```
## Feel free to modify the script and experiment with different values.

