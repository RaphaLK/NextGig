// filepath: /home/raphael/Desktop/SMAR/NextGig/backend/server.cpp
#include "server.h"
#include <iostream>
#include <cstdlib>

Server::Server() {
    firebase::AppOptions options;
    options.set_api_key("YOUR_API_KEY");
    // options.set_database_url("YOUR_DATABASE_URL");
    options.set_project_id("YOUR_PROJECT_ID");
    options.set_storage_bucket("YOUR_STORAGE_BUCKET");
    // options.set_messaging_sender_id("YOUR_MESSAGING_SENDER_ID");
    options.set_app_id("YOUR_APP_ID");

    // Initialize Firebase App
    firebase::App* app = firebase::App::Create(options);
    if (!app) {
        std::cerr << "Failed to initialize Firebase App." << std::endl;
        return;
    }

    // Initialize Firebase Auth
    auth = firebase::auth::Auth::GetAuth(app);
    if (!auth) {
        std::cerr << "Failed to initialize Firebase Auth." << std::endl;
        return;
    }


    std::cout << "Firebase initialized successfully." << std::endl;
}

void Server::signIn(const std::string& email, const std::string& password) {

}

void Server::writeData(const std::string& path, const std::string& value) {

}