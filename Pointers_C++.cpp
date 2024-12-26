#include <iostream>
#include <string>
#include <cmath>
#include <ctime>

using namespace std;

/***********************************************************
 * Author: Silas Vasconcelos Cruz -> {s-v7};
 *
 * Language: C++;
 *
 * Script: Pointers for Memory and Vector;
 *
 * Purpose: Demonstrates how to use pointers for memory 
 *          manipulation, arrays, and dynamic memory.
 **********************************************************/

// Function prototypes
void initializeArray(int* arr, int size);
void displayArray(int* arr, int size);
void dynamicMemorySv7();
void calculateArraySum(int* arr, int size);

int main() {
    /*** Variables and Setup ***/
    const int SIZE = 10; // Size of the array
    int u[SIZE];         // Static integer array

    /*** Pointers for Memory Manipulation ***/
    int* pu = &u[0]; // Pointer to the start of array u

    cout << "\n** Initializing Array with User Input **\n";
    initializeArray(pu, SIZE);

    cout << "\n** Displaying Array Addresses and Values **\n";
    displayArray(pu, SIZE);

    cout << "\n** Calculating Sum of Array Elements **\n";
    calculateArraySum(pu, SIZE);

    cout << "\n** Demonstrating Dynamic Memory Allocation **\n";
    dynamicMemorySv7();

    return 0;
}

/**
 * @brief Initializes an array with user-provided values.
 * @param arr Pointer to the array.
 * @param size Size of the array.
 */
void initializeArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        cout << "Enter a value for position [" << i << "]: ";
        int value;
        cin >> value; // Store user input in a temporary variable
        *(arr + i) = value; // Assign value to the array using the pointer
    }
}

/**
 * @brief Displays the addresses and values of an array.
 * @param arr Pointer to the array.
 * @param size Size of the array.
 */
void displayArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        cout << "Address: " << (arr + i) << " -> Value: " << *(arr + i) << '\n';
    }
}

/**
 * @brief Calculates the sum of elements in an array.
 * @param arr Pointer to the array.
 * @param size Size of the array.
 */
void calculateArraySum(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += *(arr + i);
    }
    cout << "Sum of array elements: " << sum << '\n';
}

/**
 * @brief Demonstrates dynamic memory allocation and deallocation.
 */
void dynamicMemorySv7() {
    int size;
    cout << "Enter the size of the dynamic array: ";
    cin >> size;

    int* dynamicArray = new int[size]; // Dynamic memory allocation

    // Initialize and display the dynamic array
    for (int i = 0; i < size; i++) {
        dynamicArray[i] = i * 2; // Example initialization
        cout << "Dynamic Array [" << i << "]: " << dynamicArray[i] << '\n';
    }

    delete[] dynamicArray; // Free the allocated memory
    cout << "Dynamic memory deallocated." << '\n';
}

