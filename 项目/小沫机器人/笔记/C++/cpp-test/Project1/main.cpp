#include<iostream>
#include<memory>
#include"student.h"
#include"route.h"

using namespace std;

int main(void)
{
#ifdef DEMO01
	allocator<Student> alloc;
	
	// 分配5个Student大小的内存
	Student *p = alloc.allocate(5);
	
	// 在内存上构造对象
	alloc.construct(p, 1, "jason", 12);	// 创建第一个对象
	alloc.construct(p+1, 2, "peter", 32);	// 创建第二个对象
		
	// 使用对象
	p->print();
	(p+1)->print();
	
	// 销毁对象
	alloc.destroy(p+1);
	alloc.destroy(p);
	
	// 释放内存
	alloc.deallocate(p, 5);
#endif
	
#ifdef DEMO02
	Person *person = new Student();
	delete(person);
#endif
}
