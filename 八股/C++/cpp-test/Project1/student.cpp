#include "student.h"
#include <iostream>

Student::Student() {
	std::cout << "[Student]的默认构造函数被调用." << std::endl;
}

Student::Student(uint32_t id, std::string name, uint32_t age)
:id_(id), name_(name), age_(age) {}

void Student::print() const {
	std::cout	<< "Student [" << this->name_ << "]'s "
				<< "id is [" << this->id_ << "] "
				<< "and "
				<< "age is [" << this->age_ << "]."
				<< std::endl;
}

Student::~Student() {
	std::cout << "[Student]的析构函数被调用." << std::endl;
}

