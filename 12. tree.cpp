/*
							Задание 12. Класс Tree
	Реализовать шаблонный класс Tree (дерево поиска) со следующим интерфейсом:
*********************************************************************************************************************************
template <class Type>
class Tree {
	struct Node {
		Type key;
		Node * left;
		Node * right;
		Node * parent;
	};
  private:
	Node * root;
  public:
	Tree();
	~Tree();

	Print();
	Node * Find(const Type & val);
	Node * Min();
	Node * Max();
	Node * Successor(Node * x);
	Node * Predecessor(Node * x);
	void Insert(const Type & val);
	void Delete(Node * x);
	void Print_Node(Node * x);
};
************************************************************the end**************************************************************

	1 root указывает на тот элемент, поле parent которого равно NULL;
	2 Конструктор должен создавать пустое дерево;
	3 Деструктор должен удалять все элементы дерева;
	4 Метод Print печатает содержимое дерева, при этом используется алгоритм симметричного обхода TREE_WALK (рекурсивный вариант);
	5 Метод Find находит вершину, поле key которой совпадает с val и в случае успеха возвращает указатель на вершину, в случае неудачи возвращается нулевой указатель;
	6 Метод Min (Max) находит вершину с наименьшим (наибольшим) полем key и возвращает указатель на него;
	7 Метод Successor (Predecessor) находит для заданной вершины следующую (предыдущую) вершину и возвращает указатель на найденную вершину в случае успеха. В случае неудачи возвращается нулевой указатель. Порядок задается порядком возврастания полей key;
	8 Метод Insert добавляет новую вершину с полем key равной val;
	9 Метод Delete удаляет заданную вершину при возможности удаления;
	10 Метод Print_Node печатает поле key заданной вершины.

							Задания по деревьям поиска

	1. Реализуйте бинарное дерево поиска для целых чисел. Программа получает на вход последовательность целых чисел и строит из них дерево. Элементы в дерево добавляются в соответствии с результатом поиска их места. Если элемент уже существует в дереве, добавлять его не надо. Балансировка дерева не производится. При необходимости можно добавлять новые методы в класс Tree!
	Входные данные: на вход программа получает последовательность натуральных чисел. Последовательность завершается числом 0, которое означает конец вывода, и добавлять его в дерево не надо.
	Выходные данные: выведите единственное число - высоту получившегося дерева.
	Пример соответствует следующему дереву:
								 7
								/ \
							       /   \
							      /     \
							     /	     \
						            3         9
							   / \       /
							  /   \     /
							 2     5   8
							/     / \
						       1     4   6
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								4

	2. Подсчитайте количество элементов в получившемся дереве и выведите это количество.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит.
	Выходные данные: выведите ответ на задачу.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								9

	3. Выведите второй по величине элемент в построенном дереве. Гарантируется, что такой найдется.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит.
	Выходные данные: выведите ответ на задачу.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								8

	4. Для полученного жерева выведите список всех листьев (вершин, не имеющих потомков) в порядке возрастания.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит.
	Выходные данные: выведите ответ на задачу.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								1
											4
											6
											8

	5. Для полученного дерева выведите список всех вершин, имеющих по два ребенка, в порядке возрастания.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит.
	Выходные данные: выведите ответ на задачу.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								3
											5
											7

	6. Для полученного дерева выведите список всех вершин, имеющих только одного ребенка, в порядка возрастания.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит.
	Выходные данные: выведите ответ на задачу.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								2
											9

	7. Дерево назывется сбалансированным, если для любой его вершины высота левого и правого поддерева для этой вершины различаются не более чем на 1.
	Входные данные: вводится последовательность целых чисел, оканчивающаяся нулем. Сам ноль в последовательность не входит. Постройте дерево, соответствующее данной последовательности.
	Выходные данные: определите, является ли дерево сбалансированным, выведите слово YES или NO.
	Примеры
	Входные данные									Выходные данные
	7 3 2 1 9 5 4 6 8 0								YES
*/

#include <iostream>

using namespace std;

template <class Type>
class Tree {
	struct Node {
		Type key;
		Node * left;
		Node * right;
		Node * parent;
	};
  private:
  	Node * root;
  public:
  	Tree() {
		root = NULL;
	}

	~Tree() {
		DESTRUCTOR(root);
	}

	void Print() {
		TREE_WALK(root);
		cout << endl;
	}

	Node * Find(Node * x, const Type & val) {
		if (x == NULL || x -> key == val)
			return x;
		if (x -> key < val)
			return Find(x -> right, val);
		return Find(x -> left, val);
	}

	Node * Min(Node * x) {
		while (x -> left != NULL)
			x = x -> left;
		return x;
	}

	Node * Max(Node * x) {
		while (x -> right != NULL)
			x = x -> right;
		return x;
	}

	Node * Successor(const Type & val) {
		Node * x = new Node;
		x = Find(root, val);
		if (x == NULL)
			return NULL;
		if (x -> right != NULL)
			return Min(x -> right);
		Node * y = new Node;
		y = x -> parent;
		while (y != NULL && x == y -> right) {
			x = y;
			y = y -> parent;
		}
		return y;
	}

	Node * Predecessor(const Type & val) {
		Node * x = new Node;
		x = Find(root, val);
		if (x == NULL)
			return NULL;
		if (x -> left != NULL)
			return Max(x -> left);
		Node * y = new Node;
		y = x -> parent;
		while (y != NULL && x == y -> left) {
			x = y;
			y = y -> parent;
		}
		return y;
	}

	void Insert(const Type & val) {
		Node * z = new Node;
		z -> right = NULL;
		z -> left = NULL;
		z -> parent = NULL;
		z -> key = val;
		Node * x = new Node;
		x = root;
		Node * y = NULL;
		while (x != NULL) {
			y = x;
			if (val < x -> key)
				x = x -> left;
			else
				x = x -> right;
		}
		z -> parent = y;
		if (y == NULL)
			root = z;
		else if (val <= y -> key)
			y -> left = z;
		else
			y -> right = z;
	}

	void Delete(const Type & val) {
		Node * x = new Node;
		x = Find(root, val);
		if (x == NULL) {
			cout << "There is no " << val << " in tree\n";
			return;
		}
		if (x -> left == NULL && x -> right == NULL) {
			TREE_DELETE0(x);
			return;
		}
		if ((x -> left == NULL && x -> right != NULL) || (x -> left != NULL && x -> right == NULL)) {
			TREE_DELETE1(x);
			return;
		}
		TREE_DELETE2(x);
		return;
	}

	void Print_Node(Node * x) {
		cout << x -> key << endl;
	}

	Node * get_root() {
		return root;
	}

	friend void DESTRUCTOR(Node * x) {
		if (x != NULL) {
			DESTRUCTOR(x -> left);
			DESTRUCTOR(x -> right);
			delete x;
		}
	}

	friend void TREE_WALK(Node * x) {
		if (x != NULL) {
			TREE_WALK(x -> left);
			cout << x -> key << ' ';
			TREE_WALK(x -> right);
		}
	}

	void TREE_DELETE0(Node * x) {
		if (root == x) {
			delete x;
			root = NULL;
		}
		else {
			if (x -> parent -> left == x) {
				x -> parent -> left = NULL;
			}
			else
				x -> parent -> right = NULL;
			delete x;
		}
	}

	void TREE_DELETE1(Node * x) {
		if (x -> left == NULL) {
			if (x -> parent == NULL) {
				root = x -> right;
				x -> right -> parent = NULL;
			}
			else {
				if (x -> parent -> left == x)
					x -> parent -> left = x -> right;
				else
					x -> parent -> right = x -> right;
				x -> right -> parent = x -> parent;
			}
		}
		else {
			if (x -> parent == NULL) {
				root = x -> left;
				x -> left -> parent = NULL;
			}
			else {
				if (x -> parent -> left == x)
					x -> parent -> left = x -> left;
				else
					x -> parent -> right = x -> right;
			}
		}
	}

	void TREE_DELETE2(Node * x) { // не нужно передавать Tree<Type> T
		Node * y = new Node;
		y = Successor(x -> key);
		if (y == NULL)
			return;
		x -> key = y -> key;
		if (y -> right == NULL)
			TREE_DELETE0(y);
		else
			TREE_DELETE1(y);
	}
};

int main() {
	Tree<int> t;
	int x;
	cout << "Tree elements (while not 0): ";
	cin >> x;
	while (x != 0) {
		t.Insert(x);
		cin >> x;
	}
	cout << "Tree: ";
	t.Print();
	cout << "Find number in Tree: ";
	cin >> x;
	if (t.Find(t.get_root(), x) != NULL)
		cout << "Yes\n";
	else
		cout << "No\n";
	int n;
	cout << "Number of numbers to delete: ";
	cin >> n;
	cout << "Numbers to delete: ";
	for (int i = 0; i < n; i++) {
		cin >> x;
		t.Delete(x);
	}
	t.Print();
	cout << "Minimum element is ";
	cout << t.Min(t.get_root()) -> key << endl;
	cout << "Maximum element is ";
	cout << t.Max(t.get_root()) -> key << endl;
	cout << "Successor for ";
	cin >> x;
	if (t.Successor(x) == NULL && t.Find(t.get_root(), x) != NULL)
		cout << "Null (" << x << " is maximum element)\n";
	else if (t.Find(t.get_root(), x) == NULL)
		cout << "There is no " << x << " in tree\n";
	else
		cout << t.Successor(x) -> key << endl;
	cout << "Predecessor for ";
	cin >> x;
	if (t.Predecessor(x) == NULL && t.Find(t.get_root(), x) != NULL)
		cout << "Null (" << x << " is minimum element)\n";
	else if (t.Find(t.get_root(), x) == NULL)
		cout << "There is no " << x << " in tree\n";
	else
		cout << t.Predecessor(x) -> key << endl;
	return 0;
}
