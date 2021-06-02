#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <chrono>

using namespace std::chrono;

void Init(int*& numbers, int*& squares, int*& squaresSerial, int count, int maxNum) {
	int r;
	int p;

	for (int i = 0; i < count; i++) {
		r = rand() % maxNum;
		p = rand() % 2;

		if (p) {
			r *= r;
		}
		numbers[i] = r;
		squares[i] = -1;
		squaresSerial[i] = -1;
	}
}

void PrintResults(int* squares, int count) {
	for (int i = 0; i < count; i++) {
		if (squares[i] != -1) {
			printf("%d ", squares[i]);
		}
	}
}

int main()
{
	srand(123);
	int procNum = 32;
	int count = 1000;
	int maxNum = 100;
	int* numbers = new int[count];
	int* squares = new int[count];
	int* squaresSerial = new int[count];
	int part = count / procNum;

	Init(numbers, squares, squaresSerial, count, maxNum);

	auto start = steady_clock::now();
	for (int i = 0; i < count; i++) {
		int s = sqrt(numbers[i]);

		if (s * s == numbers[i]) {
			squaresSerial[i] = numbers[i];
		}
	}
	auto end = steady_clock::now();

	printf("Serial algorithm results\n");
	PrintResults(squaresSerial, count);
	printf("%\nTime elapsed: %7.4f", (double)duration_cast<microseconds>(end - start).count() / 1000000.0);
	printf("\n-------\n");

	double startPar = omp_get_wtime();
	#pragma omp parallel for shared(numbers)
	for (int i = 0; i < procNum; i++) {
		int maxInd = (i + 1) * part;

		for (int j = i * part; j < maxInd; j++) {
			int s = sqrt(numbers[j]);

			if (s * s == numbers[j]) {
				squares[j] = numbers[j];
			}
		}
	}
	double endPar = omp_get_wtime();

	printf("OpenMP parallel algorithm results\n");
	PrintResults(squares, count);
	printf("%\nTime elapsed: %7.4f", endPar - startPar);

    return 0;
}