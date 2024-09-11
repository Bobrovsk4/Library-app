#include "Updates.h"

#include <pqxx/pqxx>
#include <string>
#include <iostream>

void update_books(std::string DBconnect) {
	try {
		pqxx::connection connectionObj(DBconnect.c_str());
		pqxx::work worker(connectionObj);

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

		worker.commit();
	}
	catch (const pqxx::sql_error& se) {
		std::cerr << "SQL ERROR : " << se.what() << std::endl;
		std::cerr << "QUERY : " << se.query() << std::endl;
		system("pause");
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR : " << e.what() << std::endl;
		system("pause");
	}
}
void update_requests(std::string DBconnect) {
	try {
		pqxx::connection connectionObj(DBconnect.c_str());
		pqxx::work worker(connectionObj);

		worker.exec(
			"WITH updated AS ("
			"    SELECT request_id, ROW_NUMBER() OVER (ORDER BY request_id) AS new_request_id "
			"    FROM requests"
			") "
			"UPDATE requests "
			"SET request_id = updated.new_request_id "
			"FROM updated "
			"WHERE requests.request_id = updated.request_id;"
		);

		worker.commit();
	}
	catch (const pqxx::sql_error& se) {
		std::cerr << "SQL ERROR : " << se.what() << std::endl;
		std::cerr << "QUERY : " << se.query() << std::endl;
		system("pause");
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR : " << e.what() << std::endl;
		system("pause");
	}
}
void update_user_books(std::string DBconnect) {
	try {
		pqxx::connection connectionObj(DBconnect.c_str());
		pqxx::work worker(connectionObj);

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
	catch (const pqxx::sql_error& se) {
		std::cerr << "SQL ERROR : " << se.what() << std::endl;
		std::cerr << "QUERY : " << se.query() << std::endl;
		system("pause");
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR : " << e.what() << std::endl;
		system("pause");
	}
}