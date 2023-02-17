#include "funcs.h"

double u(double x, double y) {
    return sqrt(4.0 + x * y);
}

double k(double x, double y) {
    return 4.0 + x + y;
}

double q(double x, double y) {
    return x + y;
}

double F(double x, double y) {
    double xpysq = x * x + y * y;
    double uxy = u(x, y);
    double qxy = q(x, y);
    return 0.25 * k(x, y) * xpysq / uxy / uxy / uxy - 0.5 * qxy / uxy + qxy * uxy;
}

double psiR(double x, double y) {
    double uxy = u(x, y);
    return uxy + 0.5 * y * k(x, y) / uxy;
}

double psiL(double x, double y) {
    double uxy = u(x, y);
    return uxy - 0.5 * y * k(x, y) / uxy;
}

double psiT(double x, double y) {
    return 0.5 * x * k(x, y) / u(x, y);
}

double psiB(double x, double y) {
    return -psiT(x, y);
}

void initxy(double *x, double *y, Data *data) {
#pragma omp parallel for private(i, col)
    for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++)
        x[i] = col * data->hx;
#pragma omp parallel for private(j, row)
    for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++)
        y[j] = row * data->hy;
}

void initw(double *w, double val, Data *data) {
#pragma omp parallel for private(i, j)
    for (int i = 0; i < data->xcount; i++)
        for (int j = 0; j < data->ycount; j++)
            w(i, j) = val;
}

void initB(double *B, double *x, double *y, int rank, Data *data) {
#pragma omp parallel for private(i, col, j, row)
    for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
        if (col == 0 || col == data->m) continue;
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
            if (row == 0 || row == data->n) continue;
            B(i, j) = F(x[i], y[j]); // (7) B_ij = F_ij
        }
    }
    if (data->xfrom == 0) { // (9) Left
#pragma omp parallel for private(j, row)
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
            if (row == 0 || row == data->n) continue;
            B(0, j) = F(x[0], y[j]) + data->hxinv * psiL(x[0], y[j]);
        }
    }
    if (data->xto == data->m) { // (9) Right
#pragma omp parallel for private(j, row)
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
            if (row == 0 || row == data->n) continue;
            B(data->xcount - 1, j) = F(x[data->xcount - 1], y[j]) + data->hxinv * psiR(x[data->xcount - 1], y[j]);
        }
    }
    if (data->yfrom == 0) { // (10) Bottom
#pragma omp parallel for private(i, col)
        for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
            if (col == 0 || col == data->m) continue;
            B(i, 0) = F(x[i], y[0]) + data->hyinv * psiB(x[i], y[0]);
        }
    }
    if (data->yto == data->n) { // (10) Top
#pragma omp parallel for private(i, col)
        for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
            if (col == 0 || col == data->m) continue;
            B(i, data->ycount - 1) = F(x[i], y[data->ycount - 1]) + data->hyinv * psiT(x[i], y[data->ycount - 1]);
        }
    }
    if (data->xfrom == 0 && data->yfrom == 0) // (11) top-left or (A1, B1) bottom-left in math
        B(0, 0) = F(0, 0) + data->sumh * (data->hx * psiL(x[0], y[0]) + data->hy * psiL(x[0], y[0])) / (data->hx + data->hy);
    if (data->xto == data->m && data->yfrom == 0) // (12) top-right or (A2, B1) bottom-right in math
        B(data->xcount - 1, 0) = F(x[data->xcount - 1], y[0]) + data->sumh * (data->hx * psiB(x[data->xcount - 1], y[0]) + data->hy * psiR(x[data->xcount - 1], y[0])) / (data->hx + data->hy);
    if (data->xto == data->m && data->yto == data->n) // (13) bottom-left or (A2, B2) top-right
        B(data->xcount - 1, data->ycount - 1) = F(x[data->xcount - 1], y[data->ycount - 1]) + data->sumh * (data->hx * psiT(x[data->xcount - 1], y[data->ycount - 1]) + data->hy * psiR(x[data->xcount - 1], y[data->ycount - 1])) / (data->hx + data->hy);
    if (data->xfrom == 0 && data->yto == data->n) // (14) bottom-right or (A1, B2) top-left
        B(0, data->ycount - 1) = F(x[0], y[data->ycount - 1]) + data->sumh * (data->hx * psiT(x[0], y[data->ycount - 1]) + data->hy * psiL(x[0], y[data->ycount - 1])) / (data->hx + data->hy);
    MPI_Barrier(MPI_COMM_WORLD);
}

double ind2x(double *w, int i, int j, double *x, double *y, Data *data) {       //       ------ ------ ------ ------    no. | top | bottom | left | right
    double kxy = k(x[i] - 0.5 * data->hx, y[j]);                                //      |      |      |      |      |   00  |  -1 |   04   |  -1  |  01  
    double wleft = w(i, j) - w(i - 1, j);                                       //      |  00  |  01  |  02  |  03  |   01  |  -1 |   05   |  00  |  02  
    return 1.0/data->hx * kxy * wleft;                                          //      |      |      |      |      |   02  |  -1 |   06   |  01  |  03  
}                                                                               //       ------ ------ ------ ------    03  |  -1 |   07   |  02  |  -1  
                                                                                //      |      |      |      |      |   04  |  00 |   08   |  -1  |  05  
double ind2y(double *w, int i, int j, double *x, double *y, Data *data) {       //      |  04  |  05  |  06  |  07  |   05  |  01 |   09   |  04  |  06  
    double kxy = k(x[i], y[j] - 0.5 * data->hy);                                //      |      |      |      |      |   06  |  02 |   10   |  05  |  07  
    double wleft = w(i, j) - w(i, j - 1);                                       //       ------ ------ ------ ------    07  |  03 |   11   |  06  |  -1  
    return 1.0/data->hy * kxy * wleft;                                          //      |      |      |      |      |   08  |  04 |   12   |  -1  |  09  
}                                                                               //      |  08  |  09  |  10  |  11  |   09  |  05 |   13   |  08  |  10  
                                                                                //      |      |      |      |      |   10  |  06 |   14   |  09  |  11  
double ind3x(double *w, int i, int j, double *x, double *y, Data *data) {       //       ------ ------ ------ ------    11  |  07 |   15   |  10  |  -1  
    double kpxy = k(x[i] + 0.5 * data->hx, y[j]);                               //      |      |      |      |      |   12  |  08 |   -1   |  -1  |  13  
    double wright = w(i + 1, j) - w(i, j);                                      //      |  12  |  13  |  14  |  15  |   13  |  09 |   -1   |  12  |  14  
    double kxy = k(x[i] - 0.5 * data->hx, y[j]);                                //      |      |      |      |      |   14  |  10 |   -1   |  13  |  15  
    double wleft = w(i, j) - w(i - 1, j);                                       //       ------ ------ ------ ------    15  |  11 |   -1   |  14  |  -1  
    return 1.0/data->hx/data->hx * (kpxy * wright - kxy * wleft);
}

double ind3y(double *w, int i, int j, double *x, double *y, Data *data) {
    double kpxy = k(x[i], y[j] + 0.5 * data->hy);
    double wright = w(i, j + 1) - w(i, j);
    double kxy = k(x[i], y[j] - 0.5 * data->hy);
    double wleft = w(i, j) - w(i, j - 1);
    return 1.0/data->hy/data->hy * (kpxy * wright - kxy * wleft);
}

double T_ind3y(double *w, double *top, int i, int j, double *x, double *y, Data *data) {
    double kpxy = k(x[i], y[j] + 0.5 * data->hy);
    double wright = w(i, j + 1) - w(i, j);
    double kxy = k(x[i], y[j] - 0.5 * data->hy);
    double wleft = w(i, j) - top[i];
    return 1.0/data->hy/data->hy * (kpxy * wright - kxy * wleft);
}

double B_ind3y(double *w, double *bottom, int i, int j, double *x, double *y, Data *data) {
    double kpxy = k(x[i], y[j] + 0.5 * data->hy);
    double wright = bottom[i] - w(i, j);
    double kxy = k(x[i], y[j] - 0.5 * data->hy);
    double wleft = w(i, j) - w(i, j - 1);
    return 1.0/data->hy/data->hy * (kpxy * wright - kxy * wleft);
}

double L_ind3x(double *w, double *left, int i, int j, double *x, double *y, Data *data) {
    double kpxy = k(x[i] + 0.5 * data->hx, y[j]);
    double wright = w(i + 1, j) - w(i, j);
    double kxy = k(x[i] - 0.5 * data->hx, y[j]);
    double wleft = w(i, j) - left[j];
    return 1.0/data->hx/data->hx * (kpxy * wright - kxy * wleft);
}

double R_ind3x(double *w, double *right, int i, int j, double *x, double *y, Data *data) {
    double kpxy = k(x[i] + 0.5 * data->hx, y[j]);
    double wright = right[j] - w(i, j);
    double kxy = k(x[i] - 0.5 * data->hx, y[j]);
    double wleft = w(i, j) - w(i - 1, j);
    return 1.0/data->hx/data->hx * (kpxy * wright - kxy * wleft);
}

void Aw(double *w, double *ret, double *x, double *y, int rank, Data *data) {
    // row, col -- real rows/cols
    // i, j -- rows/cols of each proc
    // START dividing matrix into top, bottom, left, right parts
    double *top_send = NULL, *bottom_send = NULL, *left_send = NULL, *right_send = NULL;
    double *top_recv = NULL, *bottom_recv = NULL, *left_recv = NULL, *right_recv = NULL;
    int top_neigh = -1, bottom_neigh = -1, left_neigh = -1, right_neigh = -1;
    if (data->yfrom != 0) {
        top_send = (double*)malloc(data->xcount * sizeof(double));
        top_recv = (double*)malloc(data->xcount * sizeof(double));
        top_neigh = rank - data->k;
    }
    if (data->yto != data->n) {
        bottom_send = (double*)malloc(data->xcount * sizeof(double));
        bottom_recv = (double*)malloc(data->xcount * sizeof(double));
        bottom_neigh = rank + data->k;
    }
    if (data->xfrom != 0) {
        left_send = (double*)malloc(data->ycount * sizeof(double));
        left_recv = (double*)malloc(data->ycount * sizeof(double));
        left_neigh = rank - 1;
    }
    if (data->xto != data->m) {
        right_send = (double*)malloc(data->ycount * sizeof(double));
        right_recv = (double*)malloc(data->ycount * sizeof(double));
        right_neigh = rank + 1;
    }
    // top_send[i] = w(i, "yfrom - 1"), bottom_send[i] = w(i, "yto + 1"), left_send[j] = w("xfrom - 1", j), right_send[j] = w("xto + 1", j)
    if (data->p != 1) {
        messages(w, top_neigh, bottom_neigh, left_neigh, right_neigh,\
                 top_send, bottom_send, left_send, right_send,\
                 top_recv, bottom_recv, left_recv, right_recv, data);
    }
    // END dividing
    // (7) ret_ij = q_ij * w_ij - ind3x - ind3y
#pragma omp parallel for private(i, col, j, row)
    for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
        if (col == 0 || col == data->m) continue;
        if (col == data->xfrom) { // subleft-...
            for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
                if (row == 0 || row == data->n) continue;
                if (row == data->yfrom) { // subleft-subtop = one point
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - L_ind3x(w, left_recv, i, j, x, y, data) - T_ind3y(w, top_recv, i, j, x, y, data);
                } else if (row == data->yto) { // subleft-subbottom = one point
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - L_ind3x(w, left_recv, i, j, x, y, data) - B_ind3y(w, bottom_recv, i, j, x, y, data);
                } else { // subleft
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - L_ind3x(w, left_recv, i, j, x, y, data) - ind3y(w, i, j, x, y, data);
                }
            }
        } else if (col == data->xto) { // subright-...
            for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
                if (row == 0 || row == data->n) continue;
                if (row == data->yfrom) { // subright-subtop = one point
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - R_ind3x(w, right_recv, i, j, x, y, data) - T_ind3y(w, top_recv, i, j, x, y, data);
                } else if (row == data->yto) { // subright-subbottom = one point
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - R_ind3x(w, right_recv, i, j, x, y, data) - B_ind3y(w, bottom_recv, i, j, x, y, data);
                } else { // subright
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - R_ind3x(w, right_recv, i, j, x, y, data) - ind3y(w, i, j, x, y, data);
                }
            }
        } else {
            for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
                if (row == 0 || row == data->n) continue;
                if (row == data->yfrom) { // subtop
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - ind3x(w, i, j, x, y, data) - T_ind3y(w, top_recv, i, j, x, y, data);
                } else if (row == data->yto) { // subbottom
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - ind3x(w, i, j, x, y, data) - B_ind3y(w, bottom_recv, i, j, x, y, data);
                } else { // in
                    ret(i, j) = q(x[i], y[j]) * w(i, j) - ind3x(w, i, j, x, y, data) - ind3y(w, i, j, x, y, data);
                }
            }
        }
    }
    // (9)
    if (data->xto == data->m) { // real right
#pragma omp parallel for private(j, row)
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
            if (row == 0 || row == data->n) continue;
            if (j == 0) { // problems with top. ind3y will use j = 0+1, 0, 0-1
                ret(data->xcount - 1, j) = data->hxinv * ind2x(w, data->xcount - 1, j, x, y, data) + (q(x[data->xcount - 1], y[j]) + data->hxinv) * w(data->xcount - 1, j) - T_ind3y(w, top_recv, data->xcount - 1, j, x, y, data);
            } else if (j == data->ycount - 1) { // problems with bottom. ind3y will use j = (ycount-1)+1, ycount-1, (ycount-1)-1
                ret(data->xcount - 1, j) = data->hxinv * ind2x(w, data->xcount - 1, j, x, y, data) + (q(x[data->xcount - 1], y[j]) + data->hxinv) * w(data->xcount - 1, j) - B_ind3y(w, bottom_recv, data->xcount - 1, j, x, y, data);
            } else {
                ret(data->xcount - 1, j) = data->hxinv * ind2x(w, data->xcount - 1, j, x, y, data) + (q(x[data->xcount - 1], y[j]) + data->hxinv) * w(data->xcount - 1, j) - ind3y(w, data->xcount - 1, j, x, y, data);
            }
        }
    }
    if (data->xfrom == 0) { // real left
#pragma omp parallel for private(j, row)
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++) {
            if (row == 0 || row == data->n) continue;
            if (j == 0) { // problems with top. ind3y will use j = 0+1, 0, 0-1
                ret(0, j) = -data->hxinv * ind2x(w, 1, j, x, y, data) + (q(x[0], y[j]) + data->hxinv) * w(0, j) - T_ind3y(w, top_recv, 0, j, x, y, data);
            } else if (j == data->ycount - 1) { // problems with bottom. ind3y will use j = (ycount-1)+1, ycount-1, (ycount-1)-1
                ret(0, j) = -data->hxinv * ind2x(w, 1, j, x, y, data) + (q(x[0], y[j]) + data->hxinv) * w(0, j) - B_ind3y(w, bottom_recv, 0, j, x, y, data);
            } else {
                ret(0, j) = -data->hxinv * ind2x(w, 1, j, x, y, data) + (q(x[0], y[j]) + data->hxinv) * w(0, j) - ind3y(w, 0, j, x, y, data);
            }
        }
    }
    // (10)
    if (data->yto == data->n) { // real bottom
#pragma omp parallel for private(i, col)
        for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
            if (col == 0 || col == data->m) continue;
            if (i == 0) { // problems with left. ind3x will use i = 0+1, 0, 0-1
                ret(i, data->ycount - 1) = data->hyinv * ind2y(w, i, data->ycount - 1, x, y, data) + q(x[i], y[data->ycount - 1]) * w(i, data->ycount - 1) - L_ind3x(w, left_recv, i, data->ycount - 1, x, y, data);
            } else if (i == data->xcount - 1) { // problems with right. ind3x will use i = (xcount-1)+1, xcount-1, (xcount-1)-1
                ret(i, data->ycount - 1) = data->hyinv * ind2y(w, i, data->ycount - 1, x, y, data) + q(x[i], y[data->ycount - 1]) * w(i, data->ycount - 1) - R_ind3x(w, right_recv, i, data->ycount - 1, x, y, data);
            } else {
                ret(i, data->ycount - 1) = data->hyinv * ind2y(w, i, data->ycount - 1, x, y, data) + q(x[i], y[data->ycount - 1]) * w(i, data->ycount - 1) - ind3x(w, i, data->ycount - 1, x, y, data);
            }
        }
    }
    if (data->yfrom == 0) { // real top
#pragma omp parallel for private(i, col)
        for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++) {
            if (col == 0 || col == data->m) continue;
            if (i == 0) { // problems with left. ind3x will use i = 0+1, 0, 0-1
                ret(i, 0) = -data->hyinv * ind2y(w, i, 1, x, y, data) + q(x[i], y[0]) * w(i, 0) - L_ind3x(w, left_recv, i, 0, x, y, data);
            } else if (i == data->xcount - 1) { // problems with right. ind3x will use i = (xcount-1)+1, xcount-1, (xcount-1)-1
                ret(i, 0) = -data->hyinv * ind2y(w, i, 1, x, y, data) + q(x[i], y[0]) * w(i, 0) - R_ind3x(w, right_recv, i, 0, x, y, data);
            } else {
                ret(i, 0) = -data->hyinv * ind2y(w, i, 1, x, y, data) + q(x[i], y[0]) * w(i, 0) - ind3x(w, i, 0, x, y, data);
            }
        }
    }

    if (data->xfrom == 0 && data->yfrom == 0) // top-left
        ret(0, 0) = -data->hxinv * ind2x(w, 1, 0, x, y, data) - data->hyinv * ind2y(w, 0, 1, x, y, data) + (q(x[0], y[0]) + data->sumh) * w(0, 0);
    if (data->xto == data->m && data->yfrom == 0) // top-right
        ret(data->xcount - 1, 0) = data->hxinv * ind2x(w, data->xcount - 1, 0, x, y, data) - data->hyinv * ind2y(w, data->xcount - 1, 1, x, y, data) + (q(x[data->xcount - 1], y[0]) + data->sumh) * w(data->xcount - 1, 0);
    if (data->xto == data->m && data->yto == data->n) // bottom-right
        ret(data->xcount - 1, data->ycount - 1) = data->hxinv * ind2x(w, data->xcount - 1, data->ycount - 1, x, y, data) + data->hyinv * ind2y(w, data->xcount - 1, data->ycount - 1, x, y, data) + (q(x[data->xcount - 1], y[data->ycount - 1]) + data->sumh) * w(data->xcount - 1, data->ycount - 1);
    if (data->xfrom == 0 && data->yto == data->n) // bottom-left
        ret(0, data->ycount - 1) = -data->hxinv * ind2x(w, 1, data->ycount - 1, x, y, data) + data->hyinv * ind2y(w, 0, data->ycount - 1, x, y, data) + (q(x[0], y[data->ycount - 1]) + data->sumh) * w(0, data->ycount - 1);

    if (top_send) free(top_send);
    if (top_recv) free(top_recv);
    if (bottom_send) free(bottom_send);
    if (bottom_recv) free(bottom_recv);
    if (left_send) free(left_send);
    if (left_recv) free(left_recv);
    if (right_send) free(right_send);
    if (right_recv) free(right_recv);
    MPI_Barrier(MPI_COMM_WORLD);
}

double rho(int row, int col, Data *data) {
    double rho1 = 1.0, rho2 = 1.0;
    if (row == 0 || row == data->n) rho1 = 0.5;
    if (col == 0 || col == data->m) rho2 = 0.5;
    return rho1 * rho2;
}

double scalar(double *v1, double *v2, double *x, double *y, Data *data) {
    double loc_ans = 0.0, ans = 0.0;
#pragma omp parallel for private(i, col, j, row)
    for (int i = 0, col = data->xfrom; i < data->xcount; i++, col++)
        for (int j = 0, row = data->yfrom; j < data->ycount; j++, row++)
            loc_ans += data->hx * data->hy * rho(row, col, data) * v1(i, j) * v2(i, j);
    MPI_Reduce(&loc_ans, &ans, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ans, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return ans;
}

double gridfunc_norm(double *w, Data *data) {
    double loc_max = -1.0, max = -1.0;
#pragma omp parallel for private(i, j)
    for (int i = 0; i < data->xcount; i++)
        for (int j = 0; j < data->ycount; j++) {
            double wij = fabs(w(i, j));
            if (wij > loc_max)
                loc_max = wij;
        }
    MPI_Reduce(&loc_max, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return max;
}

void r_minus_B(double *r, double *B, Data *data) {
#pragma omp parallel for private(i, j)
    for (int i = 0; i < data->xcount; i++)
        for (int j = 0; j < data->ycount; j++)
            r(i, j) -= B(i, j);
    MPI_Barrier(MPI_COMM_WORLD);
}

void w_kp1(double *wdiff, double *w, double *r, double tau, Data *data) {
#pragma omp parallel for private(i, j)
    for (int i = 0; i < data->xcount; i++)
        for (int j = 0; j < data->ycount; j++) {
            wdiff(i, j) = -tau * r(i, j);
            w(i, j) += wdiff(i, j);
        }
    MPI_Barrier(MPI_COMM_WORLD);
}

void messages(double *w, int top_neigh, int bottom_neigh, int left_neigh, int right_neigh,\
              double *top_send, double *bottom_send, double *left_send, double *right_send,\
              double *top_recv, double *bottom_recv, double *left_recv, double *right_recv, Data *data) {
    int R_TAG = 1, L_TAG = 2, T_TAG = 3, B_TAG = 4;
    if (top_neigh != -1)
#pragma omp parallel for private(i)
        for (int i = 0; i < data->xcount; i++)
            top_send[i] = w(i, 0);
    if (bottom_neigh != -1)
#pragma omp parallel for private(i)
        for (int i = 0; i < data->xcount; i++)
            bottom_send[i] = w(i, data->ycount - 1);
    if (left_neigh != -1)
#pragma omp parallel for private(j)
        for (int j = 0; j < data->ycount; j++)
            left_send[j] = w(0, j);
    if (right_neigh != -1)
#pragma omp parallel for private(j)
        for (int j = 0; j < data->ycount; j++)
            right_send[j] = w(data->xcount - 1, j);

    if (top_neigh != -1 && bottom_neigh != -1) { // send to top. not toppest and not bottomest
        MPI_Sendrecv(top_send, data->xcount, MPI_DOUBLE, top_neigh, T_TAG,\
                     bottom_recv, data->xcount, MPI_DOUBLE, bottom_neigh, T_TAG,\
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (top_neigh == -1) { // sendED to top. toppest receives bottom_recv from bottom_neigh
        MPI_Recv(bottom_recv, data->xcount, MPI_DOUBLE, bottom_neigh, T_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (bottom_neigh == -1) { // sendED to top. bottomest sends top_send to top_neigh
        MPI_Send(top_send, data->xcount, MPI_DOUBLE, top_neigh, T_TAG, MPI_COMM_WORLD);
    }
    if (bottom_neigh != -1 && top_neigh != -1) { // send to bottom. not bottomest and not toppest
        MPI_Sendrecv(bottom_send, data->xcount, MPI_DOUBLE, bottom_neigh, B_TAG,\
                     top_recv, data->xcount, MPI_DOUBLE, top_neigh, B_TAG,\
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (bottom_neigh == -1) { // sendED to bottom. bottomest receives top_recv from top_neigh
        MPI_Recv(top_recv, data->xcount, MPI_DOUBLE, top_neigh, B_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (top_neigh == -1) { // sendED to bottom. toppest sends bottom_send to bottom_neigh
        MPI_Send(bottom_send, data->xcount, MPI_DOUBLE, bottom_neigh, B_TAG, MPI_COMM_WORLD);
    }
    if (data->p >= 4) {
        if (left_neigh != -1 && right_neigh != -1) { // send to left. not leftest and not rightest
            MPI_Sendrecv(left_send, data->ycount, MPI_DOUBLE, left_neigh, L_TAG,\
                         right_recv, data->ycount, MPI_DOUBLE, right_neigh, L_TAG,\
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (left_neigh == -1) { // sendED to left. leftest receives right_recv from right_neigh
            MPI_Recv(right_recv, data->ycount, MPI_DOUBLE, right_neigh, L_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (right_neigh == -1) { // sendED to left. rightest sends left_send to left_neigh
            MPI_Send(left_send, data->ycount, MPI_DOUBLE, left_neigh, L_TAG, MPI_COMM_WORLD);
        }
        if (right_neigh != -1 && left_neigh != -1) { // send to right. not rightest and not leftest
            MPI_Sendrecv(right_send, data->ycount, MPI_DOUBLE, right_neigh, R_TAG,\
                         left_recv, data->ycount, MPI_DOUBLE, left_neigh, R_TAG,\
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (right_neigh == -1) { // sendED to right. rightest receives left_recv from left_neigh
            MPI_Recv(left_recv, data->ycount, MPI_DOUBLE, left_neigh, R_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (left_neigh == -1) { // sendED to right. leftest sends right_send to right_neigh
            MPI_Send(right_send, data->ycount, MPI_DOUBLE, right_neigh, R_TAG, MPI_COMM_WORLD);
        }
    }
}
