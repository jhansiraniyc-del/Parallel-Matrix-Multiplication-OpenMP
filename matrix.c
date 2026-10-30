
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_N 300

static double A[MAX_N][MAX_N];
static double B[MAX_N][MAX_N];
static double C[MAX_N][MAX_N];

int main(void) {
    int n, threads;

    /* Read matrix size and thread count */
    if (scanf("%d %d", &n, &threads) != 2)
        return 1;

    if (n < 1 || n > MAX_N || threads < 1 || threads > 64)
        return 1;

    /* Read matrix A */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scanf("%lf", &A[i][j]) != 1)
                return 1;
        }
    }

    /* Read matrix B */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scanf("%lf", &B[i][j]) != 1)
                return 1;
        }
    }

    /* Sequential matrix multiplication */
    double start = omp_get_wtime();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;

            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }

            C[i][j] = sum;
        }
    }

    double sequential_time = omp_get_wtime() - start;

    /* Parallel matrix multiplication */
    start = omp_get_wtime();

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;

            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }

            C[i][j] = sum;
        }
    }

    double parallel_time = omp_get_wtime() - start;

    /* Print results in JSON format */
    printf("{\"sequential_time\":%.9f,", sequential_time);
    printf("\"parallel_time\":%.9f,", parallel_time);
    printf("\"threads\":%d,\"n\":%d,\"result\":[", threads, n);

    for (int i = 0; i < n; i++) {
        if (i > 0) printf(",");

        printf("[");

        for (int j = 0; j < n; j++) {
            if (j > 0) printf(",");
            printf("%.6f", C[i][j]);
        }

        printf("]");
    }

    printf("]}\n");

    return 0;
}
