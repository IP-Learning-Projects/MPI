#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <ctype.h>
#include <string.h>

#define NR ('z' - 'a' + 1)

//This accepts only lowercase key and input
//97-122 a-z

char wrapAroundRight(char letter, int positions) {
	if (letter + positions > 'z') {
		return letter - NR + positions;
	}
	return letter + positions;
}
char wrapAroundLeft(char letter, int positions) {
	if (letter - positions < 'a') {
		return letter + NR - positions;
	}
	return letter - positions;
}

char vigenere_encrypt(char letter, char key)
{
	if (!isalpha(letter))
		return letter;

	int x;

	x = wrapAroundRight(letter, (key - 'a'));
	return x;
}

char vigenere_decrypt(char letter, char key)
{
	if (!isalpha(letter))
		return letter;

	int x;
	x = wrapAroundLeft(letter, (key - 'a'));

	return x;
}

char vigenere(char letter, char key, char mode) {
	if (mode == 'e') {
		return vigenere_encrypt(letter, key);
	}
	else {
		return vigenere_decrypt(letter, key);
	}
}


int main(int argc, char *argv[])
{
	char *initialString, *key, *finalString, mode;
	int rank, size, len, keylen;

	initialString = (char*)malloc(256);
	key = (char*)malloc(256);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;


	if (rank == 0) {

		printf("Choose mode: (E)ncrypt or (D)ecrypt \n");
		fflush(stdout);
		scanf_s("%c", &mode, 1);
		fflush(stdin);

		printf("Input text to encrypt/decrypt\n");
		fflush(stdout);
		scanf_s("%s", initialString, 200);
		fflush(stdin);

		printf("Input key\n");
		fflush(stdout);
		scanf_s("%s", key, 200);
		fflush(stdin);

		len = strlen(initialString);
		keylen = strlen(key);

	}

	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&keylen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&mode, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

	finalString = (char*)malloc(len * sizeof(char));
	finalString[len] = '\0';

	if (rank == 0) {
		for (int i = 0; i < len; i++) {
			char temp[2] = { initialString[i], key[i % keylen] };
			MPI_Send(temp, 2, MPI_CHAR, i % (size - 1) + 1, 0, MPI_COMM_WORLD);
		}
		for (int i = 0; i < len; i++) {
			MPI_Recv(&finalString[i], 1, MPI_CHAR, i % (size - 1) + 1, 0, MPI_COMM_WORLD, &status);
		}
		printf("Result: %s\n", finalString);
		fflush(stdout);
	}
	else {

		if (rank >= len % (size - 1) + 1) {
			for (int i = 0; i < len / (size - 1); i++) {
				char temp[2];
				MPI_Recv(&temp, 2, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
				char character = vigenere(temp[0], temp[1], mode);
				MPI_Send(&character, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
		}
		else {
			for (int i = 0; i < len / (size - 1) + 1; i++) {
				char temp[2];
				MPI_Recv(&temp, 2, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
				char character = vigenere(temp[0], temp[1], mode);
				MPI_Send(&character, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
		}
	}
	MPI_Finalize();
}

