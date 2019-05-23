#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

#define NR ('z' - 'a' + 1)

//65-90 A-Z
//97-122 a-z

int string_length(char s[]) {
	int c = 0;
	while (s[c] != '\0') {
		c++;
	}
	return c;
}

bool isLowerCase(char letter) {
	return (letter >= 'a' && letter <= 'z');
}

bool isUpperCase(char letter) {
	return (letter >= 'A' && letter <= 'Z');
}

bool isAlpha(char letter) {
	return isLowerCase(letter) || isUpperCase(letter);
}

char toLowerCase(char letter) {
	if (isLowerCase(letter)) {
		return letter;
	}
	return (letter - 'A') % NR + 'a';
}

char toUpperCase(char letter) {
	if (isUpperCase(letter)) {
		return letter;
	}
	return (letter - 'a') % NR + 'A';
}

char wrapAroundRight(char letter, int positions) {
	if (isLowerCase(letter)) {
		if (letter + positions > 'z') {
			return letter - NR + positions;
		}
	}
	else {
		if (letter + positions > 'Z') {
			return letter - NR + positions;
		}
	}
	return letter + positions;
}
char wrapAroundLeft(char letter, int positions) {
	if (isLowerCase(letter)) {
		if (letter - positions < 'a') {
			return letter + NR - positions;
		}
	}
	else {
		if (letter - positions < 'A') {
			return letter + NR - positions;
		}
	}
	return letter - positions;
}

char vigenere_encrypt(char letter, char key)
{
	if (!isAlpha(letter))
		return letter;

	int x;

	if (isLowerCase(letter)) {
		if (isUpperCase(key)) {
			key = toLowerCase(key);
		}
		x = wrapAroundRight(letter, (key - 'a'));
	}
	else {
		if (isLowerCase(key)) {
			key = toUpperCase(key);
		}
		x = wrapAroundRight(letter, (key - 'A'));
	}
	return x;
}

char vigenere_decrypt(char letter, char key)
{

	if (!isAlpha(letter))
		return letter;

	int x;

	if (isLowerCase(letter)) {
		if (isUpperCase(key)) {
			key = toLowerCase(key);
		}
		x = wrapAroundLeft(letter,(key - 'a'));
	}
	else {
		if (isLowerCase(key)) {
			key = toUpperCase(key);
		}
		x = wrapAroundLeft(letter, (key - 'A'));
	}

	return x;
}

char vigenere(char letter, char key, char mode) {
	if (mode == 'e' || mode == 'E') {
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
		scanf_s("%c", &mode,1);
		fflush(stdin);

		printf("Input text to encrypt/decrypt\n");
		fflush(stdout);
		scanf_s("%s", initialString, 200);
		fflush(stdin);

		printf("Input key\n");
		fflush(stdout);
		scanf_s("%s", key, 200);
		fflush(stdin);

		len = string_length(initialString);
		keylen = string_length(key);

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

