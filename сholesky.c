#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define eps 1e-18

void Main(int, double *, double *, double *, double *);
void Print(int, double *);
void Inverse(int, double *, double *); // result in second matrix = Inv
void Init(int, double *, double *); // initialize identity matrix with lower-triangle elements of A
void Scan(FILE *, int, double *); // scan matrix A
void Decompose(int, double *, double *); // A = R(^T) * D * R
void L_D(int, double *, double *); // multiply lower-triangle matrix by diagonal matrix
void Copy(int, double *, double *); // copy Inv to A with transposed elements
void Diagonal(int, double *, double *); // fix diagonal elements of A
void lanogaiD(int, double *, double *); // vise a versa
void U_L(int, double *, double *); // multiply lower-triangle matrix by upper-triangle
void Symmetry(int, double *); // from lower-triangle to symmetrical
void yrtemmyS(int, double *); // from upper-triangle to symmetrical

int  Check(int, double *); // check for first minor and symmetry
int  Sign(double); // sign of real number

double Sum(int, int, int, double *, double *); // sum from index_1 to index_2 of r and D
double Frobenius(int, double *, double *); // frobenius' norm
double MaxRowNorm(int, double *); // maximum row norm

int main(int argc, char ** argv) {
	if (argc == 1) {
		printf("Exit status: 667 NO PARAMETERS\nGive file name as parameter, for example HILBERT.txt\n");
		return 667;
	}
	FILE *file;
	int n, check;
	double * Inv = NULL, * R = NULL, * D = NULL, * R_D = NULL;
	file = fopen(argv[1], "r");
	if (file == NULL) {
		printf("Exit status 404: NOT FOUND\n");
		return 404;
	}
	if (fscanf(file, "%d", &n) == EOF) {
		printf("Exit status 666: EMPTY FILE\n");
		return 666;
	}
	R = malloc(n * n * sizeof(double));

	Scan(file, n, R);
	fclose(file);
	check = Check(n, R);
	if (check == -1) {
		printf("Exit status 13: FIRST MINOR IS 0\n");
		free(R);
		return 13;
	}
	if (check == 0) {
		printf("Exit status 47: NOT SYMMETRICAL MATRIX\n");
		free(R);
		return 47;
	}

	Inv = malloc(n * n * sizeof(double)); // A^-1
	D = malloc(n * sizeof(double)); // diagonal matrix D
	R_D = malloc(n * sizeof(double)); // diagonal elements of A

	Main(n, R, Inv, D, R_D);

	free(R);
	free(Inv);
	free(D);
	free(R_D);
	return 0;
}

void Main(int n, double * R, double * Inv, double * D, double * R_D) {
		Init(n, Inv, R); // identity matrix with lower-triangle matrix A
		Diagonal(n, R, R_D); // fix diagonal elements of A
		Decompose(n, D, R); // Matrix D
		Inverse(n, R, Inv); // Matrix R^-1
		Copy(n, R, Inv); // copy Inv to R with transposed elements
		L_D(n, Inv, D); // R^-1 * D
		U_L(n, Inv, R); // R^-1 * D * (R^-1)^T
		Symmetry(n, Inv); // make symmetrical matrix from lower-triangle Inv
		Print(n, Inv); // answer
		yrtemmyS(n, R); // restore matrix A
		lanogaiD(n, R_D, R); // restore matrix A
		printf("Frobenius' norm:  %e\n", Frobenius(n, R, Inv)); // ||A * A^(-1) - I||, result in R = A * A^(-1) - I
		printf("Maximum row norm: %e\n", MaxRowNorm(n, R)); // maximum
}

void Print(int n, double * R) {
	printf ("\n");

	if (n < 50) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++)
				printf ("%10.7lf ", R[i * n + j]);
			printf ("\n");
		}
	} else {
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++)
				printf ("%10.3e ", R[i * n + j]);
			printf(" ... ");
			printf("%10.3e\n", R[i * n + (n - 1)]);
		}
		for (int i = 0; i < 10; i++)
			printf("     .     ");
		printf(" .       ");
		printf(" .     \n");
		for (int i = 0; i < 10; i++)
			printf("     .     ");
		printf("  .      ");
		printf(" .     \n");
		for (int i = 0; i < 10; i++)
			printf("     .     ");
		printf("   .     ");
		printf(" .     \n");
		for (int j = 0; j < 10; j++)
			printf ("%10.3e ", R[(n - 1) * n + j]);
		printf(" ... ");
		printf("%10.3e\n", R[(n - 1) * n + (n - 1)]);
	}

	printf ("\n");
}

void Inverse(int n, double * R, double * Inv) {
	for (int j = n - 1; j >= 1; j--)
		for (int i = j - 1; i >= 0; i--)
			for (int k = j; k < n; k++)
				Inv[i * n + k] -= Inv[j * n + k] * R[i * n + j] / R[j * n + j];

	for (int i = 0; i < n; i++)
		for (int j = i; j < n; j++)
			Inv[i * n + j] /= R[i * n + i];
}

void Init(int n, double * Inv, double * R) {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			if (i == j)
				Inv[i * n + j] = 1;
			else if (j > i)
				Inv[i * n + j] = 0;
			else
				Inv[i * n + j] = R[i * n + j];

		}
}

void Scan(FILE * file, int n, double * R) {
	for (int i = 0; i < n; i++) 
		for (int j = 0; j < n; j++)
			fscanf(file, "%lf", &R[i * n + j]);
}

void Decompose(int n, double * D, double * R) {
	double sigma;
	D[0] = Sign(R[0]);
	R[0] = sqrt(fabs(R[0]));
	for (int j = 1; j < n; j++)
		R[j] = R[j] / R[0] / D[0];
	for (int i = 1; i < n; i++) {
		sigma = Sum(i, i, n, D, R);
		D[i] = Sign(R[i * n + i] - sigma);
		R[i * n + i] = sqrt(fabs(R[i * n + i] - sigma));
		for (int j = i + 1; j < n; j++)
			R[i * n + j] = (R[i * n + j] - Sum(i, j, n, D, R)) / R[i * n + i] / D[i];
	}
}

void L_D(int n, double * R, double * D) {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			R[i * n + j] *= D[j];
}

void Copy(int n, double * R, double * Inv) {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			R[j * n + i] = Inv[i * n + j];
}

void Diagonal(int n, double * R, double * R_D) {
	for (int i = 0; i < n; i++)
		R_D[i] = R[i * n + i];
}

void lanogaiD(int n, double * R_D, double * R) {
	for (int i = 0; i < n; i++)
		R[i * n + i] = R_D[i];
}

void U_L(int n, double * Inv, double * R) {
	double * tmp = malloc(n * sizeof(double));
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			tmp[j] = Inv[i * n + j];
			if (j < i)
				tmp[j] = 0;
		}
		for (int j = 0; j < n; j++) {
			Inv[i * n + j] = 0;
			for (int k = 0; k < n; k++) {
				Inv[i * n + j] += tmp[k] * R[k * n + j];
			}
		}
	}
	free(tmp);
}

void Symmetry(int n, double * Inv) {
	for (int j = 0; j < n; j++)
		for (int i = j + 1; i < n; i++)
			Inv[j * n + i] = Inv[i * n + j];
}

void yrtemmyS(int n, double * R) {
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
			R[j * n + i] = R[i * n + j];
}

int Check(int n, double * R) {
	if (R[0] == 0)
		return -1;
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
			if (R[i * n + j] != R[j * n + i])
				return 0;
	return 1;
}

int Sign(double x) {
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

double Sum(int index_1, int index_2, int n, double * d, double * R) {
	double ans = 0;
	for (int k = 0; k < index_1; k++)
		ans += R[k * n + index_1] * d[k] * R[k * n + index_2];
	return ans;
}

double Frobenius(int n, double * a, double * b) {
	double * tmp = malloc(n * sizeof(double));
	double norm = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) 
			tmp[j] = a[i * n + j];
		for (int j = 0; j < n; j++) {
			a[i * n + j] = 0;
			for (int k = 0; k < n; k++) {
				a[i * n + j] += tmp[k] * b[k * n + j];
			}
			if (i == j)
				a[i * n + j] -= 1;
			norm += a[i * n + j] * a[i * n + j];
		}
	}
	norm = sqrt(norm);
	free(tmp);
	return norm;
}

double MaxRowNorm(int n, double * R) {
	double max = 0;
	double sum;
	for (int i = 0; i < n; i++) {
		sum = 0;
		for (int j = 0; j < n; j++)
			sum += fabs(R[i * n + j]);
		if (sum > max)
			max = sum;
	}
	return max;
}
