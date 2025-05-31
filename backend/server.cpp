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

            // Check for JSON based protocol - first line contains the size of the JSON object
            size_t newlinePos = requestBuffer.find('\n');
            while (newlinePos != std::string::npos)
            {
                std::string sizeStr = requestBuffer.substr(0, newlinePos);
                requestBuffer = requestBuffer.substr(newlinePos + 1);

                // Try to parse the size
                try
                {
                    int jsonSize = std::stoi(sizeStr);

                    // Check if we have enough data for the JSON object
                    if (static_cast<int>(requestBuffer.size()) >= jsonSize)
                    {
                        // Extract the JSON data
                        std::string jsonData = requestBuffer.substr(0, jsonSize);
                        requestBuffer = requestBuffer.substr(jsonSize);

                        // Process the request
                        processRequest(clientSocket, jsonData);
                    }
                    else
                    {
                        // Not enough data yet, break the loop and wait for more
                        break;
                    }
                }
                catch (const std::exception &e)
                {
                    // Invalid size format, discard this line
                    std::cerr << "Invalid size format: " << sizeStr << std::endl;
                }

                // Look for next size indicator
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
                                response["hourlyRate"] = "0";
                            }
                        }
                        auto skillsField = snapshot.Get("skills");
                        if (skillsField.is_array()) {
                            QJsonArray skillsArray;
                            for (const auto& skill : skillsField.array_value()) {
                                if (skill.is_string()) {
                                    skillsArray.append(QString::fromStdString(skill.string_value()));
                                }
                            }
                            response["skills"] = skillsArray;
                        } else {
                            response["skills"] = QJsonArray(); // Empty array if no skills found
                        }
                        
                        // Add education object to the response
                        auto educationField = snapshot.Get("education");
                        if (educationField.is_map()) {
                            QJsonObject educationObj;
                            auto eduMap = educationField.map_value();
                            
                            auto school = eduMap.find("school");
                            if (school != eduMap.end() && school->second.is_string()) {
                                educationObj["school"] = QString::fromStdString(school->second.string_value());
                            } else {
                                educationObj["school"] = "";
                            }
                            
                            auto degreeLvl = eduMap.find("degree_lvl");
                            if (degreeLvl != eduMap.end() && degreeLvl->second.is_string()) {
                                educationObj["degree_lvl"] = QString::fromStdString(degreeLvl->second.string_value());
                            } else {
                                educationObj["degree_lvl"] = "";
                            }
                            
                            response["education"] = educationObj;
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
        QString uid = jsonRequest["employerId"].toString();
        QString username = jsonRequest["employerId"].toString(); // Alternative lookup (for jobs)

        if (uid.isEmpty() && username.isEmpty())
        {
            QJsonObject errorResponse;
            errorResponse["status"] = "error";
            errorResponse["error"] = "getProfile requires either a uid or username";
            sendResponse(clientSocket, QJsonDocument(errorResponse).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        qDebug() << "Getting profile for UID:" << uid << "or Username:" << username;

        // Function to process profile data once found
        auto processProfile = [this, clientSocket](const firebase::firestore::DocumentSnapshot &snapshot, const QString &identifier)
        {
            QJsonObject response;

            if (snapshot.exists())
            {
                qDebug() << "Profile found for identifier:" << identifier;

                // Create a success response with all user data
                response["status"] = "success";
                response["uid"] = QString::fromStdString(snapshot.id()); // Always return the actual UID

                // Get basic fields
                auto username = snapshot.Get("username");
                auto email = snapshot.Get("email");
                auto description = snapshot.Get("description");
                auto accountType = snapshot.Get("accountType");

                if (username.is_string())
                {
                    response["name"] = QString::fromStdString(username.string_value());
                }
                else
                {
                    response["name"] = "No Name";
                }

                if (email.is_string())
                {
                    response["email"] = QString::fromStdString(email.string_value());
                }
                else
                {
                    response["email"] = "No Email";
                }

                if (description.is_string())
                {
                    response["description"] = QString::fromStdString(description.string_value());
                }
                else
                {
                    response["description"] = "No description available";
                }

                // Get company fields for hiring manager
                auto companyName = snapshot.Get("companyName");
                auto companyDesc = snapshot.Get("companyDescription");

                if (companyName.is_string())
                {
                    response["companyName"] = QString::fromStdString(companyName.string_value());
                }
                else
                {
                    response["companyName"] = "No Company";
                }

                qDebug() << "Successfully prepared profile response for identifier:" << identifier;
            }
            else
            {
                qDebug() << "Profile not found for identifier:" << identifier;
                response["status"] = "error";
                response["error"] = "Profile not found";
            }

            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
        };

        // Try UID lookup first if UID is provided
        if (!uid.isEmpty())
        {
            firebase::firestore::DocumentReference userRef = firestore->Collection("users").Document(uid.toStdString());

            auto future = userRef.Get();
            future.OnCompletion([this, clientSocket, uid, username, processProfile](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
                                {
            if (future.error() != firebase::firestore::kErrorOk)
            {
                qDebug() << "Firestore error getting profile for UID:" << uid << "Error:" << future.error_message();
                
                // If UID lookup failed and we have a username, try username lookup
                if (!username.isEmpty()) {
                    qDebug() << "UID lookup failed, attempting username lookup for:" << username;
                    searchByUsername(clientSocket, username, processProfile);
                    return;
                }
                
                // No fallback available, return error
                QJsonObject response;
                response["status"] = "error";
                response["error"] = QString::fromStdString(future.error_message());
                QJsonDocument responseDoc(response);
                sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
                return;
            }
            
            firebase::firestore::DocumentSnapshot snapshot = *future.result();
            
            if (!snapshot.exists() && !username.isEmpty()) {
                // UID document doesn't exist, try username fallback
                qDebug() << "UID document not found, attempting username lookup for:" << username;
                searchByUsername(clientSocket, username, processProfile);
                return;
            }
            
            // Process the result (either found or not found)
            processProfile(snapshot, uid); });
        }
        else
        {
            // Only username provided, search by username directly
            searchByUsername(clientSocket, username, processProfile);
        }
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
    // —————————————
    // ADD A JOB
    // —————————————
    else if (requestType == "addJob")
    {
        // pull fields out of the incoming JSON
        QString title = jsonRequest["jobTitle"].toString();
        QString desc = jsonRequest["jobDescription"].toString();
        QString pay = jsonRequest["payment"].toString();
        QString uid = jsonRequest["employerName"].toString();
        QJsonArray skillsArray = jsonRequest["requiredSkills"].toArray();

        // build the Firestore map
        firebase::firestore::MapFieldValue jobData;
        jobData["JobTitle"] = firebase::firestore::FieldValue::String(title.toStdString());
        jobData["jobDescription"] = firebase::firestore::FieldValue::String(desc.toStdString());
        jobData["pay"] = firebase::firestore::FieldValue::String(pay.toStdString());
        jobData["date_created"] = firebase::firestore::FieldValue::Timestamp(firebase::Timestamp::Now());
        jobData["uid"] = firebase::firestore::FieldValue::String(uid.toStdString());

        // convert QJsonArray → std::vector<FieldValue>
        std::vector<firebase::firestore::FieldValue> fvSkills;
        for (auto v : skillsArray)
        {
            if (v.isString())
            {
                fvSkills.push_back(
                    firebase::firestore::FieldValue::String(v.toString().toStdString()));
            }
        }
        jobData["requiredSkills"] =
            firebase::firestore::FieldValue::Array(std::move(fvSkills));

        // fire the add
        auto future = firestore
                          ->Collection("jobs")
                          .Add(std::move(jobData));

        future.OnCompletion(
            [this, clientSocket](const firebase::Future<firebase::firestore::DocumentReference> &f)
            {
                QJsonObject resp;
                if (f.error() == firebase::firestore::kErrorOk)
                {
                    resp["status"] = "success";
                    resp["jobId"] = QString::fromStdString(f.result()->id());
                }
                else
                {
                    resp["status"] = "error";
                    resp["error"] = QString::fromUtf8(f.error_message());
                }
                sendResponse(clientSocket,
                             QJsonDocument(resp)
                                 .toJson(QJsonDocument::Compact)
                                 .toStdString());
            });
    }
    // —————————————
    // GET ALL JOBS
    // —————————————
    else if (requestType == "getJobs")
    {
        // qDebug() << "Getting jobs";
        auto future = firestore->Collection("jobs").Get();
        future.OnCompletion(
            [this, clientSocket](const firebase::Future<firebase::firestore::QuerySnapshot> &f)
            {
                QJsonObject resp;
                if (f.error() != firebase::firestore::kErrorOk)
                {
                    resp["status"] = "error";
                    resp["error"] = QString::fromUtf8(f.error_message());
                }
                else
                {
                    resp["status"] = "success";
                    resp["type"] = "getJobs";
                    QJsonArray jobsArr;

                    for (auto const &doc : f.result()->documents())
                    {
                        QJsonObject jobObj;
                        jobObj["jobId"] = QString::fromStdString(doc.id());

                        auto data = doc.GetData();
                        jobObj["jobTitle"] = QString::fromStdString(data["JobTitle"].string_value());
                        jobObj["jobDescription"] = QString::fromStdString(data["jobDescription"].string_value());
                        jobObj["payment"] = QString::fromStdString(data["pay"].string_value());
                        jobObj["employerName"] = QString::fromStdString(data["uid"].string_value());

                        // requiredSkills (array)
                        QJsonArray skillsJson;
                        for (auto const &fv : data["requiredSkills"].array_value())
                        {
                            skillsJson.append(QString::fromStdString(fv.string_value()));
                        }
                        jobObj["requiredSkills"] = skillsJson;

                        if (data.find("date_created") != data.end() && data["date_created"].is_timestamp())
                        {
                            firebase::Timestamp timestamp = data["date_created"].timestamp_value();
                            time_t seconds = timestamp.seconds();
                            struct tm *timeinfo = localtime(&seconds);
                            char buffer[80];
                            strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
                            jobObj["dateCreated"] = QString::fromStdString(buffer);
                        }
                        else
                        {
                            jobObj["dateCreated"] = QString::fromStdString("2023-01-01"); // Default date
                        }

                        jobsArr.append(jobObj);
                    }

                    resp["jobs"] = jobsArr;
                }

                sendResponse(clientSocket,
                             QJsonDocument(resp)
                                 .toJson(QJsonDocument::Compact)
                                 .toStdString());
            });
    }
    // —————————————
    // DELETE A JOB
    // —————————————
    else if (requestType == "deleteJob")
    {
        QString jobId = jsonRequest["jobId"].toString();
        if (jobId.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "deleteJob requires a jobId";
            sendResponse(clientSocket,
                         QJsonDocument(err)
                             .toJson(QJsonDocument::Compact)
                             .toStdString());
        }
        else
        {
            auto future = firestore
                              ->Collection("jobs")
                              .Document(jobId.toStdString())
                              .Delete();

            future.OnCompletion(
                [this, clientSocket, jobId](const firebase::Future<void> &f)
                {
                    QJsonObject resp;
                    if (f.error() == firebase::firestore::kErrorOk)
                    {
                        resp["status"] = "success";
                        resp["deletedId"] = jobId;
                    }
                    else
                    {
                        resp["status"] = "error";
                        resp["error"] = QString::fromUtf8(f.error_message());
                    }
                    sendResponse(clientSocket,
                                 QJsonDocument(resp)
                                     .toJson(QJsonDocument::Compact)
                                     .toStdString());
                });
        }
    }
    else if (requestType == "updateProposalStatus")
    {
        QString jobId = jsonRequest["jobId"].toString();
        QString freelancerId = jsonRequest["freelancerId"].toString();
        QString status = jsonRequest["status"].toString();

        if (jobId.isEmpty() || freelancerId.isEmpty() || status.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "updateProposalStatus requires jobId, freelancerId, and status";
            sendResponse(clientSocket, QJsonDocument(err).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        // Reference to the job document
        firebase::firestore::DocumentReference jobRef = firestore->Collection("jobs").Document(jobId.toStdString());

        // Get the job document first
        auto getFuture = jobRef.Get();
        getFuture.OnCompletion([this, clientSocket, jobId, freelancerId, status](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
                               {
        QJsonObject response;
        
        if (future.error() != firebase::firestore::kErrorOk) {
            response["status"] = "error";
            response["error"] = QString::fromStdString(future.error_message());
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        firebase::firestore::DocumentSnapshot snapshot = *future.result();
        
        if (!snapshot.exists()) {
            response["status"] = "error";
            response["error"] = "Job not found";
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        // Update data for job document
        firebase::firestore::MapFieldValue updateData;
        
        // Get proposals array
        auto proposalsField = snapshot.Get("proposals");
        if (!proposalsField.is_array()) {
            response["status"] = "error";
            response["error"] = "No proposals found for this job";
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        std::vector<firebase::firestore::FieldValue> proposals;
        bool foundProposal = false;
        
        // Update the status of the matching proposal
        for (const auto& proposal : proposalsField.array_value()) {
            if (proposal.is_map()) {
                auto proposalMap = proposal.map_value();
                auto idIter = proposalMap.find("freelancerId");
                
                if (idIter != proposalMap.end() && 
                    idIter->second.is_string() && 
                    idIter->second.string_value() == freelancerId.toStdString()) {
                    
                    // Found the matching proposal, update its status
                    firebase::firestore::MapFieldValue updatedProposal = proposalMap;
                    updatedProposal["status"] = firebase::firestore::FieldValue::String(status.toStdString());
                    
                    proposals.push_back(firebase::firestore::FieldValue::Map(updatedProposal));
                    foundProposal = true;
                } else {
                    // If this proposal isn't being updated, copy it as is
                    proposals.push_back(proposal);
                }
            }
        }
        
        if (!foundProposal) {
            response["status"] = "error";
            response["error"] = "No proposal found for the specified freelancer";
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        // Update the proposals array
        updateData["proposals"] = firebase::firestore::FieldValue::Array(proposals);
        
        // If accepting the proposal, also update the approvedFreelancer field
        if (status == "accepted") {
            updateData["approvedFreelancer"] = firebase::firestore::FieldValue::String(freelancerId.toStdString());
        }
        
        // Update the job document
        auto updateFuture = firestore->Collection("jobs").Document(jobId.toStdString()).Update(updateData);
        updateFuture.OnCompletion([this, clientSocket](const firebase::Future<void> &future) {
            QJsonObject response;
            
            if (future.error() != firebase::firestore::kErrorOk) {
                response["status"] = "error";
                response["error"] = QString::fromStdString(future.error_message());
            } else {
                response["status"] = "success";
                response["message"] = "Proposal status updated successfully";
            }
            
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
        }); });
    }
    else if (requestType == "applyForJob")
    {
        QString jobId = jsonRequest["jobId"].toString();
        QString proposalDescription = jsonRequest["proposalDescription"].toString();
        QString budgetRequest = jsonRequest["budgetRequest"].toString();
        QString freelancerUid = jsonRequest["freelancerUid"].toString();
        QString acceptedFreelancerUid = jsonRequest["acceptedFreelancer"].toString();

        if (jobId.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "applyForJob requires a jobId";
            sendResponse(clientSocket,
                         QJsonDocument(err)
                             .toJson(QJsonDocument::Compact)
                             .toStdString());

            return;
        }
        // Reference to the job document
        firebase::firestore::DocumentReference jobRef =
            firestore->Collection("jobs").Document(jobId.toStdString());

        // Get the job document first to check if it exists and to get current proposals
        auto getFuture = jobRef.Get();
        getFuture.OnCompletion([this, clientSocket, jobId, proposalDescription, budgetRequest, freelancerUid, jobRef](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
                               {
        
        QJsonObject response;
        
        if (future.error() != firebase::firestore::kErrorOk) {
            response["status"] = "error";
            response["error"] = QString::fromStdString(future.error_message());
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        firebase::firestore::DocumentSnapshot snapshot = *future.result();
        
        if (!snapshot.exists()) {
            response["status"] = "error";
            response["error"] = "Job not found";
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        // Create a new proposal object
        firebase::firestore::MapFieldValue newProposal;
        newProposal["freelancerId"] = firebase::firestore::FieldValue::String(freelancerUid.toStdString());
        newProposal["description"] = firebase::firestore::FieldValue::String(proposalDescription.toStdString());
        newProposal["budgetRequest"] = firebase::firestore::FieldValue::String(budgetRequest.toStdString());
        newProposal["status"] = firebase::firestore::FieldValue::String("pending");
        newProposal["submittedAt"] = firebase::firestore::FieldValue::Timestamp(firebase::Timestamp::Now());
        
        // Update data for job document
        firebase::firestore::MapFieldValue updateData;
        
        // Check if the job already has proposals
        auto proposalsField = snapshot.Get("proposals");
        std::vector<firebase::firestore::FieldValue> proposals;
        
        if (proposalsField.is_array()) {
            // Copy existing proposals
            proposals = proposalsField.array_value();
            
            // Check if this freelancer has already applied
            bool alreadyApplied = false;
            for (const auto& proposal : proposals) {
                if (proposal.is_map()) {
                    auto proposalMap = proposal.map_value();
                    auto idIter = proposalMap.find("freelancerId");
                    if (idIter != proposalMap.end() && 
                        idIter->second.is_string() && 
                        idIter->second.string_value() == freelancerUid.toStdString()) {
                        alreadyApplied = true;
                        break;
                    }
                }
            }
            
            if (alreadyApplied) {
                response["status"] = "error";
                response["error"] = "You have already applied for this job";
                sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
                return;
            }
        }
        
        // Add the new proposal
        proposals.push_back(firebase::firestore::FieldValue::Map(newProposal));
        updateData["proposals"] = firebase::firestore::FieldValue::Array(proposals);
        
        // Add approved freelancer field if it doesn't exist
        auto approvedField = snapshot.Get("approvedFreelancer");
        if (!approvedField.is_string()) {
            updateData["approvedFreelancer"] = firebase::firestore::FieldValue::String("");
        }
        
        // Update the job document
        auto updateFuture = firestore->Collection("jobs").Document(jobId.toStdString()).Update(updateData);
        updateFuture.OnCompletion([this, clientSocket](const firebase::Future<void> &future) {
            QJsonObject response;
            
            if (future.error() != firebase::firestore::kErrorOk) {
                response["status"] = "error";
                response["error"] = QString::fromStdString(future.error_message());
            } else {
                response["status"] = "success";
                response["message"] = "Job application submitted successfully";
            }
            
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
        }); });
    }
    else if (requestType == "getAppliedJobs")
    {
        QString freelancerId = jsonRequest["freelancerId"].toString();
        if (freelancerId.isEmpty())
        {
            QJsonObject err{{"status", "error"}, {"error", "getAppliedJobs requires a freelancerId"}};
            sendResponse(clientSocket, QJsonDocument(err).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        qDebug() << "Getting applied jobs for freelancer:" << freelancerId;

        // Run Firestore query
        firestore->Collection("jobs")
            .Get()
            .OnCompletion([=](const firebase::Future<firebase::firestore::QuerySnapshot> &fut)
                          {
    QJsonObject response;
    QJsonArray appliedJobs;

    if (fut.error() != firebase::firestore::kErrorOk) {
        response["status"] = "error";
        response["error"] = QString::fromStdString(fut.error_message());
        sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
        return;
    }

    const auto *snapshot = fut.result();
    for (const auto &doc : snapshot->documents()) {
        QString jobId = QString::fromStdString(doc.id());
        auto data = doc.GetData();

        auto propIt = data.find("proposals");
        if (propIt == data.end() || !propIt->second.is_array()) continue;

        for (const auto &pv : propIt->second.array_value()) {
            if (!pv.is_map()) continue;
            const auto &propMap = pv.map_value();

            auto fidIt = propMap.find("freelancerId");
            if (fidIt == propMap.end() || !fidIt->second.is_string()) continue;

            if (fidIt->second.string_value() != freelancerId.toStdString()) continue;

            QJsonObject jobDetails;
            jobDetails["jobId"] = jobId;

            if (auto jt = data.find("JobTitle"); jt != data.end() && jt->second.is_string())
                jobDetails["jobTitle"] = QString::fromStdString(jt->second.string_value());

            if (auto jd = data.find("jobDescription"); jd != data.end() && jd->second.is_string())
                jobDetails["jobDescription"] = QString::fromStdString(jd->second.string_value());

            // Get requiredSkills array instead of pay
            if (auto skills = data.find("requiredSkills"); skills != data.end() && skills->second.is_array()) {
                QJsonArray skillsArray;
                for (const auto& skill : skills->second.array_value()) {
                    if (skill.is_string()) {
                        skillsArray.append(QString::fromStdString(skill.string_value()));
                    }
                }
                jobDetails["requiredSkills"] = skillsArray;
            } else {
                jobDetails["requiredSkills"] = QJsonArray(); // Empty array if no skills
            }

            if (auto uid = data.find("uid"); uid != data.end() && uid->second.is_string())
                jobDetails["employerId"] = QString::fromStdString(uid->second.string_value());

            if (auto desc = propMap.find("description"); desc != propMap.end() && desc->second.is_string())
                jobDetails["proposalDescription"] = QString::fromStdString(desc->second.string_value());

            // Get budgetRequest from the proposal instead of job payment
            if (auto bud = propMap.find("budgetRequest"); bud != propMap.end()) {
                if (bud->second.is_string())
                    jobDetails["budgetRequest"] = QString::fromStdString(bud->second.string_value());
                else if (bud->second.is_integer())
                    jobDetails["budgetRequest"] = static_cast<qint64>(bud->second.integer_value());
                else if (bud->second.is_double())
                    jobDetails["budgetRequest"] = bud->second.double_value();
            } else {
                jobDetails["budgetRequest"] = "Not specified";
            }

            QString status = "pending";
            if (auto st = propMap.find("status"); st != propMap.end() && st->second.is_string())
                status = QString::fromStdString(st->second.string_value());
            jobDetails["status"] = status;

            if (auto af = data.find("approvedFreelancer"); af != data.end() && af->second.is_string())
                jobDetails["approvedFreelancer"] = QString::fromStdString(af->second.string_value());

            appliedJobs.append(jobDetails);
            break;  // Done with this job
        }
    }

    response["status"] = "success";
    response["appliedJobs"] = appliedJobs;

    sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "getApprovedJobs")
    {
        QString freelancerId = jsonRequest["freelancerId"].toString();
        QJsonObject response;

        if (freelancerId.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "getApprovedJobs requires a freelancerId";
            sendResponse(clientSocket, QJsonDocument(err).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        auto jobsQuery = firestore->Collection("jobs").Get();

        qDebug() << "getApprovedJobs 1" << Qt::endl;

        jobsQuery.OnCompletion([this, clientSocket, freelancerId](const firebase::Future<firebase::firestore::QuerySnapshot> &future)
                               {
    QJsonObject response;
    QJsonArray approvedJobsArray;
    
    if (future.error() != firebase::firestore::kErrorOk) {
        response["status"] = "error";
        response["error"] = QString::fromStdString(future.error_message());
        sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
        return;
    }
    
    const firebase::firestore::QuerySnapshot* snapshot = future.result();
    
    // Loop through all jobs
    qDebug() << "getApprovedJobs 2" << Qt::endl;
    for (const auto& doc : snapshot->documents()) {
        QString jobId = QString::fromStdString(doc.id());
        auto data = doc.GetData();
        
        auto approvedFreelancer = data.find("approvedFreelancer");

        if (approvedFreelancer != data.end() && 
            approvedFreelancer->second.is_string() && 
            approvedFreelancer->second.string_value() == freelancerId.toStdString())
        {
            QJsonObject jobDetails; // New object for each job

            jobDetails["jobId"] = jobId;
            
            // Get job details
            if (data.find("JobTitle") != data.end() && data["JobTitle"].is_string()) {
                jobDetails["jobTitle"] = QString::fromStdString(data["JobTitle"].string_value());
            }
            
            if (data.find("jobDescription") != data.end() && data["jobDescription"].is_string()) {
                jobDetails["jobDescription"] = QString::fromStdString(data["jobDescription"].string_value());
            }
            
            // Get requiredSkills array instead of pay
            if (data.find("requiredSkills") != data.end() && data["requiredSkills"].is_array()) {
                QJsonArray skillsArray;
                for (const auto& skill : data["requiredSkills"].array_value()) {
                    if (skill.is_string()) {
                        skillsArray.append(QString::fromStdString(skill.string_value()));
                    }
                }
                jobDetails["requiredSkills"] = skillsArray;
            } else {
                jobDetails["requiredSkills"] = QJsonArray(); // Empty array if no skills
            }
            
            if (data.find("uid") != data.end() && data["uid"].is_string()) {
                jobDetails["employerId"] = QString::fromStdString(data["uid"].string_value());
            }
            
            // Find the budget request from this freelancer's proposal
            if (data.find("proposals") != data.end() && data["proposals"].is_array()) {
                for (const auto& proposal : data["proposals"].array_value()) {
                    if (proposal.is_map()) {
                        const auto& propMap = proposal.map_value();
                        auto fidIt = propMap.find("freelancerId");
                        
                        if (fidIt != propMap.end() && 
                            fidIt->second.is_string() && 
                            fidIt->second.string_value() == freelancerId.toStdString()) {
                            
                            // Found this freelancer's proposal, get the budget request
                            auto budgetIt = propMap.find("budgetRequest");
                            if (budgetIt != propMap.end()) {
                                if (budgetIt->second.is_string()) {
                                    jobDetails["budgetRequest"] = QString::fromStdString(budgetIt->second.string_value());
                                } else if (budgetIt->second.is_integer()) {
                                    jobDetails["budgetRequest"] = static_cast<qint64>(budgetIt->second.integer_value());
                                } else if (budgetIt->second.is_double()) {
                                    jobDetails["budgetRequest"] = budgetIt->second.double_value();
                                }
                            } else {
                                jobDetails["budgetRequest"] = "Not specified";
                            }
                            break; // Found the proposal, no need to continue
                        }
                    }
                }
            } else {
                jobDetails["budgetRequest"] = "Not specified";
            }
            
            approvedJobsArray.append(jobDetails);
        }
    }
    
    response["status"] = "success";
    response["approvedJobs"] = approvedJobsArray;
    
    qDebug() << "getApprovedJobs" << approvedJobsArray;
    // Set the final response object and send it - just ONCE
    QJsonDocument doc(response);
    sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "getProposals")
    {
        // NOTE: THIS IS THE EMPLOYER USERNAME
        QString employerId = jsonRequest["employerId"].toString();

        if (employerId.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "getProposals requires an employerId";
            sendResponse(clientSocket, QJsonDocument(err).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        qDebug() << "Getting proposals for employer:" << employerId;

        // Query jobs collection for all jobs where this employer is the owner
        auto jobsQuery = firestore->Collection("jobs").WhereEqualTo("uid", firebase::firestore::FieldValue::String(employerId.toStdString()));
        auto future = jobsQuery.Get();

        // Create shared pointers for data that needs to persist across callbacks
        auto sharedProposalData = std::make_shared<std::map<std::string, std::vector<QJsonObject>>>();
        auto sharedTotalFreelancers = std::make_shared<int>(0);
        auto sharedProcessedFreelancers = std::make_shared<int>(0);
        auto sharedAllProposals = std::make_shared<QJsonArray>();

        future.OnCompletion([this, clientSocket, employerId, sharedProposalData, sharedTotalFreelancers,
                             sharedProcessedFreelancers, sharedAllProposals](const firebase::Future<firebase::firestore::QuerySnapshot> &future)
                            {
        QJsonObject response;
        
        if (future.error() != firebase::firestore::kErrorOk) {
            response["status"] = "error";
            response["error"] = QString::fromStdString(future.error_message());
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        const firebase::firestore::QuerySnapshot* snapshot = future.result();
        
        // If no jobs found, return empty array
        if (snapshot->empty()) {
            response["status"] = "success";
            response["proposals"] = *sharedAllProposals;
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        // Create a shared final response object that will persist through callbacks
        auto sharedFinalResponse = std::make_shared<QJsonObject>();
        (*sharedFinalResponse)["status"] = "success";
        
        // First collect all proposals and count unique freelancers
        for (const auto& doc : snapshot->documents()) {
            QString jobId = QString::fromStdString(doc.id());
            auto data = doc.GetData();
            
            // Get job details
            QString jobTitle;
            if (data.find("JobTitle") != data.end() && data["JobTitle"].is_string()) {
                jobTitle = QString::fromStdString(data["JobTitle"].string_value());
            }
            
            QString jobDescription;
            if (data.find("jobDescription") != data.end() && data["jobDescription"].is_string()) {
                jobDescription = QString::fromStdString(data["jobDescription"].string_value());
            }
            
            QString payment;
            if (data.find("pay") != data.end() && data["pay"].is_string()) {
                payment = QString::fromStdString(data["pay"].string_value());
            }
            
            // Get proposals array
            auto proposalsField = data.find("proposals");
            if (proposalsField != data.end() && proposalsField->second.is_array()) {
                const auto& proposalsArray = proposalsField->second.array_value();
                
                // Process each proposal
                for (const auto& proposalValue : proposalsArray) {
                    if (proposalValue.is_map()) {
                        QJsonObject proposalObj;
                        const auto& proposalMap = proposalValue.map_value();
                        
                        // Add job details to each proposal
                        proposalObj["jobId"] = jobId;
                        proposalObj["jobTitle"] = jobTitle;
                        proposalObj["jobDescription"] = jobDescription;
                        proposalObj["payment"] = payment;
                        
                        // Add proposal-specific fields
                        std::string freelancerId;
                        auto freelancerIdField = proposalMap.find("freelancerId");
                        if (freelancerIdField != proposalMap.end() && freelancerIdField->second.is_string()) {
                            freelancerId = freelancerIdField->second.string_value();
                            proposalObj["freelancerId"] = QString::fromStdString(freelancerId);
                            
                            auto descriptionField = proposalMap.find("description");
                            if (descriptionField != proposalMap.end() && descriptionField->second.is_string()) {
                                proposalObj["proposalDescription"] = QString::fromStdString(descriptionField->second.string_value());
                            }
                            
                            auto budgetField = proposalMap.find("budgetRequest");
                            if (budgetField != proposalMap.end() && budgetField->second.is_string()) {
                                proposalObj["requestedBudget"] = QString::fromStdString(budgetField->second.string_value());
                            }
                            
                            auto statusField = proposalMap.find("status");
                            if (statusField != proposalMap.end() && statusField->second.is_string()) {
                                proposalObj["status"] = QString::fromStdString(statusField->second.string_value());
                            } else {
                                proposalObj["status"] = "pending";
                            }
                            
                            // Add this proposal to the pending list for this freelancer
                            if (!freelancerId.empty()) {
                                (*sharedProposalData)[freelancerId].push_back(proposalObj);
                                if ((*sharedProposalData)[freelancerId].size() == 1) {
                                    // Only count each freelancer once
                                    (*sharedTotalFreelancers)++;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // If no proposals collected, return empty array
        if (*sharedTotalFreelancers == 0) {
            response["status"] = "success";
            response["proposals"] = *sharedAllProposals;
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        // Now fetch freelancer details for each proposal
        for (const auto& [freelancerId, proposals] : *sharedProposalData) {
            fetchFreelancerDetails(freelancerId, [this, clientSocket, sharedAllProposals, 
                                                  sharedProcessedFreelancers, sharedTotalFreelancers, 
                                                  proposals, sharedFinalResponse]
                                                 (const QJsonObject &freelancerData, bool success) {
                // For each proposal from this freelancer, add the freelancer details
                for (const QJsonObject& proposalObj : proposals) {
                    QJsonObject completeProposal = proposalObj;
                    
                    if (success) {
                        // Add freelancer details
                        completeProposal["freelancerName"] = freelancerData["name"];
                        completeProposal["freelancerEmail"] = freelancerData["email"];
                        
                        // Format skills as comma-separated string
                        QJsonArray skills = freelancerData["skills"].toArray();
                        QStringList skillsList;
                        for (const auto& skill : skills) {
                            skillsList.append(skill.toString());
                        }
                        completeProposal["freelancerSkills"] = skillsList.join(", ");
                    } else {
                        // Use placeholder values if freelancer data couldn't be fetched
                        completeProposal["freelancerName"] = "Unknown User";
                        completeProposal["freelancerEmail"] = "No contact info";
                        completeProposal["freelancerSkills"] = "";
                    }
                    
                    sharedAllProposals->append(completeProposal);
                }
                
                (*sharedProcessedFreelancers)++;
                
                // When all freelancers have been processed, send the final response
                if (*sharedProcessedFreelancers >= *sharedTotalFreelancers) {
                    (*sharedFinalResponse)["proposals"] = *sharedAllProposals;
                    sendResponse(clientSocket, QJsonDocument(*sharedFinalResponse).toJson(QJsonDocument::Compact).toStdString());
                }
            });
        } });
    }
    else if (requestType == "updateJobStatus")
    {
        QString jobId = jsonRequest["jobId"].toString();
        QString newStatus = jsonRequest["status"].toString(); // "pending", "in-progress", "complete"
        QString freelancerId = jsonRequest["freelancerId"].toString();

        auto jobRef = firestore->Collection("jobs").Document(jobId.toStdString());

        firebase::firestore::MapFieldValue updates = {
            {"status", firebase::firestore::FieldValue::String(newStatus.toStdString())}};

        jobRef.Update(updates).OnCompletion([=](const firebase::Future<void> &f)
                                            {
            QJsonObject response;
            if (f.error() == firebase::firestore::kErrorOk) {
                response["status"] = "success";
            } else {
                response["status"] = "error";
                response["error"] = f.error_message();
            }
            QJsonDocument doc(response);
            sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "completeJob")
    {
        QString jobId = jsonRequest["jobId"].toString();
        QString hiringManagerId = jsonRequest["hiringManagerId"].toString();
        QString freelancerId = jsonRequest["freelancerId"].toString();
        QString jobTitle = jsonRequest["jobTitle"].toString();
        QString jobDescription = jsonRequest["jobDescription"].toString();
        double budgetRequested = jsonRequest["budgetRequested"].toDouble();

        if (jobId.isEmpty() || hiringManagerId.isEmpty() || freelancerId.isEmpty())
        {
            QJsonObject err;
            err["status"] = "error";
            err["error"] = "completeJob requires jobId, hiringManagerId, and freelancerId";
            sendResponse(clientSocket, QJsonDocument(err).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        qDebug() << "Completing job:" << jobId << "for HM:" << hiringManagerId << "and Freelancer:" << freelancerId;

        // First, get the job details before deleting it
        firebase::firestore::DocumentReference jobRef = firestore->Collection("jobs").Document(jobId.toStdString());

        auto getJobFuture = jobRef.Get();
        getJobFuture.OnCompletion([this, clientSocket, jobId, hiringManagerId, freelancerId, jobTitle, jobDescription, budgetRequested](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
                                  {
        
        if (future.error() != firebase::firestore::kErrorOk) {
            QJsonObject response;
            response["status"] = "error";
            response["error"] = QString::fromStdString(future.error_message());
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        firebase::firestore::DocumentSnapshot snapshot = *future.result();
        
        if (!snapshot.exists()) {
            QJsonObject response;
            response["status"] = "error";
            response["error"] = "Job not found";
            sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        // Create the completed job entry
        firebase::firestore::MapFieldValue completedJobEntry;
        completedJobEntry["jobId"] = firebase::firestore::FieldValue::String(jobId.toStdString());
        completedJobEntry["hiringManagerId"] = firebase::firestore::FieldValue::String(hiringManagerId.toStdString());
        completedJobEntry["freelancerId"] = firebase::firestore::FieldValue::String(freelancerId.toStdString());
        completedJobEntry["jobTitle"] = firebase::firestore::FieldValue::String(jobTitle.toStdString());
        completedJobEntry["jobDescription"] = firebase::firestore::FieldValue::String(jobDescription.toStdString());
        completedJobEntry["budgetRequested"] = firebase::firestore::FieldValue::Double(budgetRequested);
        completedJobEntry["completedAt"] = firebase::firestore::FieldValue::Timestamp(firebase::Timestamp::Now());

        // Counter for completed operations
        auto completionCounter = std::make_shared<int>(0);
        auto totalOperations = 3; // 2 user updates + 1 job deletion
        auto hasError = std::make_shared<bool>(false);
        auto errorMessage = std::make_shared<QString>();

        auto checkCompletion = [this, clientSocket, completionCounter, totalOperations, hasError, errorMessage]() {
            (*completionCounter)++;
            if (*completionCounter >= totalOperations) {
                QJsonObject response;
                if (*hasError) {
                    response["status"] = "error";
                    response["error"] = *errorMessage;
                } else {
                    response["status"] = "success";
                    response["message"] = "Job completed successfully";
                }
                sendResponse(clientSocket, QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString());
            }
        };

        // Add to hiring manager's completed jobs
        firebase::firestore::DocumentReference hmRef = firestore->Collection("users").Document(hiringManagerId.toStdString());
        hmRef.Update({{"completedJobs", firebase::firestore::FieldValue::ArrayUnion({firebase::firestore::FieldValue::Map(completedJobEntry)})}})
        .OnCompletion([checkCompletion, hasError, errorMessage](const firebase::Future<void> &future) {
            if (future.error() != firebase::firestore::kErrorOk) {
                *hasError = true;
                *errorMessage = QString("Failed to update hiring manager's completed jobs: ") + QString::fromStdString(future.error_message());
            }
            checkCompletion();
        });

        // Add to freelancer's completed jobs
        firebase::firestore::DocumentReference freelancerRef = firestore->Collection("users").Document(freelancerId.toStdString());
        freelancerRef.Update({{"completedJobs", firebase::firestore::FieldValue::ArrayUnion({firebase::firestore::FieldValue::Map(completedJobEntry)})}})
        .OnCompletion([checkCompletion, hasError, errorMessage](const firebase::Future<void> &future) {
            if (future.error() != firebase::firestore::kErrorOk) {
                *hasError = true;
                *errorMessage = QString("Failed to update freelancer's completed jobs: ") + QString::fromStdString(future.error_message());
            }
            checkCompletion();
        });

        // Delete the job from jobs collection
        firebase::firestore::DocumentReference jobRef = firestore->Collection("jobs").Document(jobId.toStdString());
        jobRef.Delete().OnCompletion([checkCompletion, hasError, errorMessage](const firebase::Future<void> &future) {
            if (future.error() != firebase::firestore::kErrorOk) {
                *hasError = true;
                *errorMessage = QString("Failed to delete job from active jobs: ") + QString::fromStdString(future.error_message());
            }
            checkCompletion();
        }); });
    }
    else if (requestType == "updateCompletedJobs")
    {
        QString userId = jsonRequest["userId"].toString();
        QString jobId = jsonRequest["jobId"].toString();
        QString hiringManagerId = jsonRequest["hiringManagerId"].toString();
        QString freelancerId = jsonRequest["freelancerId"].toString();
        QString jobName = jsonRequest["jobName"].toString();
        QString jobDescription = jsonRequest["jobDescription"].toString();
        double budget = jsonRequest["budgetRequested"].toDouble();

        QJsonObject jobEntry;
        jobEntry["jobId"] = jobId;
        jobEntry["hiringManagerId"] = hiringManagerId;
        jobEntry["freelancerId"] = freelancerId;
        jobEntry["jobName"] = jobName;
        jobEntry["jobDescription"] = jobDescription;
        jobEntry["budgetRequested"] = budget;

        firebase::firestore::DocumentReference userRef = firestore->Collection("users").Document(userId.toStdString());
        userRef.Update({{"completedJobs", firebase::firestore::FieldValue::ArrayUnion({firebase::firestore::FieldValue::Map({{"jobId", firebase::firestore::FieldValue::String(jobId.toStdString())},
                                                                                                                             {"hiringManagerId", firebase::firestore::FieldValue::String(hiringManagerId.toStdString())},
                                                                                                                             {"freelancerId", firebase::firestore::FieldValue::String(freelancerId.toStdString())},
                                                                                                                             {"jobName", firebase::firestore::FieldValue::String(jobName.toStdString())},
                                                                                                                             {"jobDescription", firebase::firestore::FieldValue::String(jobDescription.toStdString())},
                                                                                                                             {"budgetRequested", firebase::firestore::FieldValue::Double(budget)}})})}})
            .OnCompletion([=](const firebase::Future<void> &f)
                          {
            QJsonObject response;
            if (f.error() == firebase::firestore::kErrorOk) {
                response["status"] = "success";
            } else {
                response["status"] = "error";
                response["error"] = f.error_message();
            }
            QJsonDocument doc(response);
            sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "getCompletedJobs")
    {
        QString key;
        if (jsonRequest.contains("hiringManagerId"))
            key = "hiringManagerId";
        else if (jsonRequest.contains("freelancerId"))
            key = "freelancerId";
        else
        {
            QJsonObject response;
            response["status"] = "error";
            response["error"] = "Missing user ID";
            QJsonDocument doc(response);
            sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString());
            return;
        }

        QString uid = jsonRequest[key].toString();
        auto userRef = firestore->Collection("users").Document(uid.toStdString());
        userRef.Get().OnCompletion([=](const firebase::Future<firebase::firestore::DocumentSnapshot> &f)
                                   {
            QJsonObject response;
            if (f.error() == firebase::firestore::kErrorOk && f.result()->exists()) {
                auto doc = f.result()->GetData();
                QJsonArray jobArray;

                if (doc.count("completedJobs")) {
                    for (const auto &entry : doc["completedJobs"].array_value()) {
                        QJsonObject job;
                        for (const auto &[k, v] : entry.map_value()) {
                            if (v.is_string())
                                job[QString::fromStdString(k)] = QString::fromStdString(v.string_value());
                            else if (v.is_double())
                                job[QString::fromStdString(k)] = v.double_value();
                        }
                        jobArray.append(job);
                    }
                }

                response["status"] = "success";
                response["completedJobs"] = jobArray;
            } else {
                response["status"] = "error";
                response["error"] = f.error_message();
            }

            QJsonDocument doc(response);
            sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString()); });
    }
    else if (requestType == "rateUser")
    {
        QString fromUser = jsonRequest["fromUserId"].toString();
        QString colleague = jsonRequest["colleagueId"].toString();
        int rating = jsonRequest["rating"].toInt();
        QString comment = jsonRequest["comment"].toString();

        auto targetRef = firestore->Collection("users").Document(colleague.toStdString());

        firebase::firestore::MapFieldValue ratingEntry = {
            {"fromUserId", firebase::firestore::FieldValue::String(fromUser.toStdString())},
            {"rating", firebase::firestore::FieldValue::Integer(rating)},
            {"comment", firebase::firestore::FieldValue::String(comment.toStdString())},
            {"timestamp", firebase::firestore::FieldValue::Timestamp(firebase::Timestamp::Now())}};

        targetRef.Update({{"ratings", firebase::firestore::FieldValue::ArrayUnion({firebase::firestore::FieldValue::Map(ratingEntry)})}}).OnCompletion([=](const firebase::Future<void> &f)
                                                                                                                                                       {
            QJsonObject response;
            if (f.error() == firebase::firestore::kErrorOk) {
                response["status"] = "success";
            } else {
                response["status"] = "error";
                response["error"] = f.error_message();
            }
            QJsonDocument doc(response);
            sendResponse(clientSocket, doc.toJson(QJsonDocument::Compact).toStdString()); });
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

void Server::fetchFreelancerDetails(const std::string &freelancerId, std::function<void(const QJsonObject &, bool)> callback)
{
    firebase::firestore::DocumentReference userRef =
        firestore->Collection("users").Document(freelancerId);

    auto future = userRef.Get();
    future.OnCompletion([callback](const firebase::Future<firebase::firestore::DocumentSnapshot> &future)
                        {
        QJsonObject freelancerData;
        
        if (future.error() != firebase::firestore::kErrorOk) {
            qDebug() << "Error fetching freelancer details:" << future.error_message();
            callback(freelancerData, false);
            return;
        }
        
        firebase::firestore::DocumentSnapshot snapshot = *future.result();
        
        if (!snapshot.exists()) {
            qDebug() << "Freelancer document doesn't exist";
            callback(freelancerData, false);
            return;
        }
        
        // Extract the freelancer details
        auto username = snapshot.Get("username");
        if (username.is_string()) {
            freelancerData["name"] = QString::fromStdString(username.string_value());
        } else {
            freelancerData["name"] = "No Name";
        }
        
        auto email = snapshot.Get("email");
        if (email.is_string()) {
            freelancerData["email"] = QString::fromStdString(email.string_value());
        } else {
            freelancerData["email"] = "No Email";
        }
        
        // Get skills array
        QJsonArray skillsArray;
        auto skillsField = snapshot.Get("skills");
        if (skillsField.is_array()) {
            for (const auto& skill : skillsField.array_value()) {
                if (skill.is_string()) {
                    skillsArray.append(QString::fromStdString(skill.string_value()));
                }
            }
        }
        freelancerData["skills"] = skillsArray;
        
        callback(freelancerData, true); });
}

// Add this method to server.cpp:
void Server::searchByUsername(int clientSocket, const QString& username, 
                             std::function<void(const firebase::firestore::DocumentSnapshot&, const QString&)> processProfile)
{
    qDebug() << "Searching for user by username:" << username;
    
    // Query the users collection for documents where username field matches
    auto usernameQuery = firestore->Collection("users")
                                  .WhereEqualTo("username", firebase::firestore::FieldValue::String(username.toStdString()));
    
    auto future = usernameQuery.Get();
    future.OnCompletion([this, clientSocket, username, processProfile](const firebase::Future<firebase::firestore::QuerySnapshot> &future)
                        {
        if (future.error() != firebase::firestore::kErrorOk)
        {
            qDebug() << "Firestore error searching by username:" << username << "Error:" << future.error_message();
            QJsonObject response;
            response["status"] = "error";
            response["error"] = QString::fromStdString(future.error_message());
            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        const firebase::firestore::QuerySnapshot* snapshot = future.result();
        
        if (snapshot->empty()) {
            qDebug() << "No user found with username:" << username;
            QJsonObject response;
            response["status"] = "error";
            response["error"] = "Profile not found";
            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
            return;
        }
        
        if (snapshot->size() > 1) {
            qWarning() << "Multiple users found with username:" << username << "Using the first one.";
        }
        
        // Get the first matching document
        auto documents = snapshot->documents();
        if (!documents.empty()) {
            const firebase::firestore::DocumentSnapshot& userDoc = documents[0];
            qDebug() << "Found user by username:" << username << "UID:" << QString::fromStdString(userDoc.id());
            processProfile(userDoc, username);
        } else {
            qDebug() << "No documents in query result for username:" << username;
            QJsonObject response;
            response["status"] = "error";
            response["error"] = "Profile not found";
            QJsonDocument responseDoc(response);
            sendResponse(clientSocket, responseDoc.toJson(QJsonDocument::Compact).toStdString());
        }
    });
}