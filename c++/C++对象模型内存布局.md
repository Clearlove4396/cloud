

#### TIPS

- 含有虚函数或者其父类含有虚函数的类，编译器都会为其添加一个虚函数表。

- C++中虚函数的作用主要是为了实现多态机制。多态，简单来说，是指在继承层次中，父类的指针或引用可以具有多种形态——当它指向某个子类对象时，通过它能够调用到子类的函数，而非父类的函数。

- 使用虚函数实现的多态是一种运行时多态。当一个base class object被直接初始化为一个derived class object时，derived object会被切割以塞入较小的base type内存中，derived type将没有留下任何蛛丝马迹（理解：这里的蛛丝马迹，主要是指虚函数表的函数也被替换为base class中的虚函数实现，而不是derived class中的重写后的虚函数）。一个pointer或一个reference之所以支持多态，是因为它们并不引发内存中任何“与类型有关的内存委托操作（type-dependent commitment）”；会受到改变的，只有它们所指向的内存的“大小和内容解释方式”而已。

  

#### 使用指针访问虚函数表

```c++
/*
	如何使用指针访问虚函数表？
*/
#include <iostream>
using namespace std;

class Base {
private:
	int _i;
public:
	Base(int i): _i(i){}

	virtual void print(void) { cout << "base print" << endl; }
	virtual void setI() { cout << "base setI" << endl; }
	virtual ~Base() { cout << "base ~" << endl; }
};

typedef void (*Func)(void);

int main() {
	Base b(7);
	cout << hex << "基类对象的地址是：" << &b << endl;
	cout << hex << "虚函数表的地址是：" << *(int*)(&b) << endl;
	cout << hex << "虚函数表中第一个函数的地址是：" << *(int*)(*(int*)&b) << endl;
	Func f1 = (Func)(*(int*)(*(int*)&b));
	f1();  //输出base print;

	Func f2 = (Func)(*((int*)(*(int*)&b) + 1));
	f2();  //输出base setI

	getchar();
}
```

![image-20200807182104561](.\pictures\使用指针访问函数)



#### 非继承下的C++对象模型

```c++
class Base
{
public:
    Base(int i) :baseI(i){};
    int getI(){ return baseI; }
    static void countI(){};
    virtual ~Base(){}
    virtual void print(void){ cout << "Base::print()"; }
    
private:
    int baseI;
    static int baseS;
};
```

- nonstatic数据成员被置于每一个类实例化的对象中，而static数据成员被置于类对象之外（数据区中）。static与nonstatic函数也都放在类对象之外。virtual函数的入口地址放在虚函数表中。

- virtual function

  - 每个**类**生成一个表格，成为虚表（vtbl）。虚表中的每一项是一个函数地址——即类中声明的虚函数的入口地址，并且**按照虚函数的声明顺序排列**。

  - 每个类**对象**都有一个虚表指针（vptr），由编译器自动生成。在vs编译器中，vptr被放置在对象的最前端。

  - 另外，虚函数表的前面设置了一个指向type_info的指针，用以支持RTTI（Run Time Type Identification，运行时类型识别）。RTTI是为多态而生成的信息，包括对象继承关系，对象本身的描述等，只有具有虚函数的对象才会生成.

    ```c++
    Base p;
    RTTICompleteObjectLocator str = 
        *((RTTICompleteObjectLocator*)*((int*)*(int*)(&p) - 1));   //注意这里是-1
    
    string classname(str.pTypeDescriptor->name);
    classname = classname.substr(4, classname.find("@@") - 4);
    cout <<  "根据type_info信息输出类名:"<< classname << endl;   //  Base
    ```

![image-20200807184750802](.\pictures\无继承c++对象模型)

#### 单继承

- 如果子类重写了父类的虚函数，则子类虚函数将覆盖表中对应的父类虚函数。注意子类与父类拥有各自的一个虚函数表。
- 若子类并无重写父类虚函数，而是声明了自己新的虚函数，则该虚函数地址将扩充到虚函数表的最后（在vs中看不到）。而对于虚继承，若子类重写父类虚函数，同样地将覆盖父类中虚函数表对应位置，而若子类声明了自己的新的虚函数，则编译器将为子类增加一个新的虚表指针vptr。

```c++
/*
	单继承下的C+=对象模型
*/

#include <iostream>
using namespace std;

class Base
{
public:
    Base(int i) :baseI(i) {};
    int getI() { return baseI; }
    static void countI() {};
    virtual ~Base() { cout << "Base ~" << endl; }
    virtual void print(void) { cout << "Base::print()" << endl; }
    virtual void Base_print(void) { cout << "Base::Base_print()" << endl; }

private:
    int baseI;
    static int baseS;
};

class Derive : public Base
{
public:
    Derive(int d) :Base(1000), DeriveI(d) {};
    //overwrite父类虚函数
    virtual void print(void) { cout << "Drive::print()" << endl; }
    // Derive声明的新的虚函数
    virtual void Drive_print() { cout << "Drive::Drive_print()" << endl; }
    virtual ~Derive() { cout << "Derive ~" << endl; }
private:
    int DeriveI;
};

typedef void(*Func)(void);

int main()
{
    Derive d(7);
    cout << sizeof(d) << endl;   // 12
    //Func f1 = (Func) * ((int*)(*(int*)(&d)) + 0);
    //f1();  // 执行虚函数，会出错
    Func f2 = (Func) * ((int*)(*(int*)(&d)) + 1);
    f2();   // Drive::print()
    Func f3 = (Func) * ((int*)(*(int*)(&d)) + 2);
    f3();   // Base::Base_print()
    Func f4 = (Func) * ((int*)(*(int*)(&d)) + 3);
    f4();  //  Drive::Drive_print()

    Base b = d;
    f2 = (Func) * ((int*)(*(int*)(&b)) + 1);
    f2();  // Base::print()

    getchar();
}
```

![image-20200807195204486](.\pictures\单继承对象模型1)



![image-20200807194456443](.\pictures\单继承对象模型)



#### 一般多继承

- 子类重写父类虚函数时，在虚函数表中会覆盖父类的虚函数。如果两个父类有同名的虚函数，则每一个都会进行覆盖。
- 子类自己的虚函数，则只会加在第一个基类的虚函数表的末尾处，对后面的基类的虚函数表没有影响。
- 父类在子类中按照继承顺序排列。

```c++
/*
    非菱形多继承
*/
#include <iostream>
using namespace std;

class Base
{
public:
    Base(int i) :baseI(i) {};
    virtual ~Base() { cout << "base ~" << endl; }
    int getI() { return baseI; }
    static void countI() {};
    virtual void print(void) { cout << "Base::print()" << endl; }

private:
    int baseI;
    static int baseS;
};

class Base_2
{
public:
    Base_2(int i) :base2I(i) {};
    virtual ~Base_2() { cout << "base2 ~" << endl; }
    int getI() { return base2I; }
    static void countI() {};
    virtual void print(void) { cout << "Base_2::print()" << endl; }

private:
    int base2I;
    static int base2S;
};

class Drive_multyBase :public Base, public Base_2
{
public:
    Drive_multyBase(int d) :Base(1000), Base_2(2000), Drive_multyBaseI(d) {};
    virtual void print(void) { cout << "Drive_multyBase::print" << endl; }
    virtual void Drive_print() { cout << "Drive_multyBase::Drive_print" << endl; }
    virtual ~Drive_multyBase() { cout << "Derive_multyBase ~" << endl; }

private:
    int Drive_multyBaseI;
};

typedef void (*Func)(void);

int main()
{
    Drive_multyBase d(7);

    //Func f1 = (Func) * ((int*)(*(int*)(&d)) + 0);
    //f1();  // 执行析构函数
    Func f2 = (Func) * ((int*)(*(int*)(&d)) + 1);
    f2();   // Drive_multyBase::print
    Func f3 = (Func) * ((int*)(*(int*)(&d)) + 2);
    f3();   // Drive_multyBase::Drive_print
    //Func f4 = (Func) * ((int*)(*(int*)(&d)) + 3);
    //f4();   // 超出虚表边界

    //Func f2_1 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(Base)))) + 0);
    //f2_1();  //执行析构函数
    Func f2_2 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(Base)))) + 1);
    f2_2();  //// Drive_multyBase::print
    //Func f2_3 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(Base)))) + 2);
    //f2_3();  //// 超出虚表边界

    getchar();
}
```

![image-20200807202717145](.\pictures\一般多重继承1)



![image-20200807202242873](.\pictures\一般多重继承)



#### 菱形继承

- 菱形继承也称为钻石继承或重复继承，它指的是基类被某个派生类简单重复继承了多次。这样，派生类对象中拥有多份基类实例。

```c++
/*
	菱形继承
*/

#include <iostream>
using namespace std;

class B
{
public:
    int ib;

public:
    B(int i = 1) :ib(i) {}
    virtual void f() { cout << "B::f()" << endl; }
    virtual void Bf() { cout << "B::Bf()" << endl; }
};

class B1 : public B
{
public:
    int ib1;

public:
    B1(int i = 100) :ib1(i) {}
    virtual void f() { cout << "B1::f()" << endl; }
    virtual void f1() { cout << "B1::f1()" << endl; }
    virtual void Bf1() { cout << "B1::Bf1()" << endl; }
};

class B2 : public B
{
public:
    int ib2;

public:
    B2(int i = 1000) :ib2(i) {}
    virtual void f() { cout << "B2::f()" << endl; }
    virtual void f2() { cout << "B2::f2()" << endl; }
    virtual void Bf2() { cout << "B2::Bf2()" << endl; }
};

class D : public B1, public B2
{
public:
    int id;

public:
    D(int i = 10000) :id(i) {}
    virtual void f() { cout << "D::f()" << endl; }
    virtual void f1() { cout << "D::f1()" << endl; }
    virtual void f2() { cout << "D::f2()" << endl; }
    virtual void Df() { cout << "D::Df()" << endl; }
};

typedef void (*Func)(void);

int main()
{
    D d(7);

    Func f1 = (Func) * ((int*)(*(int*)(&d)) + 0);
    f1();  //D::f()
    Func f2 = (Func) * ((int*)(*(int*)(&d)) + 1);
    f2();   // B::Bf()
    Func f3 = (Func) * ((int*)(*(int*)(&d)) + 2);
    f3();   // D::f1()
    Func f4 = (Func) * ((int*)(*(int*)(&d)) + 3);
    f4();   //B1::Bf1()
    Func f5 = (Func) * ((int*)(*(int*)(&d)) + 4);
    f5();   //D::Df()
    //Func f6 = (Func) * ((int*)(*(int*)(&d)) + 5);
    //f6();   //超出边界

    cout << endl << endl;

    Func f2_1 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(B1)))) + 0);
    f2_1();  //D::f()
    Func f2_2 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(B1)))) + 1);
    f2_2();  //B::Bf()
    Func f2_3 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(B1)))) + 2);
    f2_3();  //D::f2()
    Func f2_4 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(B1)))) + 3);
    f2_4();  //B2::Bf2()
    //Func f2_5 = (Func) * ((int*)(*((int*)((char*)(&d) + sizeof(B1)))) + 4);
    //f2_5();  //  超出边界
    
    // d.ib = 1;               //二义性错误,调用的是B1的ib还是B2的ib？
    d.B1::ib = 1;           //正确
    d.B2::ib = 1;           //正确

    getchar();
}
```

![image-20200807231608876](.\pictures\菱形继承1)





![image-20200807224806042](.\pictures\菱形继承)



#### 虚继承

- 虚继承解决了菱形继承中最派生类拥有多个间接父类实例的情况。
  - 虚继承的子类，**如果本身定义了新的虚函数，则编译器为其生成一个虚函数指针（vptr）以及一张虚函数表**（如果没有自己定义的新的虚函数，则不会生成虚函数表和虚函数指针）。该vptr位于对象内存最前面。
  - 虚继承的子类也单独保留了父类的vptr与虚函数表。这部分内容接在子类内容后面，以一个四字节的0来分界。
  - 虚继承的子类对象中，含有四字节的虚表指针偏移值。

##### 虚基类表

- 在C++对象模型中，虚继承而来的子类会生成一个隐藏的虚基类指针（vbptr），虚基类指针总是在虚函数表指针之后，因而，对某个类实例来说，如果它有虚基类指针，那么虚基类可能在0字节偏移处（当该类没有vptr时），也可能在实例的4字节偏移处。

- 一个类的虚基类指针指向的虚基类表，与虚函数表一样，虚基类表也由多个条目组成，条目中存放的是偏移值。第一个条目存放虚基类表指针（vbptr）所在地址到该类内存首地址的偏移值，由第一段的分析我们知道，这个偏移值为0（类没有vptr）或者-4（类有虚函数，此时有vptr）。虚基类表的第二、第三...个条目依次为该类的最左虚继承父类、次左虚继承父类的内存地址相对于~~虚基类表指针（vbptr）（虚函数表（vptr））~~（子类对象首地址（原文是“虚基类表指针”，但是实际出来的却是相对于“子类对象首地址”））的偏移值。

  ![image-20200807235312588](.\pictures\虚基类表)

##### 简单虚继承

```c++
/*
	简单虚继承
*/

#include <iostream>
using namespace std;

class B
{
public:
    int ib;

public:
    B(int i = 1) :ib(i) {}
    virtual void f() { cout << "B::f()" << endl; }
    virtual void Bf() { cout << "B::Bf()" << endl; }
};

class B1 : virtual public B
{
public:
    int ib1;

public:
    B1(int i = 100) :ib1(i) {}
    virtual void f() { cout << "B1::f()" << endl; }
    virtual void f1() { cout << "B1::f1()" << endl; }
    virtual void Bf1() { cout << "B1::Bf1()" << endl; }
};

typedef void(*Fun)(void);

int main()
{
    B1 a;
    cout << "B1对象内存大小为：" << sizeof(a) << endl;

    //取得B1的虚函数表
    cout << "[0]B1::vptr";
    cout << "\t地址：" << (int*)(&a) << endl;

    //输出虚表B1::vptr中的函数
    for (int i = 0; i < 2; ++i)
    {
        cout << "  [" << i << "]";
        Fun fun1 = (Fun) * ((int*)*(int*)(&a) + i);
        fun1();
        cout << "\t地址：\t" << *((int*)*(int*)(&a) + i) << endl;
    }

    //[1]
    cout << "[1]vbptr ";
    cout << "\t地址：" << (int*)(&a) + 1 << endl;  //虚表指针的地址
    //输出虚基类指针条目所指的内容
    for (int i = 0; i < 2; i++)
    {
        cout << "  [" << i << "]";
        cout << *(int*)((int*)*((int*)(&a) + 1) + i);
        cout << endl;
    }

    //[2]
    cout << "[2]B1::ib1=" << *(int*)((int*)(&a) + 2);
    cout << "\t地址：" << (int*)(&a) + 2;
    cout << endl;

    //[3]
    cout << "[3]值=" << *(int*)((int*)(&a) + 3);
    cout << "\t\t地址：" << (int*)(&a) + 3;
    cout << endl;

    //[4]
    cout << "[4]B::vptr";
    cout << "\t地址：" << (int*)(&a) + 3 << endl;

    //输出B::vptr中的虚函数
    for (int i = 0; i < 2; ++i)
    {
        cout << "  [" << i << "]";
        Fun fun1 = (Fun) * ((int*)*((int*)(&a) + 4) + i);
        fun1();
        cout << "\t地址:\t" << *((int*)*((int*)(&a) + 4) + i) << endl;
    }

    //[5]
    cout << "[5]B::ib=" << *(int*)((int*)(&a) + 5);
    cout << "\t地址: " << (int*)(&a) + 5;
    cout << endl;

    getchar();
}
```



![image-20200808001535741](.\pictures\简单虚继承1)



![image-20200808001445327](.\pictures\简单虚继承)



##### 虚拟菱形继承

```c++
/*
	虚拟菱形继承
*/
#include <iostream>
using namespace std;

class B
{
public:
    int ib;

public:
    B(int i = 1) :ib(i) {}
    virtual void f() { cout << "B::f()" << endl; }
    virtual void Bf() { cout << "B::Bf()" << endl; }
};

class B1 : virtual public B
{
public:
    int ib1;

public:
    B1(int i = 100) :ib1(i) {}
    virtual void f() { cout << "B1::f()" << endl; }
    virtual void f1() { cout << "B1::f1()" << endl; }
    virtual void Bf1() { cout << "B1::Bf1()" << endl; }
};

class B2 : virtual public B
{
public:
    int ib2;

public:
    B2(int i = 1000) :ib2(i) {}
    virtual void f() { cout << "B2::f()" << endl; }
    virtual void f2() { cout << "B2::f2()" << endl; }
    virtual void Bf2() { cout << "B2::Bf2()" << endl; }
};

class D : public B1, public B2
{
public:
    int id;

public:
    D(int i = 10000) :id(i) {}
    virtual void f() { cout << "D::f()" << endl; }
    virtual void f1() { cout << "D::f1()" << endl; }
    virtual void f2() { cout << "D::f2()" << endl; }
    virtual void Df() { cout << "D::Df()" << endl; }
};

typedef void (*Func)(void);

int main()
{
    B b;
    B1 b1;
    B2 b2;
    D d;

    cout << "B的对象模型分布：" << endl;
    cout << "\tB对象大小： " << sizeof(b) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)(*(int*)(&b)) + i);
        f();
    }
    cout << "\t\tB::ib = " << *((int*)(&b) + 1) << endl;

    cout << endl << endl;

    cout << "B1的对象模型分布：" << endl;
    cout << "\tB1对象大小：" << sizeof(b1) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)(*(int*)(&b1)) + i);
        f();
    }
    cout << "\t[1]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        cout << "vbptr" << "[" << i << "]: ";
        cout << *((int*)(*((int*)(&b1) + 1)) + i) << endl;
    }
    cout << "\t[2]" << endl;
    cout << "\t\tB1::ib1 = " << *((int*)(&b1) + 2) << endl;
    cout << "\t[3]" << endl;
    cout << "\t\tspace: " << *((int*)(&b1) + 3) << endl;

    cout << "\t[4]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func)*((int*)(*((int*)(&b1) + 4)) + i);
        f();
    }
    cout << "\t[5]" << endl;
    cout << "\t\tB::ib = " << *((int*)(&b1) + 5) << endl;
    
    cout << endl << endl;

    cout << "D的对象模型分布：" << endl;
    cout << "\tD对象大小：" << sizeof(d) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 3; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 0) + i);
        f();
    }

    cout << "\t[1] B1::vbptr:" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t[" << i << "]= ";
        cout << *((int*)*((int*)(&d) + 1) + i) << endl;
    }

    cout << "\t[2] B1::ib1 = " << *((int*)(&d) + 2) << endl;
    
    cout << "\t[3]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 3) + i);
        f();
    }

    cout << "\t[4] B2::vbptr:" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t[" << i << "]= ";
        cout << *((int*)*((int*)(&d) + 4) + i) << endl;
    }
    
    cout << "\t[5] B2::ib2 = " << *((int*)(&d) + 5) << endl;
    cout << "\t[6] D::id = " << *((int*)(&d) + 6) << endl;
    cout << "\t[7] space = " << *((int*)(&d) + 7) << endl;

    cout << "\t[8]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 8) + i);
        f();
    }

    cout << "\t[8] B:ib = " << *((int*)(&d) + 9) << endl;

    getchar();
}
```

![image-20200808100321052](.\pictures\虚拟菱形继承1)



![image-20200808100214251](.\pictures\虚拟菱形继承)



```c++
#include <iostream>
using namespace std;

class B
{
public:
    int ib;

public:
    B(int i = 1) :ib(i) {}
    virtual void f() { cout << "B::f()" << endl; }
    virtual void Bf() { cout << "B::Bf()" << endl; }
};

class B1 : virtual public B
{
public:
    int ib1;

public:
    B1(int i = 100) :ib1(i) {}
    virtual void f() { cout << "B1::f()" << endl; }
    virtual void f1() { cout << "B1::f1()" << endl; }
    virtual void Bf1() { cout << "B1::Bf1()" << endl; }
};

class B2 : virtual public B
{
public:
    int ib2;

public:
    B2(int i = 1000) :ib2(i) {}
    virtual void f() { cout << "B2::f()" << endl; }
    virtual void f2() { cout << "B2::f2()" << endl; }
    virtual void Bf2() { cout << "B2::Bf2()" << endl; }
};

class D : virtual public B1, public B2
{
public:
    int id;

public:
    D(int i = 10000) :id(i) {}
    virtual void f() { cout << "D::f()" << endl; }
    virtual void f1() { cout << "D::f1()" << endl; }
    virtual void f2() { cout << "D::f2()" << endl; }
    virtual void Df() { cout << "D::Df()" << endl; }
};

typedef void (*Func)(void);

int main()
{
    B b;
    B1 b1;
    B2 b2;
    D d;

    cout << "B的对象模型分布：" << endl;
    cout << "\tB对象大小： " << sizeof(b) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)(*(int*)(&b)) + i);
        f();
    }
    cout << "\t\tB::ib = " << *((int*)(&b) + 1) << endl;

    cout << endl << endl;

    cout << "B1的对象模型分布：" << endl;
    cout << "\tB1对象大小：" << sizeof(b1) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)(*(int*)(&b1)) + i);
        f();
    }
    cout << "\t[1]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        cout << "vbptr" << "[" << i << "]: ";
        cout << *((int*)(*((int*)(&b1) + 1)) + i) << endl;
    }
    cout << "\t[2]" << endl;
    cout << "\t\tB1::ib1 = " << *((int*)(&b1) + 2) << endl;
    cout << "\t[3]" << endl;
    cout << "\t\tspace: " << *((int*)(&b1) + 3) << endl;

    cout << "\t[4]" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)(*((int*)(&b1) + 4)) + i);
        f();
    }
    cout << "\t[5]" << endl;
    cout << "\t\tB::ib = " << *((int*)(&b1) + 5) << endl;

    cout << endl << endl;

    cout << "D的对象模型分布：" << endl;
    cout << "\tD对象大小：" << sizeof(d) << endl;
    cout << "\t[0]" << endl;
    for (int i = 0; i < 3; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 0) + i);
        f();
    }

    cout << "\t[1] B2::vbptr:" << endl;
    for (int i = 0; i < 3; i++) {
        cout << "\t\t[" << i << "]= ";
        cout << *((int*)*((int*)(&d) + 1) + i) << endl;
    }

    cout << "\t[2] B2::ib2 = " << *((int*)(&d) + 2) << endl;
    cout << "\t[3] D::id = " << *((int*)(&d) + 3) << endl;
    cout << "\t[4] space = " << *((int*)(&d) + 4) << endl;

    cout << "\t[5] B1::vptr" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 5) + i);
        f();
    }
    cout << "\t[6] B::ib = " << *((int*)(&d) + 6) << endl;
    cout << "\t[7] space = " << *((int*)(&d) + 7) << endl;

    cout << "\t[8] B1::vptr:" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t";
        Func f = (Func) * ((int*)*((int*)(&d) + 8) + i);
        f();
    }

    cout << "\t[9] B1::vbptr:" << endl;
    for (int i = 0; i < 2; i++) {
        cout << "\t\t[" << i << "]= ";
        cout << *((int*)*((int*)(&d) + 9) + i) << endl;
    }
    cout << "\t[10] B1:ib1 = " << *((int*)(&d) + 10) << endl;

    getchar();
}
```



![image-20200808105522701](C:\Users\ylem\Desktop\面试总结\c++\pictures\菱形继承2_2)

![image-20200808105326594](C:\Users\ylem\Desktop\面试总结\c++\pictures\菱形继承2_1)