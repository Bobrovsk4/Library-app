#include "Librarian.h"
#include "User.h"

#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <clocale>
#include <thread>
#include <chrono>


int menuLibrarian() {
	std::cout << "\tМеню\n";
	std::cout <<
		"1. Добавить книгу\n" <<
		"2. Удалить книгу\n" <<
		"3. Посмотреть каталог\n"
		"4. Посмотреть заявки\n" <<
		"5. Создать рекомендации\n" <<
		"0. Выйти\n";
	int res;
	std::cin >> res;
	if (res >= 0 && res <= 5)
		return res;
	else
		return menuLibrarian();
}

int menuUser() {
	std::cout << "\tМеню\n";
	std::cout <<
		"1. Приобрести книгу\n" <<
		"2. Вернуть книгу\n" <<
		"0. Выйти\n";
	int res;
	std::cin >> res;
	if (res == 0 || res == 1 || res == 2)
		return res;
	else
		return menuUser();
}

int main()
{
	std::string libPass = "password";
	setlocale(LC_ALL, "ru");
	std::string connectionString = "host=localhost port=5432 dbname=library user=postgres password=barowskiy8968";
	bool running = true;

	while (running) {
		std::cout << "Выберите способ входа: \n" <<
			"1. Войти как библиотекарь;\n" <<
			"2. Войти как пользователь.\n" <<
			"0. Выйти.\n";

		int choice;
		std::cin >> choice;

		switch (choice) {
		case 1: { // Библиотекарь
			system("cls");
			std::cout << "Пароль: ";
			std::string passInput;
			std::cin >> passInput;
			if (passInput.compare(libPass) != 0) {
				std::cout << "Неверный пароль\n";
				break;
			}
			Librarian libr(connectionString);

			int librarianChoice = -1;
			while (librarianChoice != 0) {
				librarianChoice = menuLibrarian();

				system("cls");
				switch (librarianChoice) {
				case 1:
					libr.addBook();
					break;
				case 2:
					libr.deleteBook();
					break;
				case 3:
					libr.checkCatalog();
					break;
				case 4:
					libr.checkRequests(); 
					break;
				case 0:
					break;
				}
			}
			system("cls");
			break;
		}
		case 2: { // Пользователь
			system("cls");
			std::cout << "Логин: ";
			std::string username;
			std::cin >> username;
			
			pqxx::connection connectionObj(connectionString.c_str());
			pqxx::nontransaction reader(connectionObj);

			pqxx::result response = reader.exec("SELECT user_id FROM users WHERE username=" + reader.quote(username) + ";");
			reader.commit();
			if (response.empty()) {
				std::cout << "Пользователь не найден.\nДобавить пользователя? (1-Да, 0-Нет)\n";

				int choice;
				std::cin >> choice;
				while (choice != 1 && choice != 0) {
					std::cout << "Неверный ввод. Попробуйте снова.\n";
					std::cin >> choice;
				}

				if (choice == 0) {
					break;
				}

				system("cls");
				std::cout << "Логин: ";
				std::string login;
				std::cin.ignore();
				std::getline(std::cin, login);
				std::cout << "Почта: ";
				std::string email;
				std::getline(std::cin, email);

				pqxx::work worker(connectionObj);
				worker.exec("INSERT INTO users (username, email) VALUES (" +
					worker.quote(login) + ", " +
					worker.quote(email) + ");");
				worker.commit();

				system("cls");
				std::cout << "Пользователь добавлен.\n";
				std::this_thread::sleep_for(std::chrono::milliseconds(1500));
				system("cls");
			}
			else {
				User user(username, connectionString);

				int user_id = response[0][0].as<int>();
				pqxx::nontransaction reader(connectionObj);
				pqxx::result bookResponse = reader.exec("SELECT b.title, b.author, b.year FROM user_books ub JOIN books b ON ub.book_id = b.book_id WHERE ub.user_id = " + reader.quote(user_id) + ";");
				
				reader.commit();
				if (!bookResponse.empty()) {
					std::cout << "Ваши книги:\n------------\n";
					for (const auto& row : bookResponse) {
						std::cout << "Название: " << row["title"].c_str() << std::endl;
						std::cout << "Автор: " << row["author"].c_str() << std::endl;
						std::cout << "Год выпуска: " << row["year"].c_str() << std::endl << std::endl;
					}
				}
				else {
					std::cout << "У вас еще нет книг.\n\n";
				}

				int userChoice = menuUser();
				system("cls");
				switch (userChoice) {
				case 1:
					user.getBook();
					break;
				case 2:
					if (!bookResponse.empty()) {
						user.returnBook();
					}
					else {
						std::cout << "У вас еще нет книг.\n";
						std::this_thread::sleep_for(std::chrono::milliseconds(1500));
					}
					break;
				case 0:
					break;
				}
			}
			system("cls");
			break;
		}
		case 0: {
			running = false;
			break;
		}
		default: {
			std::cout << "Неверный ввод. Попробуйте снова.\n";
			break;
		}
		}
	}
	return 0;
}
