#ifndef STUDENT_H
#define STUDENT_H

#include<string>
#include"person.h"

class Student: public Person {
public:
	Student();
	Student(uint32_t id, std::string name, uint32_t age);
	~Student();
	void print() const;
	
private:
	uint32_t id_;
	std::string name_;
	uint32_t age_;
};

#endif
