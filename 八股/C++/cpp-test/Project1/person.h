#ifndef PERSON_H
#define PERSON_H

#include<string>

class Person {
public:
	Person();
	Person(std::string gender, int age_);
	virtual ~Person();
private:
	std::string gender_;
	int age_;
};

#endif
