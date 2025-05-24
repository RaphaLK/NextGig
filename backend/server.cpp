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

firebase::auth::User* Server::signIn(const std::string &email, const std::string &password)
{
    auto future = auth->SignInWithEmailAndPassword(email.c_str(), password.c_str());
    future.OnCompletion([](const firebase::Future<firebase::auth::AuthResult>& completed_future) {
        if (completed_future.error() != firebase::auth::kAuthErrorNone) {
            std::cerr << "Sign-in failed: " << completed_future.error_message() << std::endl;
            return;
        }
        const firebase::auth::AuthResult& result = *completed_future.result();
        std::cout << "Sign-in successful for user: " << result.user.email() << std::endl;
    });

    return nullptr;  
}

void Server::signOut() {
    auth->SignOut();
    std::cout << "User signed out." << std::endl;
}

// Register a new user
void Server::registerUser(const std::string &email, const std::string &password, 
                          const std::string &accountType, const std::string &username,
                          std::function<void(firebase::auth::User*, const std::string& error)> callback) {
    auto future = auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());

    future.OnCompletion([this, email, accountType, username, callback](const firebase::Future<firebase::auth::AuthResult>& completed_future) {
        if (completed_future.error() != firebase::auth::kAuthErrorNone) {
            callback(nullptr, completed_future.error_message());
            return;
        }

        const firebase::auth::AuthResult& result = *completed_future.result();
        firebase::auth::User user = result.user;

        std::string uid = user.uid();
        firebase::database::DatabaseReference ref = database->GetReference(("users/" + uid).c_str());

        std::map<std::string, firebase::Variant> userData;
        userData["email"] = email;
        userData["username"] = username;
        userData["accountType"] = accountType;
        userData["createdAt"] = static_cast<int64_t>(::time(nullptr));

        ref.SetValue(userData);

        // yeeesh not sure if this will work
        callback(&user, "");
    });
}

// Check if user is a HiringManager
bool Server::isHiringManager(const std::string& uid) {
    firebase::database::DatabaseReference ref = database->GetReference(("users/" + uid + "/accountType").c_str());
    auto future = ref.GetValue();
    future.OnCompletion([](const firebase::Future<firebase::database::DataSnapshot>& result) {
        if (result.error() != firebase::database::kErrorNone) {
            std::cerr << "Error getting user type: " << result.error_message() << std::endl;
            return false;
        }
        
        firebase::database::DataSnapshot snapshot = *result.result();
        if (snapshot.exists() && snapshot.value().is_string()) {
            return snapshot.value().string_value() == "Hiring Manager";
        }
        return false;
    });
    
    return false; // This will be replaced in the completion callback
}

void Server::writeData(const std::string &path, const std::string &value)
{
}