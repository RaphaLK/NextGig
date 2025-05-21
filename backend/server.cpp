// filepath: /home/raphael/Desktop/SMAR/NextGig/backend/server.cpp
#include "server.h"
#include <firebase/app.h>
#include <firebase/auth.h>
#include <firebase/database.h>
#include <iostream>

Server::Server() {
    // Initialize Firebase App
    firebase::App* app = firebase::App::Create();
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

    // Initialize Firebase Database
    database = firebase::database::Database::GetInstance(app);
    if (!database) {
        std::cerr << "Failed to initialize Firebase Database." << std::endl;
        return;
    }

    std::cout << "Firebase initialized successfully." << std::endl;
}

void Server::signIn(const std::string& email, const std::string& password) {
    auto result = auth->SignInWithEmailAndPassword(email.c_str(), password.c_str());
    if (result.error() != firebase::auth::kAuthErrorNone) {
        std::cerr << "Sign-in failed: " << result.error_message() << std::endl;
    } else {
        std::cout << "Sign-in successful for user: " << result.user()->email() << std::endl;
    }
}

void Server::writeData(const std::string& path, const std::string& value) {
    firebase::database::DatabaseReference ref = database->GetReference(path.c_str());
    ref.SetValue(value.c_str());
}