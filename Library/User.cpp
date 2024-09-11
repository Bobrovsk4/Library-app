#include "User.h"
#include "Updates.h"

#include <pqxx/pqxx>
#include <iostream>


int User::getID()
{
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::nontransaction reader(connectionObj);
	pqxx::result id = reader.exec("SELECT user_id FROM users WHERE username = " + reader.quote(name) + ";");
	reader.commit();
	return id[0][0].as<int>();
}

User::User(std::string _name, std::string _DBconnect) {
	name = _name;
	DBconnect = _DBconnect;
}


void User::getBook() {
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::work worker(connectionObj);
	pqxx::result books = worker.exec("SELECT book_id, title, author, year FROM books WHERE status = true;");

	int count = 0;
	std::cout << "Доступные книги:\n------------\n";
	for (const auto& row : books) {
		std::cout << "ID: " << row["book_id"].c_str() << std::endl;
		std::cout << "Название: " << row["title"].c_str() << std::endl;
		std::cout << "Автор: " << row["author"].c_str() << std::endl;
		std::cout << "Год выпуска: " << row["year"].c_str() << std::endl;
		std::cout << std::endl;
		++count;
	}
	std::cout << std::endl;
	
	std::cout << "Введите номер книги, которую желаете приобрести: ";
	int choice;
	std::cin >> choice;
	while (choice <= 0 || choice > count) {
		std::cout << "Неверный ввод. Попробуйте снова\n";
		std::cin >> choice;
	}

	worker.exec("INSERT INTO requests (user_id, book_id) VALUES (" +
		worker.quote(getID()) + ", " +
		worker.quote(choice) + ");"
	);
	worker.commit();

	update_requests(DBconnect);

	system("cls");
}

void User::returnBook() {
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::work worker(connectionObj);

	pqxx::result bookResponse = worker.exec("SELECT b.book_id, b.title, b.author, b.year FROM user_books ub JOIN books b ON ub.book_id = b.book_id WHERE ub.user_id = " + worker.quote(getID()) + ";");;

	std::cout << "----------\n";
	if (!bookResponse.empty()) {
		for (const auto& row : bookResponse) {
			std::cout << "ID: " << row["book_id"].c_str() << std::endl;
			std::cout << "Название: " << row["title"].c_str() << std::endl;
			std::cout << "Автор: " << row["author"].c_str() << std::endl;
			std::cout << "Год выпуска: " << row["year"].c_str() << std::endl << std::endl;
		}
	}
	else {
		std::cout << "У вас еще нет книг.\n\n";
	}

	std::cout << "Введите ID книги, которую хотите вернуть: ";
	int book_id;
	std::cin >> book_id;

	worker.exec("DELETE FROM user_books WHERE book_id = " + worker.quote(book_id) + ";");

	worker.exec("UPDATE books SET status = true WHERE book_id = " + worker.quote(book_id) + ";");
	
	worker.commit();
}
