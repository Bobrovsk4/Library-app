#pragma once
#include <string>

class User {
private:
	std::string name;
	std::string DBconnect;
	int getID();
public:
	User(std::string _name, std::string _DBconnect);

	std::string getUsername();
	void getBook();
	void returnBook();
};