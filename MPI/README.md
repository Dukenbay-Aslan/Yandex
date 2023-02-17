# Численное решение краевой задачи
Методом конечных разностей приближенно решается краевая задача для уравнения Пуассона с потенциалом в прямоугольной области.
### Математическая постановка задачи
Задан прямоугольник $\Pi = [0, 4] \times [0,3]$. В нем рассматривается дифференциальное уравнение Пуассона с потенциалом
$$-\Delta u + q(x, y)u = F(x, y),$$ где оператор Лапласа
$$\Delta u = \frac{\partial}{\partial x} \left( k(x, y) \frac{\partial u}{\partial x} \right) + \frac{\partial}{\partial y} \left( k(x, y) \frac{\partial u}{\partial y} \right).$$
Заданы функции $$u(x, y) = \sqrt{4 + xy}, \quad k(x, y) = 4 + x + y, \quad q(x, y) = x + y.$$
В расчетной области $\Pi$ определяется равномерная прямоугольная сетка $\overline{\omega}_h = \overline{\omega}_1 \times \overline{\omega}_2$, где
$$\overline{\omega}_1 = \{ x_i = ih_1, i = \overline{0, M} \},\quad \overline{\omega}_2 = \{ y_j = jh_2, j = \overline{0, N} \},\quad h_1 = \frac4M, \quad h_2 = \frac3N.$$
### Результаты запусков
Задача со следующими входными данными решалась на сервере [IBM Polus на базе ВМК МГУ](http://hpc.cmc.msu.ru/polus)
| Число MPI-процессов | $M \times N$ | Ускорение |
| --- | --- | --- |
|<p> 4 <p> 8 <p> 16 <p> 32 | 500 $\times$ 500 |<p> 1 <p> 1.991 <p> 3.937 <p> 7.706|
|<p> 4 <p> 8 <p> 16 <p> 32 | 500 $\times$ 1000 |<p> 1 <p> 1.998 <p> 4.022 <p> 7.927|
