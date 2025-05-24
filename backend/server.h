#pragma once
#include <firebase/auth.h>
#include <firebase/database.h>
#include <firebase/app.h>
#include <string>
#include <map>

class Server
{
public:
    Server();
    firebase::auth::User *signIn(const std::string &email, const std::string &password);
    firebase::auth::User *registerUser(const std::string &email, const std::string &password,
                      const std::string &accountType, const std::string &username,
                      std::function<void(firebase::auth::User *, const std::string &error)> callback);

    bool isHiringManager(const std::string &uid);
    void writeData(const std::string &path, const std::string &value);
    void signOut();

private:
    firebase::auth::Auth *auth;
    firebase::database::Database *database;
};