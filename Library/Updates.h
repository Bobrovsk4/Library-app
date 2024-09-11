#pragma once

#include <pqxx/pqxx>
#include <string>

void update_books(std::string DBconnect);
void update_requests(std::string DBconnect);
void update_user_books(std::string DBconnect);