/*
					Задание 3. Классы Vector, Point, Rectangle
	1. Реализовать класс Vector, представляющий закрепленный вектор (начало вектора совпадает с началом координат) на плоскости. Класс должен иметь два поля х и у типа double, отвечающие за первую и вторую координаты вектора соответственно.
		Класс должен иметь три конструктора: конструктор по умолчанию (инициализирует нулевой вектор), конструктор с 2-мя параметрами копирования. В деструкторе необходимо выдавать сообщение.
		Реализовать методы:
		1. возвращающие значения полей;
		2. печатающий поля объекта;
		3. возвращающий длину вектора;
		4. нормирующий вектор.
		Перегрузить:
		1. бинарные операторы: +, - (для сложения и вычитания векторов);
		2. бинарный оператор * для умножения вектора на вещественное число;
		3. оператор присваивания;
		4. составные операторы: +=, -=, *= (аналоги операторов из пунктов 1 и 2);
		5. бинарный оператор * для скалярного произведения векторов;
		6. операторы сравнения;

	2. Реализовать класс Point, представляющий точку на плоскости. Класс должен иметь два поля рх и ру типа double, отвечающие за первую и вторую координаты точки соответственно.
		Класс должен иметь три конструктора: конструктор по умолчанию (инициализирует точку (0, 0)), конструктор с 2-мя параметрами и конструктор копирования. В деструкторе необходимо выдавать сообщение.
		Реализовать методы:
		1. возвращающие значения полей;
		2. печатающий поля объекта;
		3. возвращающий расстояние между двумя точками;
		Перегрузить:
		1. бинарные операторы +, - для сложения и вычитания точки с вектором;
		2. бинарный оператор - для вычитания точки от точки (результатом будет закрепленный вектор);
		3. бинарный оператор * для умножения точки на вещественное число (покоординатное умножение);
		4. оператор присваивания;
		5. составные операторы: +=, -=, *= (аналоги операторов из пунктов 1, 2, 3);
		6. бинарный оператор * для скалярного произведения векторов;
		7. операторы сравнения: ==, !=;
		8. операторы сравнения: <, <=, >, >= (порядок залать лексикографический, например, одна точка "больше" другой, если первая координата первой точки больше первой координаты второй точки, и при равенстве первых координат, если вторая координата первой точки больше второй координаты второй точки).
	3. Реализовать класс Rectangle, представляющий прямоугольник на плоскости, стороны которого параллельны осям координат. Класс должен иметь четыре поля: l - абсцисса левой стороны, b - ордината нижней стороны, w - ширина, h - высота.
		Класс должен иметь четыре конструктора:
		  а) конструктор по умолчанию (инициализирует вырожденный прямоугольник - точку (0, 0));
		  б) конструктор с 4 параметрами;
		  в) конструктор с тремя параметрами Rectangle(Point lb, double ww, double hh), где точка lb - вершина левого нижнего угла, а значения ww и hh - ширина и высота прямоугольника соответственно;
		  г) конструктор копирования.
		В деструкторе необходимо выдавать сообщение.
		Реализовать методы:
		1. возвращающие абсциссы левой и правой сторон;
		2. возвращающие ординаты нижней и праой сторон;
		3. возвращающие ширину и высоту;
		4. возвращающие вершины левого нижнего и правого верхнего углов (возвращаются точки);
		5. проверяющий принадлежность заданной точки прямоугольнику (bool-евый метод);
		6. осуществляющий параллельный перенос по заданному вектору;
		7. расширяющий прямоугольник (правая верхняя вершина параллельно переносится по заданному вектору);
		8. возвращающий прямоугольник, который находится в пересечении двух заданных прямоугольников (если прямоугольники не пересекаются, то возвращается вырожденный прямоугольник);
		9. возвращающий наименьший прямоугольник, который содержит два заданных прямоугольника.
	В методах вместо передачи объекта по значению использовать передачу по константной ссылке (т.е. ссылке на константу). Если в методе не предполагается изменение полей "своего" объекта, то метод должен быть объявлен константным.
*/

#include <iostream>
#include <cmath>

using namespace std;

class Vector {
  private:
	double x, y;
  public:
  	Vector();
	Vector(double x1, double y1);
	Vector(const Vector &v);
	~Vector();
	double get_x() const;
	double get_y() const;
	void show() const;
	double length() const;
	void normalize();
	Vector operator+(const Vector &v) const;
	Vector operator-(const Vector &v) const;
	Vector operator*(const double &alpha) const;
	Vector & operator=(const Vector &v);
	Vector & operator+=(const Vector &v);
	Vector & operator-=(const Vector &v);
	Vector & operator*=(const double &alpha);
	double operator*(const Vector &v) const;
	int operator==(const Vector &v) const;
	int operator!=(const Vector &v) const;
};

Vector::Vector() {
	x = 0;
	y = 0;
}
Vector::Vector(double x1, double y1) {
	x = x1;
	y = y1;
}
Vector::Vector(const Vector &v) {
	x = v.x;
	y = v.y;
}
Vector::~Vector() {
	cout << "Destructor for Vector called\n";
}
double Vector::get_x() const {
	return x;
}
double Vector::get_y() const {
	return y;
}
void Vector::show() const {
	cout << '(' << x << ", " << y << ")\n"; 
}
double Vector::length() const {
	return sqrt(x * x + y * y);
}
void Vector::normalize() {
	double l = this -> length();
	x /= l;
	y /= l;
}
Vector Vector::operator+(const Vector &v) const {
	return Vector(x + v.x, y + v.y);
}
Vector Vector::operator-(const Vector &v) const {
	return Vector(x - v.x, y - v.y);
}
Vector Vector::operator*(const double &alpha) const {
	return Vector(x * alpha, y * alpha);
}
Vector & Vector::operator=(const Vector &v) {
	x = v.x;
	y = v.y;
	return *this;
}
Vector & Vector::operator+=(const Vector &v) {
	x += v.x;
	y += v.y;
	return *this;
}
Vector & Vector::operator-=(const Vector &v) {
	x -= v.x;
	y -= v.y;
	return *this;
}
Vector & Vector::operator*=(const double &alpha) {
	x *= alpha;
	y *= alpha;
	return *this;
}
double Vector::operator*(const Vector &v) const {
	return (x * v.x + y * v.y);
}
int Vector::operator==(const Vector &v) const {
	if (x == v.x && y == v.y)
		return 1;
	return 0;
}
int Vector::operator!=(const Vector &v) const {
	if (x == v.x && y == v.y)
		return 0;
	return 1;
}

class Point {
  private:
  	double px, py;
  public:
	Point();
	Point(double x, double y);
	Point(const Point &p);
	~Point();
	double get_px() const;
	double get_py() const;
	void show() const;
	double distance(const Point &p) const;
	Point operator+(const Vector &v) const;
	Point operator-(const Vector &v) const;
	Vector operator-(const Point &p) const;
	Point operator*(const double &alpha) const;
	Point & operator=(const Point &p);
	Point & operator+=(const Vector &v);
	Point & operator-=(const Vector &v);
	Point & operator *=(const double &alpha);
	int operator==(const Point &p) const;
	int operator!=(const Point &p) const;
	int operator<(const Point &p) const;
	int operator<=(const Point &p) const;
	int operator>(const Point &p) const;
	int operator>=(const Point &p) const;
};

Point::Point() {
	px = 0;
	py = 0;
}
Point::Point(double x, double y) {
	px = x;
	py = y;
}
Point::Point(const Point &p) {
	px = p.px;
	py = p.py;
}
Point::~Point() {
	cout << "Destructor for Point called\n";
}
double Point::get_px() const {
	return px;
}
double Point::get_py() const {
	return py;
}
void Point::show() const {
	cout << '(' << px << ", " << py << ")\n";
}
double Point::distance(const Point &p) const {
	double x = p.px - px;
	double y = p.py - py;
	return sqrt(x * x + y * y);
}
Point Point::operator+(const Vector &v) const {
	return Point(px + v.get_x(), py + v.get_y());
}
Point Point::operator-(const Vector &v) const {
	return Point(px - v.get_x(), py - v.get_y());
}
Vector Point::operator-(const Point &p) const {
	return Vector(p.px - px, p.py - py);
}
Point Point::operator*(const double &alpha) const {
	return Point(px * alpha, py * alpha);
}
Point & Point::operator=(const Point &p) {
	px = p.px;
	py = p.py;
	return *this;
}
Point & Point::operator+=(const Vector &v) {
	px += v.get_x();
	py += v.get_y();
	return *this;
}
Point & Point::operator-=(const Vector &v) {
	px -= v.get_x();
	py -= v.get_y();
	return *this;
}
Point & Point::operator*=(const double &alpha) {
	px *= alpha;
	py *= alpha;
	return *this;
}
//
int Point::operator==(const Point &p) const {
	if (px == p.px && py == p.py)
		return 1;
	return 0;
}
int Point::operator!=(const Point &p) const {
	if (px == p.px && py == p.py)
		return 0;
	return 1;
}
int Point::operator<(const Point &p) const {
	if (px < p.px)
		return 1;
	if (px == p.px && py < p.py)
		return 1;
	return 0;
}
int Point::operator<=(const Point &p) const {
	if (px <= p.px)
		return 1;
	if (px == p.px && py <= p.py)
		return 1;
	return 0;
}
int Point::operator>(const Point &p) const {
	if (px <= p.px)
		return 0;
	if (px == p.px && py <= p.py)
		return 0;
	return 1;
}
int Point::operator>=(const Point &p) const {
	if (px < p.px)
		return 0;
	if (px == p.px && py < p.py)
		return 0;
	return 1;
}

class Rectangle {
  private:
	double l, b, w, h;
  public:
  	Rectangle();
	Rectangle(double l1, double b1, double w1, double h1);
	Rectangle(Point lb, double ww, double hh);
	Rectangle(const Rectangle &r);
	~Rectangle();
	void show() const;
	double get_l() const;
	double get_r() const;
	double get_b() const;
	double get_t() const;
	double get_w() const;
	double get_h() const;
	Point get_lb() const;
	Point get_rt() const;
	bool is_inside(const Point &p) const;
	Rectangle move(const Vector &v) const;
	Rectangle expand(const Vector &v) const;
	Rectangle intersection(const Rectangle &r) const;
	Rectangle consists(const Rectangle &r) const;
};

Rectangle::Rectangle() {
	l = 0;
	b = 0;
	w = 0;
	h = 0;
}
Rectangle::Rectangle(double l1, double b1, double w1, double h1) {
	l = l1;
	b = b1;
	w = w1;
	h = h1;
}
Rectangle::Rectangle(Point lb, double ww, double hh) {
	l = lb.get_px();
	b = lb.get_py();
	w = ww;
	h = hh;
}
Rectangle::Rectangle(const Rectangle &r) {
	l = r.l;
	b = r.b;
	w = r.b;
	h = r.h;
}
Rectangle::~Rectangle() {
	cout << "Destructor for Rectangle called\n";
}
void Rectangle::show() const {
	cout << '(' << l << ", " << b << ", " <<  w << ", " << h << ")\n";
}
double Rectangle::get_l() const {
	return l;
}
double Rectangle::get_r() const {
	return l + w;
}
double Rectangle::get_b() const {
	return b;
}
double Rectangle::get_t() const {
	return b + h;
}
double Rectangle::get_w() const {
	return w;
}
double Rectangle::get_h() const {
	return h;
}
Point Rectangle::get_lb() const {
	return Point(l, b);
}
Point Rectangle::get_rt() const {
	return Point(l + w, b + h);
}
bool Rectangle::is_inside(const Point &p) const {
	if (l <= p.get_px() && p.get_px() <= l + w && b <= p.get_py() && p.get_py() <= b + h)
		return true;
	return false;
}
Rectangle Rectangle::move(const Vector &v) const {
	return Rectangle(l + v.get_x(), b + v.get_y(), w, h);
}
Rectangle Rectangle::expand(const Vector &v) const {
	return Rectangle(l, b, w + v.get_x(), h + v.get_y());
}
Rectangle Rectangle::intersection(const Rectangle &r) const {
	if (b + h < r.b || r.b + r.h < b || l + w < r.l || r.l + r.w < l)
		return Rectangle();
	double left, bottom, width, height;
	(l >= r.l) ? left = l : left = r.l;
	(b >= r.b) ? bottom = b : bottom = r.b;
	(l + w <= r.l + r.w) ? width = l + w - left : width = r.w + r.l - left;
	(b + h <= r.b + r.h) ? height = b + h - bottom : height = r.b + r.h - bottom;
	return Rectangle(left, bottom, width, height);
}
Rectangle Rectangle::consists(const Rectangle &r) const {
	double left, bottom, width, height;
	(l <= r.l) ? left = l : left = r.l;
	(b <= r.b) ? bottom = b : bottom = r.b;
	(l + w >= r.l + r.w) ? width = l + w - left : width = r.l + r.w - left;
	(b + h >= r.b + r.h) ? height = b + h - bottom : height = r.b + r.h - bottom;
	return Rectangle(left, bottom, width, height);
}

int main() {
//	Vector Av, Bv, Cv;
//	Point Ap, Bp, Cp;
	Rectangle Ar(0, 0, 2, 2), Br(2, 2, 2, 2);
	(Br.consists(Ar)).show();
	(Br.intersection(Ar)).show();
	return 0;
}
