#define _GNU_SOURCE
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

double f(double x) {
    return exp(-x*x);
}
double result = 0;
const int nthreads = 32767;
int xleft = -4, xright = 4, n = 0x8000000 / nthreads;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *monte () {
    struct drand48_data rs;
    double avg = 0;
    srand48_r(rand(), &rs);
    for(int i = 0; i < n; i++) {
        double r = 0;
        drand48_r(&rs, &r);
        avg += f(xleft + r * (xright - xleft));
    }
    pthread_mutex_lock(&mutex);
    result += avg / n / nthreads * (xright - xleft);
    pthread_mutex_unlock(&mutex);
}

int main() {
    pthread_t threads[nthreads];
    for(int i = 0; i < nthreads; i++)  
        pthread_create(&threads[i], NULL, monte, NULL);
    for(int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);
    printf("%lf\n", result);
}