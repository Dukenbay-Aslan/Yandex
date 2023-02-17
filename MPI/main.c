#include "funcs.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Wrong number of arguments, required 2: M N\n");
        return -1;
    }

    double eps = 1.e-4;
    int M = strtol(argv[1], NULL, 0);
    int N = strtol(argv[2], NULL, 0);

    int rank, p;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int procs_x;
    if (p == 1 || p == 2) procs_x = 1;
    else if (p == 4 || p == 8) procs_x = 2;
    else if (p == 16 || p == 32) procs_x = 4;
    int procs_y = p / procs_x;

    Data data;
    data.m = M; data.n = N; data.p = p;
    data.k = procs_x; data.l = procs_y;
    for (int t = 0; t < procs_x; t++) {
        if (rank % procs_x == t) {
            data.xfrom = t * (M + 1) / procs_x;
            data.xto = (t + 1) * (M + 1) / procs_x - 1;
            data.xcount = data.xto - (data.xfrom - 1);
        }
    }
    for (int s = 0; s < procs_y; s++) {
        if (rank / procs_x == s) {
            data.yfrom = s * (N + 1) / procs_y;
            data.yto = (s + 1) * (N + 1) / procs_y - 1;
            data.ycount = data.yto - (data.yfrom - 1);
        }
    }
    data.hx = 4.0 / data.m; data.hy = 3.0 / data.n;
    data.hxinv = 2.0/data.hx; data.hyinv = 2.0/data.hy, data.sumh = data.hxinv + data.hyinv;

    double *x = (double*)malloc(data.xcount * sizeof(double));
    double *y = (double*)malloc(data.ycount * sizeof(double));
    double *w = (double*)malloc(data.ycount * data.xcount * sizeof(double));
    double *r = (double*)malloc(data.ycount * data.xcount * sizeof(double));
    double *B = (double*)malloc(data.ycount * data.xcount * sizeof(double));
    double *Ar = (double*)malloc(data.ycount * data.xcount * sizeof(double));
    double *wdiff = (double*)malloc(data.ycount * data.xcount * sizeof(double));

    initxy(x, y, &data);
    initB(B, x, y, rank, &data);
    initw(w, 2.5, &data);
    MPI_Barrier(MPI_COMM_WORLD);

    double w_norm = 1.0;
    int step = 0;

    double time = -1.0, loc_time = MPI_Wtime();
    while (w_norm > eps && step < 128000) {
        Aw(w, r, x, y, rank, &data);
        r_minus_B(r, B, &data);
        Aw(r, Ar, x, y, rank, &data);
        double Ar_r = scalar(Ar, r, x, y, &data);
        double Ar_Ar = scalar(Ar, Ar, x, y, &data);
        double tau = Ar_r / Ar_Ar;
        w_kp1(wdiff, w, r, tau, &data);
        w_norm = gridfunc_norm(wdiff, &data);
        step++;
    }
    loc_time = MPI_Wtime() - loc_time;
    MPI_Reduce(&loc_time, &time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0)
        printf("norm = %e, step = %d, time = %lf\n", w_norm, step, time);

    if (x) free(x);
    if (y) free(y);
    if (w) free(w);
    if (r) free(r);
    if (B) free(B);
    if (Ar) free(Ar);
    if (wdiff) free(wdiff);
    MPI_Finalize();

    return 0;
}
