#include <iterator>
#include <algorithm>
#include <stack>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "SCREEN.h"
#include <vector>
#include <ctime>
#include <chrono>

using namespace std;

const int lim = 1000;

struct Node {  //Узел дерева
    int key;         //Вес
    Node* left, * right; //Левое и правое поддерево
    int b;            //
    vector<int> serials;
    bool next;
    Node(int k, Node* l = nullptr, Node* r = nullptr) : key(k), b(0), left(l), right(r), next(false) { }
    ~Node() { delete left; delete right; }
    void Display(int, int, int);             //Вывод узла/ключа в буфер вывода
    Node(const Node&) = delete;   //Конструктор копирования для узла
};

int setval(string& s, int pos, int val) {
    string t(to_string(val));
    if (pos + t.size() - 1 <=  MAXCON)
        for (auto p : t) s[pos++] = p;
    return t.size();
}

void Node::Display(int row, int col, int depth) {
    if ((row >= MAXROW) || (col < 0) || (col >= MAXCON)) { SCREEN[MAXROW - 1][col] = '+'; return; };
    int ofl = offset[depth + 1];
    setval(SCREEN[row], col, key);
    if (b == 1) { row--; ofl = 2; b = 0; }
    if (left) left->Display(row + 2, col - ofl + 1, depth + 1);
    if (right)
        if (!next) { right->Display(row + 2, col + offset[depth + 1] - 1, depth + 1); }
        else {
            right->b = 1;
            right->Display(row + 1, col + 1, depth);
        }
}


using MyStack = stack<pair<Node*, int>>; //Тип стека для обхода дерева
//ИТЕРАТОР ЧТЕНИЯ
//template<class Container = Set> — настройка на контейнер не требуется
struct myiter : public std::iterator<std::forward_iterator_tag, int>
{
    Node* Ptr;   //Реальный указатель
    MyStack St;  //Стек с путём от корня дерева
    myiter(Node* p = nullptr) : Ptr(p) { }
    myiter(Node* p, const MyStack&& St) : Ptr(p), St(move(St)) { }
    bool operator == (const myiter& Other) const { return Ptr == Other.Ptr; }
    bool operator != (const myiter& Other) const { return !(*this == Other); }
    myiter& operator++();
    myiter operator++(int) { myiter temp(*this); ++* this; return temp; }
    pointer operator->() { return &Ptr->key; }
    reference operator*() { return Ptr->key; }
};

//ИТЕРАТОР ВСТАВКИ (универсальный)
template <typename Container, typename Iter = myiter>
class outiter : public std::iterator<std::output_iterator_tag, typename Container::value_type>
{
protected:
    Container& container;    // Контейнер для вставки элементов
    Iter iter;                           // Текущее значение итератора чтения
public:
    outiter(Container& c, Iter it) : container(c), iter(it) { } // Конструктор
    const outiter<Container>& // Присваивание — вставка ключа в контейнер
        operator = (const typename Container::value_type& value) {
        iter = container.insert(value, iter).first;
        return *this;
    }
    const outiter<Container>& //Присваивание копии — фиктивное
        operator = (const outiter<Container>&) { return *this; }
    outiter<Container>& operator* () { return *this; } // Разыменование — пустая операция
    outiter<Container>& operator++ () { return *this; } // Инкремент — пустая операция
    outiter<Container>& operator++ (int) { return *this; }
};

// Функция для создания итератора вставки
template <typename Container, typename Iter>
inline outiter<Container, Iter> inserter(Container& c, Iter it)
{
    return outiter<Container, Iter>(c, it);
}

class Tree {     //Контейнер — ДДП
    const char tag;
    Node* root;
    size_t h, count; //Высота и мощность дерева
    int subCount;
public:          //Стандартные элементы контейнера
    using key_type = int;
    using value_type = int;
    using key_compare = less<int>;
    void swap(Tree& rgt)  //Обмен содержимого двух ДДП
    {
        std::swap(root, rgt.root);
        std::swap(count, rgt.count);      std::swap(h, rgt.h);
        std::swap(subCount, rgt.subCount);
    };

    int Power() { return subCount; }
    //Стандартная функция вставки (оболочка)
   // myiter insert(const myiter& where, const int& k);
    //myiter insert(const int& k, const myiter& where = myiter(nullptr))
    //{
    //    return insert(where, k);
    //}
    void Display();
    void ShowSet();
    vector<int> getSub();
    void ShowSub();
    myiter begin()const;
    myiter end()const { return myiter(nullptr); }
    pair<myiter, bool> insert(int, myiter = myiter(nullptr));
    void CONCAT(Tree&);
    void SUBST(Tree&, int);
    void CHANGE(Tree&, int);

    void upSerials(int k) {
        myiter x = begin();
        while (x.Ptr != nullptr)
        {
            for (int i = 0; i < x.Ptr->serials.size(); i++)
                if (x.Ptr->serials[i] > k) x.Ptr->serials[i]--;
            x++;
        }
    }
    pair<myiter, bool> erase(int k) {
        stack <pair<Node*, int>> St;
        pair<myiter, bool> answer;
        Node* p(nullptr), * q(root), *r(nullptr);
        int a(0);
        bool cont = (q != nullptr);
        while (cont && (k != q->key))
        {
            St.push(make_pair(p, a));
            p = q;
            a = k > q->key;
            q = a == 1 ? q->right : q->left;
            if (q == nullptr) { cont = false; }
        }
        if (cont) {
            int c = q->serials.back();
            
            q->serials.pop_back();
            
            if (q->serials.empty()) {
                --count;
                if (r = q->right) {
                    if (r->left) {
                        St.push(make_pair(p, a));
                        p = q;
                        a = 1;
                        do {
                            St.push(make_pair(p, a));
                            p = r;
                            a = 0;
                            r = a == 1 ? r->right : r->left;
                        } while (r->left);
                        q->key = r->key;
                        p->left = r->right;
                        q = r;
                    }
                    else {
                        r->left = q->left;
                        if (p)
                            if (a == 1) p->right = r;
                            else p->left = r;
                        else root = r;
                        St.pop();
                        p = r;
                        a = 1;
                    }
                }
                else {
                    if (p)
                        if (a == 1) p->right = q->left;
                        else p->left = q->left;
                    else root = q->left;
                }
            }
            if (p->next && p->right == q) { p->next == false; }
            if (q) { q->left = q->right = nullptr; }
            subCount--;
        }
        if (cont && q->serials.empty()) delete q;
        answer = make_pair(myiter(q), cont);
        return answer;
    };
    Tree() : tag('R'), root(nullptr), h(0), count(0) { }
    int size() { return count; }
    template<typename MyIt>   // Формирование ДДП из отрезка
    Tree(MyIt, MyIt);
    Tree(int pow, char t = 'R') : tag(t)  {
        for (int i = 0; i < pow; ++i)
        {
            insert(rand()%lim);
        }
    }
    Tree(vector<int> ini, const char t = ' ') : tag(t) {
        subCount = 0;
        for (auto x : ini)
        {
            insert(x);
        }
    };  // ДДП из списка инициализации
    ~Tree() { delete root; }
    myiter find(int k) const {
        Node* temp{ root };
        while (temp)
        {
            if (temp->key == k) return myiter(temp);
            else temp = (temp->key > k) ? temp->left : temp->right;
        }
        return myiter(nullptr);
    }; // Поиск по ключу
    Tree(const Tree& rgt) : Tree()
    {
        vector<int> Sub;
        subCount = 0;
        Sub.resize(rgt.subCount);
        for (myiter x = rgt.begin(); x != nullptr; x++)
        {
            for (auto i : x.Ptr->serials)
                Sub[i] = x.Ptr->key;
        }
        for (auto i : Sub) insert(i);
    }
    Tree(Tree&& rgt) : Tree() { swap(rgt); }
    /*Tree& operator = (const Tree& rgt)
    {
        Tree temp;
        for (auto x : rgt) temp.insert(x);
        swap(temp);
        return *this;
    }*/

    //Двуместные операции над множествами
    Tree& operator = (Tree&& rgt) { swap(rgt); return *this; }
    Tree& operator |= (const Tree&);
    Tree operator | (const Tree& rgt) const
    {
        Tree result(*this); return (result |= rgt);
    }
    Tree& operator &= (const Tree&);
    Tree operator & (const Tree& rgt) const
    {
        Tree result(*this); return (result &= rgt);
    }
    Tree& operator -= (const Tree&);
    Tree operator - (const Tree& rgt) const
    {
        Tree result(*this); return (result -= rgt);
    }
    Tree& operator ^= (const Tree&);
    Tree operator ^ (const Tree& rgt) const
    {
        Tree result(*this); return (result ^= rgt);
    }
};



Tree& Tree::operator |= (const Tree& rgt) {
    Tree temp;
    set_union(begin(), end(), rgt.begin(), rgt.end(), inserter(temp, myiter(nullptr)));
    swap(temp);
    return *this;
}

Tree& Tree::operator &= (const Tree& rgt) {
    Tree temp;
    set_intersection(begin(), end(), rgt.begin(), rgt.end(), inserter(temp, myiter(nullptr)));
    swap(temp);
    return *this;
}

Tree& Tree::operator -= (const Tree& rgt) {
    Tree temp;
    set_difference(begin(), end(), rgt.begin(), rgt.end(), inserter(temp, myiter(nullptr)));
    swap(temp);
    return *this;
}

Tree& Tree::operator ^= (const Tree& rgt) {
    Tree temp;
    set_symmetric_difference(begin(), end(), rgt.begin(), rgt.end(), inserter(temp, myiter(nullptr)));
    swap(temp);
    return *this;
}

void Tree::Display() {
    std::cout << "1-2 Tree named " << tag <<" (n=" << count << " h=" << h << ")\n";
    screen_init();
    root->Display(0, offset[0], 0);
    screen_refresh();
}


pair<myiter, bool> Tree::insert(int k, myiter where)
{
    Node* t{ root };
    bool cont{ true }, up{ false };
    stack<pair<Node*, int>> St;
    if (!where.Ptr) { //Вставка в пустое дерево или свободная
   //===== Инициализация =====
        if (t == nullptr) {      // Дерево пусто
            root = new Node(k);
            count = h = 1;
            St.push(make_pair(root, 1));   // Инициализация стека…
            root->serials.push_back(subCount);
            subCount++;
            return make_pair(myiter(root, move(St)), true); // и выход
        }
        else St.push(make_pair(root, 1));
        // Инициализация стека: корень; случай 1
    }
    else {  //Начать с места предыдущей вставки
        t = where.Ptr;
        St = move(where.St); // Взять стек из итератора
    }
    while (cont) { //Поиск по дереву
        if (k == t->key)    // Выход «вставка не понадобилась»
        {
            t->serials.push_back(subCount);
            subCount++;
            return make_pair(myiter(t, move(St)), false);
        }
        if (k < t->key) {
            if (t->left) { //Идём влево
                St.push(make_pair(t, 2)); //опускаем указатель; случай 2
                t = t->left;
            }
            else { //Вставка левого
                t->left = new Node(k, nullptr, t->left);
                t->left->serials.push_back(subCount);
                subCount++;
                cont = false;
            }
        }
        else if (!t->right) { //Вставка второго справа
            t->right = new Node(k);
            t->right->serials.push_back(subCount);
            subCount++;
            t->next = true;     //*************************************
            cont = false;
        }
        else if (t->next) {  //Группа из двух
            if (k == t->right->key) { // И при этом элемент равен второму
                t->right->serials.push_back(subCount);
                subCount++;
                return make_pair(myiter(t->right, move(St)), false); // То Вставка не нужна
            }
            else if (k < t->right->key) { //Меньше правого
                if (t->right->left) { //Есть путь вниз
                    St.push(make_pair(t, 3)); // — случай 3
                    t = t->right->left;
                }
                else { //Вставка среднего
                    t->right->left = new Node(k, nullptr, t->right->left);
                    t->right->left->serials.push_back(subCount);
                    subCount++;
                    cont = false;
                }
            }
            else {
                if (t->right->right) { //Есть путь вниз
                    St.push(make_pair(t, 4)); // — случай 4
                    t = t->right->right;
                }
                else { //Вставка третьего
                    t->right->right = new Node(k);
                    t->right->right->serials.push_back(subCount);
                    subCount++;
                    up = t->right->next = true; //true, стало три //*********
                    cont = false;
                }
            }
        }
        else if (t->right) {      //Есть правый ниже
            St.push(make_pair(t, 3));   // — случай 3
            t = t->right;
        }
        else {   //Вставка второго, образование группы
            t->right = new Node(k);
            t->right->serials.push_back(subCount);
            subCount++;
            t->next = true;     //************************************
            cont = false;
        }
        while (up) {   //Группа из трёх: передача второго на уровень вверх
            std::swap(t->key, t->right->key);
            std::swap(t->serials, t->right->serials);
            Node* t1{ t->right };
            t->next = t1->next = false;
            t->right = t->right->right;
            t1->right = t1->left;
            t1->left = t->left;
            t->left = t1;
            t1 = t;
            t = St.top().first;
            switch (St.top().second) {
            case 1:   //Дошли до корня — дерево подросло, конец
                ++h; //Счёт высоты
                up = false;
                break;
            case 2:   //Вставка левого
                std::swap(t->key, t1->key);
                std::swap(t->serials, t1->serials);
                t->left = t1->left;
                t1->left = t1->right;
                t1->right = t->right;
                t->right = t1;
                up = t1->next = t->next;   //Продолжать, если стало три
                break;
            case 3:
                if (t->next) {   //Вставка среднего, продолжать
                    t->right->left = t1->right;
                    t1->right = t->right;
                    t->right = t1;
                    t1->next = true;
                }
                else {   //Вставка второго справа, конец
                    t->next = true;
                    up = t1->next = false;
                    St.pop();
                }
                break;
            case 4:      //Вставка  — присоединение к группе третьим, продолжать
                t->right->next = true;
                t1->next = false;
            }
            if (up) St.pop();
        }   //while(up)
    }   //while(cont)
    ++count; //Счёт мощности
    return make_pair(myiter(t, move(St)), true);
};

myiter Tree::begin()const { //Поиск первого элемента множества
    MyStack St;
    Node* p(root);
    if (p) {   //Идём по левой ветви, запоминая путь в стеке
        while (p->left) {
            St.push(make_pair(p, 0));
            p = p->left;
        }
    }
    return myiter(p, move(St)); //Создаём итератор, передаём ему стек
}

void Tree::CONCAT(Tree& rgt) {
    vector<int> sub1, sub2;
    sub1 = getSub();
    sub2 = rgt.getSub();
    sub1.insert(sub1.end(), sub2.begin(), sub2.end());
    Tree temp(sub1);
    swap(temp);
}

void Tree::SUBST(Tree& rgt, int pos) {
    vector<int> sub1, sub2;
    sub1 = getSub();
    sub2 = rgt.getSub();
    if (pos > sub1.size())
        sub1.insert(sub1.end(), sub2.begin(), sub2.end());
    else sub1.insert(sub1.begin() + pos, sub2.begin(), sub2.end());
    Tree temp(sub1);
    swap(temp);
}

void Tree::CHANGE(Tree& rgt, int pos) {
    vector<int> sub1, sub2, stemp;
    sub1 = getSub();
    sub2 = rgt.getSub();
    if (pos > sub1.size()) sub1.insert(sub1.end(), sub2.begin(), sub2.end());
    else {
        stemp.insert(stemp.end(), sub1.begin(), sub1.begin() + pos);
        stemp.insert(stemp.end(), sub2.begin(), sub2.end());
        if (pos + sub2.size() < sub1.size())
            stemp.insert(stemp.end(), sub1.begin() + pos + sub2.size(), sub1.end());
        sub1 = stemp;
        stemp.clear();
    }
    Tree temp(sub1);
    swap(temp);
}

void Tree::ShowSet() {
    Tree temp(*this);
    cout << tag << " as a Set: {";
    for (auto x : temp) std::cout << x << " ";
    std::cout << "}\n";
}

vector<int> Tree::getSub() {
    vector<int> Sub;
    Sub.resize(subCount);
    for (myiter x = begin(); x != nullptr; x++)
    {
        for (auto i : x.Ptr->serials) 
            Sub[i] = x.Ptr->key;
    }
    return Sub;
}

void Tree::ShowSub() {
    vector <int> a = getSub();
    std::cout << tag <<  " as a Subsequence: <";
    for (auto x : a)  std::cout << x << " ";
    std::cout << ">\n";
}

//Инкремент = шаг по дереву, внутренний обход
myiter& myiter::operator++()
{
    if (!Ptr) { //Первое обращение?
        return *this; //Не работает без предварительной установки на дерево
    }
    if (Ptr->right) {    // Шаг вправо
        St.push(make_pair(Ptr, 1));
        Ptr = Ptr->right;
        while (Ptr->left) { //... и искать крайний левый
            St.push(make_pair(Ptr, 0));
            Ptr = Ptr->left;
        }
    }
    else {       // Подъём вверх, пока слева пусто
        pair<Node*, int> pp(Ptr, 1);
        while (!St.empty() && pp.second) { pp = St.top(); St.pop(); }
        if (pp.second) //Шаг вправо — тупик, завершить!
        {
            Ptr = nullptr;
        }
        else Ptr = pp.first;  // Шаг вправо и продолжать
    }
    return (*this);
}

void PrepareAnd(Tree& first, Tree& second, int quantity) {
    for (int i = 0; i < quantity; i++)
    {
        int x = rand() % lim;
        first.insert(x);
        second.insert(x);
    }
}

void ShowTree(Tree& t) {
    t.ShowSet(); t.ShowSub();
}


int main()
{
    using namespace std::chrono;
    //srand((unsigned int)2);
    bool debag = false;
    srand((unsigned int)time(nullptr));
    auto MaxDop = 5;
    auto step = 5;
    int mid_pow = 0, set_count = 0;
    auto Used = [&](Tree& t) { mid_pow += t.Power(); ++set_count; };
    auto DebOut = [debag](Tree& t) {if (debag) { ShowTree(t); system("Pause"); } };
    auto rand = [](int d) {return std::rand() % d; };
    ofstream fout("in.txt");
    for (int i = 0; i < 200; i++) {
        cout << "Start test number " << i + 1 << endl;
        int p = rand(20) + 1 + step*i;
        init_offset();

        Tree A(p, 'A');
        Tree B(p, 'B');
        Tree C(p, 'C');
        Tree D(p, 'D');
        Tree E(p, 'E');
        Tree F(p, 'F');
        Tree G(p, 'G');
        Tree R(p, 'R');
        int q_and(rand(MaxDop) + 1);
        PrepareAnd(A, R, q_and);
        if (debag) { ShowTree(A); ShowTree(R); }
        Used(A); Used(R);

        auto t1 = std::chrono::high_resolution_clock::now();
        if (debag) cout << "\n=== R&=A ===(" << q_and << ") ";
        R &= A;   DebOut(R); Used(R);

        if (debag) ShowTree(B);
        Used(B);
        if (debag) cout << "\n=== R|=B ===";
        R |= B;  DebOut(R); Used(R);

        if (debag) {
            cout << "\n=== R.Concat(F) ===";
            ShowTree(F);
        } Used(F);
        R.CONCAT(F);   DebOut(R);	Used(R);
        int q_sub(rand(MaxDop) + 1);

        PrepareAnd(C, R, q_sub);
        if (debag) {
            ShowTree(R); ShowTree(C);
        } mid_pow += q_sub; Used(C);
        if (debag) cout << "\n=== R-=C ===(" << q_sub << ") ";
        R -= C;  DebOut(R); Used(R);

        int d = rand(R.Power() + 1);
        if (debag) {
            cout << "\n=== R.Subst (G, " << d << ") ===";
            ShowTree(G);
        }Used(G);
        R.SUBST(G, d);   DebOut(R);	Used(R);

        PrepareAnd(D, R, q_sub);
        if (debag) { ShowTree(R); ShowTree(D); }mid_pow += q_sub; Used(D);
        if (debag) cout << "\n=== R^=D ===(" << q_sub << ") ";
        R ^= D;  DebOut(R); Used(R);

        int e = rand(R.Power() + 1);
        if (debag) {
            cout << "\n=== R.Change (E, " << e << ") ===";
            ShowTree(E);
        } Used(E);
        R.CHANGE(E, e);
        DebOut(R);
        Used(R);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto dt = duration_cast<duration<double>>(t2 - t1);
        mid_pow /= set_count;
        fout << p << "  " << dt.count() << std::endl; //Выдача в файл
    }
    cout << "All test was ended" << endl;
   /* cout << "\n=== End === (" << p << " : " << set_count << " * " <<
        mid_pow << " DT=" << (dt.count()) << ")\n";
    std::cin.get();*/
    return 0;
}