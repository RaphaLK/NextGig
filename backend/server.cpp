// filepath: /home/raphael/Desktop/SMAR/NextGig/backend/server.cpp
#include "server.h"
#include <iostream>
#include <cstdlib>

Server::Server()
{
    firebase::AppOptions options;
    options.set_api_key("yAIzaSyDZd-3ROjIock8O2-NgpoLxgciVsfVq5Uk");
    // options.set_database_url("DATABASE_URL");
    options.set_project_id("scumydegree-f4ed8");
    options.set_storage_bucket("1:174532801638:android:c02e1725c79e1d760c69a7");
    // options.set_messaging_sender_id("MESSAGING_SENDER_ID");
    options.set_app_id("scumydegree-f4ed8.firebasestorage.app");

    // Initialize Firebase App
    firebase::App *app = firebase::App::Create(options);
    if (!app)
    {
        std::cerr << "Failed to initialize Firebase App." << std::endl;
        return;
    }

    // Initialize Firebase Auth
    auth = firebase::auth::Auth::GetAuth(app);
    if (!auth)
    {
        std::cerr << "Failed to initialize Firebase Auth." << std::endl;
        return;
    }

    std::cout << "Firebase initialized successfully." << std::endl;
}

void Server::signIn(const std::string &email, const std::string &password)
{
}

void Server::writeData(const std::string &path, const std::string &value)
{
}