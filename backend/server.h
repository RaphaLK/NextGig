#pragma once
#include <string>
#include <firebase/auth.h>
#include <firebase/database.h>
#include <firebase/app.h>


class Server {
public:
    Server();
    void signIn(const std::string& email, const std::string& password);
    void writeData(const std::string& path, const std::string& value);

private:
    firebase::auth::Auth* auth;
    firebase::database::Database* database;
};