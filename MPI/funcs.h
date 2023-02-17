#include <stdio.h>
#include "mpi.h"
#include "omp.h"
#include <stdlib.h>
#include <math.h>

#define w(i, j) w[(j) * data->xcount + (i)]
#define r(i, j) r[(j) * data->xcount + (i)]
#define B(i, j) B[(j) * data->xcount + (i)]
#define Ar(i, j) Ar[(j) * data->xcount + (i)]
#define v1(i, j) v1[(j) * data->xcount + (i)]
#define v2(i, j) v2[(j) * data->xcount + (i)]
#define ret(i, j) ret[(j) * data->xcount + (i)]
#define wdiff(i, j) wdiff[(j) * data->xcount + (i)]
#define umat(i, j) umat[(j) * data->xcount + (i)]

typedef struct Data_of_proc {
    int xfrom, xto, xcount, m;
    int yfrom, yto, ycount, n;
    int k, l, p;
    double hx, hy, hxinv, hyinv, sumh;
} Data;

double u(double x, double y);
double k(double x, double y);
double q(double x, double y);
double F(double x, double y);
double psiR(double x, double y);
double psiL(double x, double y);
double psiT(double x, double y);
double psiB(double x, double y);
double rho(int row, int col, Data *data);
double gridfunc_norm(double *w, Data *data);
void initw(double *w, double val, Data *data);
void initxy(double *x, double *y, Data *data);
void r_minus_B(double *r, double *B, Data *data);
void initB(double *B, double *x, double *y, int rank, Data *data);
void w_kp1(double *wdiff, double *w, double *r, double tau, Data *data);
double scalar(double *v1, double *v2, double *x, double *y, Data *data);
double ind2x(double *w, int i, int j, double *x, double *y, Data *data);
double ind2y(double *w, int i, int j, double *x, double *y, Data *data);
double ind3x(double *w, int i, int j, double *x, double *y, Data *data);
double ind3y(double *w, int i, int j, double *x, double *y, Data *data);
void Aw(double *w, double *ret, double *x, double *y, int rank, Data *data);
double T_ind3y(double *w, double *up, int row, int col, double *x, double *y, Data *data);
double B_ind3y(double *w, double *down, int row, int col, double *x, double *y, Data *data);
double L_ind3x(double *w, double *left, int row, int col, double *x, double *y, Data *data);
double R_ind3x(double *w, double *right, int row, int col, double *x, double *y, Data *data);
void messages(double *w, int top_neigh, int bottom_neigh, int left_neigh, int right_neigh, double *top_send, double *bottom_send, double *left_send, double *right_send, double *top_recv, double *bottom_recv, double *left_recv, double *right_recv, Data *data);
