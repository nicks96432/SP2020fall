#include "threadutils.h"

inline static void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

inline static void threeNumSort(int *a, int *b, int *c)
{
	if (*a > *b)
		swap(a, b);
	if (*a > *c)
		swap(a, c);
	if (*b > *c)
		swap(b, c);
}

void BinarySearch(int thread_id, int init, int maxiter)
{
	ThreadInit(thread_id, init, maxiter);
	/*
    Some initilization if needed.
    */
	Current->y = 0;
	Current->z = 100;
	for (Current->i = 0; Current->i < Current->N; ++Current->i)
	{
		sleep(1);
		/*
         * Do the computation, then output result.
         * Call ThreadExit() if the work is done.
         */
		int mid = (Current->y + Current->z) / 2;
		printf("BinarySearch: %d\n", mid);
		if (mid == Current->x)
			ThreadExit();
		else if (mid > Current->x)
			Current->z = mid - 1;
		else
			Current->y = mid + 1;
		ThreadYield();
	}
	ThreadExit();
}

void BlackholeNumber(int thread_id, int init, int maxiter)
{
	ThreadInit(thread_id, init, maxiter);
	/*
    Some initilization if needed.
    */
	for (Current->i = 0; Current->i < Current->N; ++Current->i)
	{
		sleep(1);
		/*
        Do the computation, then output result.
        Call ThreadExit() if the work is done.
        */
		Current->z = Current->x / 100;
		Current->y = (Current->x - Current->z * 100) / 10;
		Current->x = Current->x % 10;
		threeNumSort(&Current->x, &Current->y, &Current->z);
		const int big = Current->z * 100 + Current->y * 10 + Current->x;
		const int small = Current->x * 100 + Current->y * 10 + Current->z;
		Current->x = big - small;
		const int hundred = Current->x / 100;
		const int ten = (Current->x - hundred * 100) / 10;
		const int one = Current->x % 10;
		printf("BlackholeNumber: ");
		if (hundred)
			printf("%d%d%d\n", hundred, ten, one);
		else if (ten)
			printf("%d%d\n", ten, one);
		else
			printf("%d\n", one);
		if (Current->x == 495)
			ThreadExit();
		ThreadYield();
	}
	ThreadExit();
}

void FibonacciSequence(int thread_id, int init, int maxiter)
{
	ThreadInit(thread_id, init, maxiter);
	/*
    Some initilization if needed.
    */
	Current->y = 0;
	for (Current->i = 0; Current->i < Current->N; ++Current->i)
	{
		sleep(1);
		/*
        Do the computation, then output result.
        */
		Current->z = Current->x + Current->y;
		printf("FibonacciSequence: %d\n", Current->z);
		Current->y = Current->x;
		Current->x = Current->z;
		ThreadYield();
	}
	ThreadExit();
}
