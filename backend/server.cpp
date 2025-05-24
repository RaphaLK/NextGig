#include "server.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QString>
#include <QByteArray>

Server::Server()
{
    firebase::AppOptions options;
    options.set_api_key("AIzaSyDZd-3ROjIock8O2-NgpoLxgciVsfVq5Uk");
    options.set_project_id("scumydegree-f4ed8");
    options.set_database_url("https://scumydegree-f4ed8.firebaseio.com");
    options.set_storage_bucket("scumydegree-f4ed8.appspot.com");
    options.set_app_id("1:174532801638:android:c02e1725c79e1d760c69a7");

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

    // Initialize Firestore -- i love segfaults
    database = firebase::database::Database::GetInstance(app);
    if (!database)
    {
        std::cerr << "Failed to initialize Firebase Database." << std::endl;
        return;
    }
    database->set_persistence_enabled(true);

    std::cout << "Firebase initialized successfully." << std::endl;
}

firebase::auth::User *Server::signIn(const std::string &email, const std::string &password)
{
    auto future = auth->SignInWithEmailAndPassword(email.c_str(), password.c_str());
    future.OnCompletion([](const firebase::Future<firebase::auth::AuthResult> &completed_future)
                        {
        if (completed_future.error() != firebase::auth::kAuthErrorNone) {
            std::cerr << "Sign-in failed: " << completed_future.error_message() << std::endl;
            return;
        }
        const firebase::auth::AuthResult& result = *completed_future.result();
        std::cout << "Sign-in successful for user: " << result.user.email() << std::endl; });

    return nullptr;
}

void Server::signOut()
{
    auth->SignOut();
    std::cout << "User signed out." << std::endl;
}

// Register a new user
firebase::auth::User *Server::registerUser(const std::string &email, const std::string &password,
                                           const std::string &accountType, const std::string &username,
                                           std::function<void(firebase::auth::User *, const std::string &error)> callback)
{
    auto future = auth->CreateUserWithEmailAndPassword(email.c_str(), password.c_str());

    future.OnCompletion([this, email, accountType, username, callback](const firebase::Future<firebase::auth::AuthResult> &completed_future)
                        {
        if (completed_future.error() != firebase::auth::kAuthErrorNone) {
            std::cerr << "Registration failed: " << completed_future.error_message() << std::endl;
            callback(nullptr, completed_future.error_message());
            return;
        }

        try {
            std::cout << "User created successfully" << std::endl;
            
            // Get a pointer to the user for safe use after this function completes
            const firebase::auth::AuthResult& result = *completed_future.result();
            const firebase::auth::User& user = result.user;
            firebase::auth::User* userPtr = const_cast<firebase::auth::User*>(&user);
            
            if (!userPtr) {
                callback(nullptr, "User object is null");
                return;
            }
            
            std::string uid = userPtr->uid();
            std::cout << "User ID: " << uid << std::endl;
            
            firebase::database::DatabaseReference ref = database->GetReference(("users/" + uid).c_str());
            
            std::map<std::string, firebase::Variant> userData;
            userData["email"] = email;
            userData["username"] = username;
            userData["accountType"] = accountType;
            userData["createdAt"] = static_cast<int64_t>(::time(nullptr));
            
            // Wait for database write to complete before calling the callback
            auto setValueFuture = ref.SetValue(userData);
            setValueFuture.OnCompletion([userPtr, callback](const firebase::Future<void>& future) {
                if (future.error() != firebase::database::kErrorNone) {
                    std::cerr << "Failed to save user data: " << future.error_message() << std::endl;
                    callback(nullptr, future.error_message());
                } else {
                    std::cout << "User data saved successfully" << std::endl;
                    callback(userPtr, "");
                }
            });
        } catch (const std::exception& e) {
            std::cerr << "Exception during registration: " << e.what() << std::endl;
            callback(nullptr, std::string("Exception: ") + e.what());
        } });

    return nullptr;
}

bool Server::startServer(int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cerr << "Failed to set socket options" << std::endl;
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Failed to bind to port " << port << std::endl;
        return false;
    }

    if (listen(serverSocket, 3) < 0)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }

    running = true;
    serverThread = std::thread(&Server::serverLoop, this);

    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void Server::stopServer() {
    if (!running) return;
    
    running = false;
    
    // Close server socket to unblock accept()
    close(serverSocket);
    
    // Wait for server thread to finish
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int socket : clientSockets) {
            close(socket);
        }
        clientSockets.clear();
    }
    
    std::cout << "Server stopped" << std::endl;
}

void Server::serverLoop()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (running)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (clientSocket < 0)
        {
            if (running)
            {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }

        // Add client to list
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        // Start a thread to handle this client
        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void Server::handleClient(int clientSocket)
{
    // Buffer to store incoming data
    char buffer[4096];
    ssize_t bytesRead;

    // Set socket to non-blocking mode
    int flags = fcntl(clientSocket, F_GETFL, 0);
    fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

    bool clientConnected = true;
    std::string requestBuffer;

    while (clientConnected && getServerStatus())
    {
        // Read data from socket
        bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead > 0)
        {
            // Null-terminate the buffer
            buffer[bytesRead] = '\0';

            // Add to request buffer
            requestBuffer += buffer;

            // Check if we have a complete request
            // For simplicity, assume requests end with a newline character
            size_t newlinePos = requestBuffer.find('\n');
            while (newlinePos != std::string::npos)
            {
                // Extract complete request
                std::string request = requestBuffer.substr(0, newlinePos);
                requestBuffer = requestBuffer.substr(newlinePos + 1);

                // Process the request
                processRequest(clientSocket, request);

                // Check for more complete requests in buffer
                newlinePos = requestBuffer.find('\n');
            }
        }
        else if (bytesRead == 0)
        {
            // Client disconnected
            clientConnected = false;
        }
        else
        {
            // Error or would block
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                clientConnected = false;
            }

            // Sleep briefly to avoid busy wait
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Remove client from list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket),
                            clientSockets.end());
    }

    // Close the socket
    close(clientSocket);
    std::cout << "Client disconnected" << std::endl;
};

void Server::processRequest(int clientSocket, const std::string &request)
{
    std::cout << "Received request: " << request << std::endl;

    // Parse JSON request using QJson
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(request), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject())
    {
        QJsonObject errorResponse;
        errorResponse["status"] = "error";
        errorResponse["error"] = "Invalid JSON format";

        QJsonDocument responseDoc(errorResponse);
        sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
        return;
    }

    QJsonObject jsonRequest = jsonDoc.object();
    QString requestType = jsonRequest["type"].toString();

    if (requestType == "signin")
    {
        std::cout << "Processing signin request" << std::endl;
        QString email = jsonRequest["email"].toString();
        QString password = jsonRequest["password"].toString();

        auto future = auth->SignInWithEmailAndPassword(email.toStdString().c_str(), password.toStdString().c_str());
        future.OnCompletion([this, clientSocket](const firebase::Future<firebase::auth::AuthResult> &completed_future)
                            {
            QJsonObject response;

            if (completed_future.error() != firebase::auth::kAuthErrorNone) {
                response["status"] = "error";
                response["error"] = completed_future.error_message();
            } else {
                const firebase::auth::AuthResult &result = *completed_future.result();
                response["status"] = "success";
                response["uid"] = QString::fromStdString(result.user.uid());
                response["email"] = QString::fromStdString(result.user.email());
            }

            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "register")
    {
        std::cout << "Processing register request for " << jsonRequest["email"].toString().toStdString() << std::endl;
        QString email = jsonRequest["email"].toString();
        QString password = jsonRequest["password"].toString();
        QString accountType = jsonRequest["accountType"].toString();
        QString username = jsonRequest["username"].toString();

        registerUser(email.toStdString(), password.toStdString(), accountType.toStdString(), username.toStdString(),
                     [this, clientSocket](firebase::auth::User *user, const std::string &error)
                     {
                         QJsonObject response;

                         if (!error.empty())
                         {
                             response["status"] = "error";
                             response["error"] = QString::fromStdString(error);
                         }
                         else
                         {
                             response["status"] = "success";
                             if (user)
                             {
                                 response["uid"] = QString::fromStdString(user->uid());
                                 response["email"] = QString::fromStdString(user->email());
                             }
                         }

                         QJsonDocument responseDoc(response);
                         sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
                     });
    }
    else if (requestType == "signout")
    {
        signOut();
        QJsonObject response;
        response["status"] = "success";

        QJsonDocument responseDoc(response);
        sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
    }
    else if (requestType == "isHiringManager")
    {
        QString uid = jsonRequest["uid"].toString();
        firebase::database::DatabaseReference ref = database->GetReference(("users/" + uid.toStdString() + "/accountType").c_str());
        auto future = ref.GetValue();

        future.OnCompletion([this, clientSocket](const firebase::Future<firebase::database::DataSnapshot> &result)
                            {
            QJsonObject response;

            if (result.error() != firebase::database::kErrorNone) {
                response["status"] = "error";
                response["error"] = result.error_message();
            } else {
                firebase::database::DataSnapshot snapshot = *result.result();
                response["status"] = "success";
                if (snapshot.exists() && snapshot.value().is_string()) {
                    response["isHiringManager"] = (snapshot.value().string_value() == "Hiring Manager");
                } else {
                    response["isHiringManager"] = false;
                }
            }

            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else
    {
        QJsonObject response;
        response["status"] = "error";
        response["error"] = "Unknown request type: " + requestType;

        QJsonDocument responseDoc(response);
        sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
    }
}

void Server::sendResponse(int clientSocket, const std::string &response) {
    // Add newline as message terminator
    std::string fullResponse = response + "\n";
    
    // Send response
    ssize_t bytesSent = send(clientSocket, fullResponse.c_str(), fullResponse.size(), 0);
    
    if (bytesSent < 0) {
        std::cerr << "Failed to send response: " << strerror(errno) << std::endl;
    } else if (static_cast<size_t>(bytesSent) < fullResponse.size()) {
        std::cerr << "Warning: Not all data was sent" << std::endl;
    } else {
        std::cout << "Response sent successfully" << std::endl;
    }
}// Check if user is a HiringManager
bool Server::isHiringManager(const std::string &uid)
{
    firebase::database::DatabaseReference ref = database->GetReference(("users/" + uid + "/accountType").c_str());
    auto future = ref.GetValue();
    future.OnCompletion([](const firebase::Future<firebase::database::DataSnapshot> &result)
                        {
        if (result.error() != firebase::database::kErrorNone) {
            std::cerr << "Error getting user type: " << result.error_message() << std::endl;
            return false;
        }
        
        firebase::database::DataSnapshot snapshot = *result.result();
        if (snapshot.exists() && snapshot.value().is_string()) {
            return snapshot.value().string_value() == "Hiring Manager";
        }
        return false; });

    return false; // This will be replaced in the completion callback
}