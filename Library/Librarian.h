#pragma once
#include <string>

class Librarian {
private:
	std::string DBconnect;
public:
	Librarian(std::string _DBconnect);

	void addBook();
	void deleteBook();
	void checkCatalog();
	void checkRequests();
};