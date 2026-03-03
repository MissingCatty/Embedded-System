#include "person.h"
#include <iostream>

Person::Person() {
	std::cout << "[Person]的默认构造函数被调用." << std::endl;
}

Person::Person(std::string gender, int age)
:gender_(gender), age_(age) {}

Person::~Person() {
	std::cout << "[Person]的析构函数被调用." << std::endl;
}
