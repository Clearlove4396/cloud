//单例模式

#include <iostream>
#include <vector>
using namespace std;

//带垃圾回收的懒汉式
class Singleton{
public:
	static Singleton* getInstance(){
		if(ins == nullptr){
			ins = new Singleton();
		}
		return ins;
	}
	
	class cGarbo{
		public:
			~cGarbo(){
				if(Singleton::ins != nullptr){
					delete Singleton::ins;
					Singleton::ins = nullptr;
				}
			}
	}; 
	
	static cGarbo garbo;
	
private:
	Singleton(){}
	Singleton(const Singleton&){}
	Singleton& operator= (const Singleton& ){}
	
	static Singleton* ins;
}; 

Singleton* Singleton::ins = nullptr;
Singleton::cGarbo garbo;


//静态局部变量的懒汉模式
 class Singleton{
public:
	static Singleton* getInstance(){
		static Singleton ins;
		return &ins;
	}
	
private:
	Singleton(){}
	Singleton(const Singleton& ){}
	Singleton& operator= (const Singleton& ){}
}; 


//饿汉模式
class Singleton{
public:
	static Singleton* getInstance(){
		return &ins;
	}
	
private:
	Singleton(){}
	Singleton(const Singleton& ){}
	Singleton& operator= (const Singleton& ){}
	
	static Singleton ins;
};

Singleton Singleton::ins; 


////////

int main()
{
	
}
