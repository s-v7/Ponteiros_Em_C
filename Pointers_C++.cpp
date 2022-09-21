#include<iostream>
#include<string>
#include<cmath>

using namespace std;


/***********************************************************
 *	Author: Silas Vasconcelos Cruz -> {s-v7};
 *	
 *	Language: C++;
 *	
 *	Script: Pointers for memory and Vector;
 *
 * ********************************************************/

int main(){
	/**Vetor de Inteiros **/
	int u[10];
	int v[10];
	int i;
	/**Ponteiros para Inteiros**/
	int *pu, *pv;
	/**Obtendo o Endereço de **/
	pu = &u[0];
	pv = &v[0];
	/**Laço for **/
	for(i = 0; i < 10; i++){ //  OR while(*pu
		*(pu+i) = 0;
		cout << "Enter Values: ["<< i <<"]" << '\n';
		cin >> i;
		*(pu+i) = i;
		/* *(pu) += 1;  Ver outro Valores*/
	}
	for(int i : u){
		/*Escreva os endereços e os valores para o Usuário */
		cout << pu+i << "->" << *(pu+i) << '\n' << '\n';
	}
	return 0;
}

