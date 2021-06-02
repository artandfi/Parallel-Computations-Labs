#define _CRT_SECURE_NO_WARNINGS

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <chrono>

using namespace std::chrono;

void Init(int*& numbers, int*& areSquares, int count, int maxNum) {
	int r;
	int p;

	for (int i = 0; i < count; i++) {
		r = rand() % maxNum;
		p = rand() % 2;

		if (p) {
			r *= r;
		}
		numbers[i] = r;
		areSquares[i] = 0;
	}
}

void FindPerfectSquares(int* numbers, int*& areSquares, int count, int& squares) {
	int s;
	for (int i = 0; i < count; i++) {
		s = sqrt(numbers[i]);

		if (s * s == numbers[i]) {
			areSquares[i] = 1;
			squares++;
		}
	}
}

bool AssertEqual(int* arr1, int* arr2, int count) {
	for (int i = 0; i < count; i++) {
		if (arr1[i] != arr2[i]) {
			return false;
		}
	}
	return true;
}

void OutputResults(int* numbers, int* areSquares, int count, double parallelTime, int squares) {
	printf("\nParallel algorithm\n");
	for (int i = 0; i < count; i++) {
		auto s = areSquares[i] ? "" : " not";
		printf("Number %d is%s a perfect square\n", numbers[i], s);
	}
	printf("Squares found: %d\n", squares);
	printf("Execution time: %7.4fs\n", parallelTime);
	printf("---------\n");
	printf("Serial algorithm\n");

	int* areSquaresSerial = new int[count];
	int squaresSerial = 0;
	auto start = steady_clock::now();
	FindPerfectSquares(numbers, areSquaresSerial, count, squaresSerial);
	auto end = steady_clock::now();
	printf("Squares found: %d\n", squaresSerial);
	printf("Execution time: %7.4fs\n", (double)duration_cast<microseconds>(end - start).count() / 1000000.0);
}

void OutputTime(int* numbers, int* areSquares, int count, double parallelTime) {
	printf("P: %7.4fs\n", parallelTime);

	int squaresSerial = 0;
	auto start = steady_clock::now();
	FindPerfectSquares(numbers, areSquares, count, squaresSerial);
	auto end = steady_clock::now();
	printf("S: %7.4fs\n", (double)duration_cast<microseconds>(end - start).count() / 1000000.0);
}

int main(int argc, char* argv[]) {
	srand(123);
	int procNum;
	int procRank;
	int squares = 0;
	int squaresLocal = 0;
	double start;
	double end;
	double parallel;
	double serial;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int count = 1000;
	int maxNum = 100;
	int* numbers = new int[count];
	int* areSquares = new int[count];
	int part = count / procNum;
	
	if (procRank == 0) {
		Init(numbers, areSquares, count, maxNum);
	}
	
	int* bufNumbers = new int[part];
	int* bufAreSquares = new int[part];

	MPI_Scatter(numbers, part, MPI_INT, bufNumbers, part, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(areSquares, part, MPI_INT, bufAreSquares, part, MPI_INT, 0, MPI_COMM_WORLD);

	start = MPI_Wtime();
	FindPerfectSquares(bufNumbers, bufAreSquares, part, squaresLocal);
	end = MPI_Wtime();
	parallel = end - start;

	MPI_Gather(bufAreSquares, part, MPI_INT, areSquares, part, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(bufNumbers, part, MPI_INT, numbers, part, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Reduce(&squaresLocal, &squares, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Finalize();
	
	if (procRank == 0) {
		OutputResults(numbers, areSquares, count, parallel, squares);
		printf("N = %d, P = %d\n", count, procNum);
		OutputTime(numbers, areSquares, count, parallel);
		printf("---------");
	}

	return 0;
}