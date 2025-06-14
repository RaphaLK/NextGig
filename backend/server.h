#pragma once
#include <firebase/auth.h>
#include <firebase/firestore.h>
#include <firebase/app.h>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <arpa/inet.h>
#include <fcntl.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QDebug>
class Server
{
public:
    Server();
    ~Server() { stopServer(); };
    firebase::auth::User *signIn(const std::string &email, const std::string &password);
    firebase::auth::User *registerUser(const std::string &email, const std::string &password,
                                       const std::string &accountType, const std::string &username,
                                       std::function<void(firebase::auth::User *, const std::string &error)> callback);

    bool startServer(int port = 8080);
    void stopServer();
    bool getServerStatus() { return running; }

    void writeData(const std::string &path, const std::string &value);
    void signOut();

private:
    firebase::auth::Auth *auth;
    firebase::firestore::Firestore *firestore;

    // Socket server variables
    bool running;
    int serverSocket;
    std::thread serverThread;
    std::mutex clientsMutex;
    std::vector<int> clientSockets;

    // Socket server methods and helper functions
    void serverLoop();
    void handleClient(int clientSocket);
    void processRequest(int clientSocket, const std::string &request);
    void sendResponse(int clientSocket, const std::string &response);
    void fetchFreelancerDetails(const std::string &freelancerId,std::function<void(const QJsonObject &, bool)> callback);
    void searchByUsername(int clientSocket, const QString& username, 
                         std::function<void(const firebase::firestore::DocumentSnapshot&, const QString&)> processProfile);
};