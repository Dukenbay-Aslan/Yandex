/*
						Задание 5. Класс Permutation
	Реализовать класс Permutation, представляющий перестановку чисел от 1 до n. Класс должен иметь два поля:
		а) указатель на int-овый массив, хранящий перестановку (образ отображения); 
		б) размер массива.
	Реализовать следующий методы:
		1  конструктор Permutation(int n), создающий тождественную перестановку;
		2  конструктор Permutation(int n, int * mas), создающий перестановку на основе массива mas;
		3  копирующий конструктор;
		4  деструктор;
		5  метод печати;
		6  метод, печатающий разложение перестанвоки в произведение непересекающийхся циклов;
		7  метод, печатающий длины циклов из разложения в циклы;
		8  метод, возвращающий обратную перестановку;
		9  метод, возвращающий k-ую степень перестановки (k - неотрицательное число) (возвращается перестановка);
		10 метод, возвращающий длину перестановки;
		11 метод, возвращающий порядок перестановки (возвращается неотрицательное число);
		12 метод, возвращающий знак перестановки.
	Перегрузить:
		1 оператор присваивания;
		2 оператор умножения для вычисления произведения двух перестановок (перегрузить с помощью дружественной функции);
		3 оператор присваивания.
*/

#include <iostream>

using namespace std;

class Permutation {
	int * arr, size;
  public:
	Permutation(int n);
	Permutation(int n, int * a);
	Permutation(const Permutation & p);
	~Permutation();

	void print() const;
	void break_down(int flag = 0) const;
	void lengths_of_cycles(int flag = 1) const;
	Permutation inverse() const;
	Permutation power(const int & k) const;
	int length() const;
	int order() const;
	bool sign() const;

	Permutation & operator=(const Permutation & p);
	Permutation operator*(const Permutation & p) const;
	bool operator==(const Permutation & p) const;
};

int main() {
	int n, pow;
	cout << "Size of first permutation: ";
	cin >> n;
	int * arr = new int[n];
	cout << "First permutation: ";
	for (int i = 0; i < n; i++)
		cin >> arr[i];
	Permutation p1(n, arr);
	cout << "p1 = ";
	p1.print();
	cout << "length of p1 = " << p1.length() << endl;
	cout << "break_down = ";
	p1.break_down();
	cout << "lengths_of_cycles = ";
	p1.lengths_of_cycles();
	cout << "inverse = ";
	(p1.inverse()).print();
	cout << "order of p1 = " << p1.order() << endl;
	cout << "to the power of ";
	cin >> pow;
	cout << "= ";
	(p1.power(pow)).print();
	if (p1.sign())
		cout << "p1 is even\n";
	else
		cout << "p1 is odd\n";
	int m;
	cout << "Size of second permutation: ";
	cin >> m;
	int *ar = new int[m];
	cout << "Second permutation: ";
	for (int i = 0; i < m; i++)
		cin >> ar[i];
	Permutation p2(m, ar);
	(p1 * p2).print();
	delete []arr;
	delete []ar;
	return 0;
}

Permutation::Permutation(int n) {
	size = n + 1;
	arr = new int[size];
	for (int i = 1; i <= n; i++)
		arr[i] = i;
}

Permutation::Permutation(int n, int * a) {
	size = n + 1;
	arr = new int[size];
	arr[0] = 0;
	for (int i = 1; i <= n; i++)
		arr[i] = a[i - 1];
}

Permutation::Permutation(const Permutation & p) {
	size = p.size;
	arr = new int[size];
	for (int i = 0; i < size; i++)
		arr[i] = p.arr[i];
}

Permutation::~Permutation() {
	delete []arr;
}

void Permutation::print() const {
	for (int i = 1; i < size; i++)
		cout << arr[i] << ' ';
	cout << endl;
}

void Permutation::break_down(int flag) const {
	int start, i, length;
	int used[size];
	for (i = 0; i < size; i++)
		used[i] = 0;
	for (start = 1; start < size; start++) {
		length = 1;
		if (used[start] == 1)
			continue;
		used[start] = 1;
		i = arr[start];
		if (flag == 0)
			cout << '(' << start << ' ';
		while (i < size && i != start) {
			used[i] = 1;
			if (flag == 0)
				cout << i << ' ';
			else
				length++;
			i = arr[i];
		}
		if (flag == 0)
			cout << "\b)";
		else
			cout << length << ' ';
	}
	cout << endl;
}

void Permutation::lengths_of_cycles(int flag) const {
	this -> break_down(flag);
}

Permutation Permutation::inverse() const {
	Permutation p(size - 1);
	for (int i = 0; i < size; i++)
		p.arr[arr[i]] = i;
	return p;
}

Permutation Permutation::power(const int & k) const {
	if (k == 0)
		return Permutation(size - 1);
	int l;
	Permutation p(size - 1);
	for (int i = 1; i < size; i++) {
		l = i;
		for (int j = 0; j < k; j++)
			l = arr[l];
		p.arr[i] = l;
	}
	return p;
}

int Permutation::length() const {
	return (size - 1);
}

int Permutation::order() const {
	Permutation p(size - 1), q(size - 1);
	q = *this;
	int i;
	for (i = 2; !(q == p); i++)
		q = q * (*this);
	return i - 1;
}

bool Permutation::sign() const {
	int s = 0;
	for (int i = 1; i < size; i++) {
		for (int j = i + 1; j < size; j++)
			if (arr[i] > arr[j])
				s++;
	}
	if (s % 2)
		return false;
	return true;
}

Permutation & Permutation::operator=(const Permutation & p) {
	size = p.size;
	arr = new int[size];
	for (int i = 0; i < size; i++)
		arr[i] = p.arr[i];
	return *this;
}

Permutation Permutation::operator*(const Permutation & p) const {
	Permutation q(size - 1);
	for (int i = 1; i < size; i++) {
		q.arr[i] = arr[p.arr[i]];
	}
	return q;
}

bool Permutation::operator==(const Permutation & p) const {
	int i;
	if (size != p.size)
		return 0;
	for(i = 1; i < size && arr[i] == p.arr[i]; i++);
	if (i == size)
		return 1;
	return 0;
}
