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
#include <QDebug>
#include <QJsonArray>

Server::Server()
{
    firebase::AppOptions options;
    options.set_api_key("AIzaSyDZd-3ROjIock8O2-NgpoLxgciVsfVq5Uk");
    options.set_project_id("scumydegree-f4ed8");
    // options.set_database_url("https://scumydegree-f4ed8.firebaseio.com");
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

    // Initialize Firestore
    firestore = firebase::firestore::Firestore::GetInstance(app);
    if (!firestore)
    {
        std::cerr << "Failed to initialize Firebase Firestore." << std::endl;
        return;
    }

    // Test Firestore connection
    // firebase::firestore::DocumentReference testRef = firestore->Collection("server_tests").Document("connection_test");
    // firebase::firestore::MapFieldValue testData;
    // testData["status"] = firebase::firestore::FieldValue::String("connected");
    // testData["timestamp"] = firebase::firestore::FieldValue::ServerTimestamp();

    // auto testFuture = testRef.Set(testData);
    // testFuture.OnCompletion([](const firebase::Future<void> &future)
    //                         {
    //     if (future.error() != firebase::firestore::kErrorOk) {
    //         std::cerr << "Firestore test write failed: " << future.error_message() << std::endl;
    //     } else {
    //         std::cout << "Firestore test write successful" << std::endl;
    //     } });
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
            
            // Use Firestore instead of Realtime Database
            firebase::firestore::DocumentReference userDoc = firestore->Collection("users").Document(uid);
            
            // Start with common fields for both user types
            firebase::firestore::MapFieldValue userData;
            userData["email"] = firebase::firestore::FieldValue::String(email);
            userData["username"] = firebase::firestore::FieldValue::String(username);
            userData["accountType"] = firebase::firestore::FieldValue::String(accountType);
            userData["createdAt"] = firebase::firestore::FieldValue::ServerTimestamp();
            userData["description"] = firebase::firestore::FieldValue::String("");
            
            // Create empty arrays for common collections
            std::vector<firebase::firestore::FieldValue, std::allocator<firebase::firestore::FieldValue>> emptyJobHistory;
            std::vector<firebase::firestore::FieldValue, std::allocator<firebase::firestore::FieldValue>> emptyTags;
            std::vector<firebase::firestore::FieldValue, std::allocator<firebase::firestore::FieldValue>> emptyAccomplishments;
            
            userData["tags"] = firebase::firestore::FieldValue::Array(emptyTags);
            userData["accomplishments"] = firebase::firestore::FieldValue::Array(emptyAccomplishments);
            
            // Create empty education struct
            firebase::firestore::MapFieldValue education;
            education["jobTitle"] = firebase::firestore::FieldValue::String("");
            education["startDate"] = firebase::firestore::FieldValue::String("");
            education["endDate"] = firebase::firestore::FieldValue::String("");
            education["description"] = firebase::firestore::FieldValue::String("");
            
            userData["education"] = firebase::firestore::FieldValue::Map(education);
            userData["jobHistory"] = firebase::firestore::FieldValue::Array(emptyJobHistory);
            
            // Add user type-specific fields
            if (accountType == "Hiring Manager") {
                userData["companyName"] = firebase::firestore::FieldValue::String("");
                userData["companyDescription"] = firebase::firestore::FieldValue::String("");
                
                // Add additional Hiring Manager specific fields
                std::vector<firebase::firestore::FieldValue, std::allocator<firebase::firestore::FieldValue>> emptyJobPostings;
                userData["jobPostings"] = firebase::firestore::FieldValue::Array(emptyJobPostings);
            } else {
                // For Freelancer
                userData["hourlyRate"] = firebase::firestore::FieldValue::Double(0.0);
                
                // Add additional Freelancer specific fields
                std::vector<firebase::firestore::FieldValue, std::allocator<firebase::firestore::FieldValue>> emptySkills;
                userData["skills"] = firebase::firestore::FieldValue::Array(emptySkills);
                
                // Freelancer preferences
                firebase::firestore::MapFieldValue preferences;
                preferences["remoteOnly"] = firebase::firestore::FieldValue::Boolean(false);
                preferences["minHourlyRate"] = firebase::firestore::FieldValue::Double(0.0);
                userData["preferences"] = firebase::firestore::FieldValue::Map(preferences);
            }
            
            // Wait for firestore write to complete before calling the callback
            auto setDocFuture = userDoc.Set(userData);
            setDocFuture.OnCompletion([userPtr, uid, callback](const firebase::Future<void>& future) {
                if (future.error() != firebase::firestore::kErrorOk) {
                    std::cerr << "Failed to save user data: " << future.error_message() << std::endl;
                    callback(nullptr, future.error_message());
                } else {
                    std::cout << "User data saved successfully for uid: " << uid << std::endl;
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

void Server::stopServer()
{
    if (!running)
        return;

    running = false;

    // Close server socket to unblock accept()
    close(serverSocket);

    // Wait for server thread to finish
    if (serverThread.joinable())
    {
        serverThread.join();
    }

    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int socket : clientSockets)
        {
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
            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
        } else {
            const firebase::auth::AuthResult &result = *completed_future.result();
            std::string uid = result.user.uid();
            
            // Now fetch the user's data from Firestore
            firebase::firestore::DocumentReference userDoc = firestore->Collection("users").Document(uid);
            auto userFuture = userDoc.Get();
            
            userFuture.OnCompletion([this, clientSocket, uid, result](const firebase::Future<firebase::firestore::DocumentSnapshot> &future) {
                QJsonObject response;
                
                if (future.error() != firebase::firestore::kErrorOk) {
                    response["status"] = "error";
                    response["error"] = future.error_message();
                } else {
                    firebase::firestore::DocumentSnapshot snapshot = *future.result();
                    
                    if (snapshot.exists()) {
                        // Create a success response with all user data
                        response["status"] = "success";
                        response["uid"] = QString::fromStdString(uid);
                        response["email"] = QString::fromStdString(result.user.email());
                        
                        // Get account type
                        auto accountType = snapshot.Get("accountType");
                        if (accountType.is_string()) {
                            response["isHiringManager"] = (accountType.string_value() == "Hiring Manager");
                            response["accountType"] = QString::fromStdString(accountType.string_value());
                        }
                        
                        // Get username/name
                        auto username = snapshot.Get("username");
                        if (username.is_string()) {
                            response["name"] = QString::fromStdString(username.string_value());
                        }
                        
                        // Get description if it exists
                        auto description = snapshot.Get("description");
                        if (description.is_string()) {
                            response["description"] = QString::fromStdString(description.string_value());
                        } else {
                            response["description"] = "";
                        }
                        
                        // For hiring managers, get company info
                        if (accountType.is_string() && accountType.string_value() == "Hiring Manager") {
                            auto companyName = snapshot.Get("companyName");
                            if (companyName.is_string()) {
                                response["companyName"] = QString::fromStdString(companyName.string_value());
                            } else {
                                response["companyName"] = "";
                            }
                            
                            auto companyDesc = snapshot.Get("companyDescription");
                            if (companyDesc.is_string()) {
                                response["companyDescription"] = QString::fromStdString(companyDesc.string_value());
                            } else {
                                response["companyDescription"] = "";
                            }
                        }
                        
                        // For freelancers, get hourly rate
                        if (accountType.is_string() && accountType.string_value() == "Freelancer") {
                            auto hourlyRate = snapshot.Get("hourlyRate");
                            if (hourlyRate.is_double()) {
                                response["hourlyRate"] = hourlyRate.double_value();
                            } else {
                                response["hourlyRate"] = 0.0;
                            }
                        }
                    } else {
                        // User exists in Auth but not in Firestore
                        response["status"] = "error";
                        response["error"] = "User profile not found";
                    }
                }
                
                QJsonDocument responseDoc(response);
                sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
            });
        } });
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
    else if (requestType == "getProfile")
    {
        QString uid = jsonRequest["uid"].toString();

        // Get user reference
        firebase::firestore::DocumentReference userRef = firestore->Collection("users").Document(uid.toStdString());

        // Get the document
        auto future = userRef.Get();
        future.OnCompletion([this, clientSocket](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
        {
                QJsonObject response;

                if (future.error() != firebase::firestore::kErrorOk)
                {
                    response["status"] = "error";
                    response["error"] = future.error_message();
                }
                else
                {
                    firebase::firestore::DocumentSnapshot snapshot = *future.result();

                    if (snapshot.exists())
                    {
                        // Create a success response with all user data
                        response["status"] = "success";

                        // Get basic fields
                        auto name = snapshot.Get("username");
                        auto email = snapshot.Get("email");
                        auto description = snapshot.Get("description");

                        if (name.is_string())
                            response["name"] = QString::fromStdString(name.string_value());
                        if (email.is_string())
                            response["email"] = QString::fromStdString(email.string_value());
                        if (description.is_string())
                            response["description"] = QString::fromStdString(description.string_value());

                        // Get company fields for hiring manager
                        auto companyName = snapshot.Get("companyName");
                        auto companyDesc = snapshot.Get("companyDescription");

                        if (companyName.is_string())
                            response["companyName"] = QString::fromStdString(companyName.string_value());
                        if (companyDesc.is_string())
                            response["companyDescription"] = QString::fromStdString(companyDesc.string_value());

                        // Get arrays (tags, accomplishments)
                        // ... add code to extract these arrays from Firestore

                        // Get job history
                        // ... add code to extract job history from Firestore
                    }
                    else
                    {
                        response["status"] = "error";
                        response["error"] = "Profile not found";
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
    else if (requestType == "updateProfile")
    {
        std::cout << "Processing update profile request" << std::endl;
        QString uid = jsonRequest["uid"].toString();

        // Get user reference
        firebase::firestore::DocumentReference userRef = firestore->Collection("users").Document(uid.toStdString());

        // Create update data map
        firebase::firestore::MapFieldValue updateData;

        // Basic info
        if (jsonRequest.contains("description"))
        {
            updateData["description"] = firebase::firestore::FieldValue::String(
                jsonRequest["description"].toString().toStdString());
        }

        if (jsonRequest.contains("hourlyRate"))
        {
            updateData["hourlyRate"] = firebase::firestore::FieldValue::Double(
                jsonRequest["hourlyRate"].toDouble());
        }

        // Skills
        if (jsonRequest.contains("skills"))
        {
            QJsonArray skillsArray = jsonRequest["skills"].toArray();
            std::vector<firebase::firestore::FieldValue> skills;

            for (const auto &skill : skillsArray)
            {
                skills.push_back(firebase::firestore::FieldValue::String(
                    skill.toString().toStdString()));
            }

            updateData["skills"] = firebase::firestore::FieldValue::Array(skills);
        }

        // Tags
        if (jsonRequest.contains("tags"))
        {
            QJsonArray tagsArray = jsonRequest["tags"].toArray();
            std::vector<firebase::firestore::FieldValue> tags;

            for (const auto &tag : tagsArray)
            {
                tags.push_back(firebase::firestore::FieldValue::String(
                    tag.toString().toStdString()));
            }

            updateData["tags"] = firebase::firestore::FieldValue::Array(tags);
        }

        // Education
        if (jsonRequest.contains("education"))
        {
            QJsonObject educationObj = jsonRequest["education"].toObject();
            firebase::firestore::MapFieldValue education;

            education["jobTitle"] = firebase::firestore::FieldValue::String(
                educationObj["jobTitle"].toString().toStdString());
            education["startDate"] = firebase::firestore::FieldValue::String(
                educationObj["startDate"].toString().toStdString());
            education["endDate"] = firebase::firestore::FieldValue::String(
                educationObj["endDate"].toString().toStdString());
            education["description"] = firebase::firestore::FieldValue::String(
                educationObj["description"].toString().toStdString());

            updateData["education"] = firebase::firestore::FieldValue::Map(education);
        }

        // Accomplishments
        if (jsonRequest.contains("accomplishments"))
        {
            QJsonArray accomplishmentsArray = jsonRequest["accomplishments"].toArray();
            std::vector<firebase::firestore::FieldValue> accomplishments;

            for (const auto &acc : accomplishmentsArray)
            {
                accomplishments.push_back(firebase::firestore::FieldValue::String(
                    acc.toString().toStdString()));
            }

            updateData["accomplishments"] = firebase::firestore::FieldValue::Array(accomplishments);
        }

        // Job history
        if (jsonRequest.contains("jobHistory"))
        {
            QJsonArray jobHistoryArray = jsonRequest["jobHistory"].toArray();
            std::vector<firebase::firestore::FieldValue> jobHistory;

            for (const auto &jobValue : jobHistoryArray)
            {
                QJsonObject jobObj = jobValue.toObject();
                firebase::firestore::MapFieldValue job;

                job["jobTitle"] = firebase::firestore::FieldValue::String(
                    jobObj["jobTitle"].toString().toStdString());
                job["startDate"] = firebase::firestore::FieldValue::String(
                    jobObj["startDate"].toString().toStdString());
                job["endDate"] = firebase::firestore::FieldValue::String(
                    jobObj["endDate"].toString().toStdString());
                job["description"] = firebase::firestore::FieldValue::String(
                    jobObj["description"].toString().toStdString());

                jobHistory.push_back(firebase::firestore::FieldValue::Map(job));
            }

            updateData["jobHistory"] = firebase::firestore::FieldValue::Array(jobHistory);
        }

        // Update the document in Firestore
        auto future = userRef.Update(updateData);
        future.OnCompletion([this, clientSocket](const firebase::Future<void> &future)
                            {
            QJsonObject response;
            
            if (future.error() != firebase::firestore::kErrorOk) {
                std::cerr << "Error updating profile: " << future.error_message() << std::endl;
                response["status"] = "error";
                response["error"] = QString::fromStdString(future.error_message());
            } else {
                std::cout << "Profile updated successfully" << std::endl;
                response["status"] = "success";
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

void Server::sendResponse(int clientSocket, const std::string &response)
{
    // Add newline as message terminator
    std::string fullResponse = response + "\n";

    // Send response
    ssize_t bytesSent = send(clientSocket, fullResponse.c_str(), fullResponse.size(), 0);

    if (bytesSent < 0)
    {
        std::cerr << "Failed to send response: " << strerror(errno) << std::endl;
    }
    else if (static_cast<size_t>(bytesSent) < fullResponse.size())
    {
        std::cerr << "Warning: Not all data was sent" << std::endl;
    }
    else
    {
        std::cout << "Response sent successfully" << std::endl;
    }
}
