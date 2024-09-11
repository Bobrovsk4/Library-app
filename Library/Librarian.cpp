#include "Librarian.h"
#include "Updates.h"

#include <curl/curl.h>
#include <pqxx/pqxx>
#include <iostream>
#include <thread>
#include <chrono>

Librarian::Librarian(std::string _DBconnect) {
	DBconnect = _DBconnect;
}

void Librarian::addBook() {
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::work worker(connectionObj);

	std::cout << "Название: ";
	std::string title;
	std::cin.ignore();
	std::getline(std::cin, title);
	std::cout << "Автор: ";
	std::string author;
	std::getline(std::cin, author);
	std::cout << "Год выпуска: ";
	int year;
	std::cin >> year;

	worker.exec("INSERT INTO books (title, author, year, status) VALUES (" +
		worker.quote(title) + ", " +
		worker.quote(author) + ", " +
		worker.quote(year) + ", " +
		worker.quote("true") + ");"
	);
	worker.commit();

	update_books(DBconnect);
	update_user_books(DBconnect);
	
	system("cls");
	std::cout << "Книга добавлена.";
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	system("cls");
}
void Librarian::deleteBook() {
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::nontransaction reader(connectionObj);
	pqxx::result response = reader.exec("SELECT book_id, title FROM books;");
	reader.commit();
	int counter = 0;
	for (const auto& row : response)
	{
		for (const auto& field : row)
		{
			std::cout << field.c_str() << ". ";

		}
		std::cout << std::endl;
		++counter;
	}

	std::cout << "\nУдалить книгу (номер): ";
	int book_id;
	std::cin >> book_id;
	while (book_id < 1 || book_id > counter) {
		std::cout << "Неверный ввод. Попробуйте снова.\n";
		std::cin >> book_id;
	}


	std::cout << "Вы уверены, что хотите удалить эту книгу? (1-Да, 0-Нет)\n";
	int res;
	std::cin >> res;
	while (res < 0 || res > 1) {
		std::cout << "Неверный ввод. Попробуйте снова.\n";
		std::cin >> res;
	}


	if (res == 1) {
		try {
			pqxx::work worker(connectionObj);

			worker.exec("DELETE FROM books WHERE book_id = " + worker.quote(book_id) + ";");

			worker.exec(
				"WITH updated_books AS ("
				"    SELECT book_id, ROW_NUMBER() OVER (ORDER BY book_id) AS new_book_id "
				"    FROM books"
				") "
				"UPDATE books "
				"SET book_id = updated_books.new_book_id "
				"FROM updated_books "
				"WHERE books.book_id = updated_books.book_id;"
			);

			worker.exec(
				"WITH updated_books AS ("
				"    SELECT book_id, ROW_NUMBER() OVER (ORDER BY book_id) AS new_book_id "
				"    FROM books"
				") "
				"UPDATE user_books "
				"SET book_id = updated_books.new_book_id "
				"FROM updated_books "
				"WHERE user_books.book_id = updated_books.book_id;"
			);

			worker.commit();
		}
		catch (const pqxx::sql_error& e) {
			std::cerr << e.what() << std::endl;
			std::cerr << e.query() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		system("cls");
		std::cout << "Книга удалена." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		system("cls");
	}
	else {
		return;
	}

}
void Librarian::checkCatalog() {
	try {
		pqxx::connection connectionObj(DBconnect.c_str());
		pqxx::nontransaction reader(connectionObj);
		
		pqxx::result books = reader.exec("SELECT title,author, year FROM books WHERE status = true;");

		if (!books.empty()) {
			std::cout << "Доступные книги:\n-------------\n";
			for (const auto& row : books) {
				std::cout << "Название: " << row["title"].c_str() << std::endl;
				std::cout << "Автор: " << row["author"].c_str() << std::endl;
				std::cout << "Год выпуска: " << row["year"].c_str() << std::endl << std::endl;
			}
		}

		books = reader.exec("SELECT title,author, year FROM books WHERE status = false;");

		if (!books.empty()) {
			std::cout << "Недоступные книги:\n-------------\n";
			for (const auto& row : books) {
				std::cout << "Название: " << row["title"].c_str() << std::endl;
				std::cout << "Автор: " << row["author"].c_str() << std::endl;
				std::cout << "Год выпуска: " << row["year"].c_str() << std::endl << std::endl;
			}
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}
void Librarian::checkRequests() {
	pqxx::connection connectionObj(DBconnect.c_str());
	pqxx::nontransaction reader(connectionObj);
	pqxx::result requests = reader.exec("SELECT request_id, user_id, book_id FROM requests;");

	if (requests.empty()) {
		std::cout << "Нет активных заявок.\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		system("cls");
		return;
	}

	int count = 0;
	pqxx::result data;
	for (const auto& row : requests) {
		std::cout << "ID: " << row["request_id"].c_str() << std::endl;

		data = reader.exec("SELECT username FROM users WHERE user_id = " + reader.quote(row["user_id"].c_str()) + ";");
		std::cout << "Пользователь: " << data[0][0].as<std::string>() << std::endl;

		data = reader.exec("SELECT title FROM books WHERE book_id = " + reader.quote(row["book_id"].c_str()) + ";");
		std::cout << "Название: " << data[0][0].as<std::string>() << std::endl;

		++count;
		std::cout << "-----------\n";
	}
	reader.commit();

	std::cout << "\nВыберите номер заявки: ";
	int num;
	std::cin >> num;
	while (num <= 0 || num > count) {
		std::cout << "Неверный ввод. Попробуйте снова\n";
		std::cin >> num;
	}

	std::cout << "1-Принять / 2-Отказать\n";
	int choice;
	std::cin >> choice;
	while (choice != 1 && choice != 2) {
		std::cout << "Неверный ввод. Попробуйте снова\n";
		std::cin >> choice;
	}

	switch (choice) {
	case 1: {
		pqxx::nontransaction reader(connectionObj);
		pqxx::result data = reader.exec("SELECT user_id, book_id FROM requests WHERE request_id = " + reader.quote(num) + ";");
		reader.commit();

		if (data.empty()) {
			throw std::runtime_error("Request not found");
		}

		int user_id = data[0][0].as<int>();
		int book_id = data[0][1].as<int>();

		pqxx::work worker(connectionObj);

		worker.exec("INSERT INTO user_books (user_id, book_id) VALUES (" +
			worker.quote(user_id) + ", " +
			worker.quote(book_id) + ");"
		);

		worker.exec("DELETE FROM requests WHERE request_id = " + worker.quote(num) + ";");

		worker.exec("UPDATE books SET status = false WHERE book_id = " + worker.quote(book_id) + ";");

		worker.commit();

		update_requests(DBconnect);
		update_user_books(DBconnect);

		break;
	}
	case 2: {
		pqxx::work worker(connectionObj);
		worker.exec("DELETE FROM requests WHERE request_id = " + worker.quote(num) + ";");
		worker.commit();

		update_requests(DBconnect);

		break;
	}
	}
	system("cls");
}