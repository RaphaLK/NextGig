#include "client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>


// Initialize static instance
BackendClient* BackendClient::instance = nullptr;

BackendClient::BackendClient(QObject* parent) : QObject(parent), connected(false) {
    socket = new QTcpSocket(this);
    
    // Connect socket signals
    connect(socket, &QTcpSocket::connected, this, &BackendClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &BackendClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &BackendClient::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &BackendClient::onError);
}

BackendClient* BackendClient::getInstance() {
    if (!instance) {
        instance = new BackendClient();
    }
    return instance;
}

bool BackendClient::connectToServer(const QString& host, quint16 port) {
    if (connected) return true;
    
    socket->connectToHost(host, port);
    return socket->waitForConnected(5000); // 5 second timeout
}

void BackendClient::disconnect() {
    if (connected) {
        socket->disconnectFromHost();
    }
}

bool BackendClient::isConnected() const {
    return connected;
}

void BackendClient::signIn(const QString& email, const QString& password, 
                           std::function<void(firebase::auth::User*, const QString&)> callback) {
    QJsonObject request;
    request["type"] = "signin";
    request["email"] = email;
    request["password"] = password;
    
    sendRequest(request, [callback](const QJsonObject& response) {
        if (response["status"].toString() == "success") {
            // In a real implementation, you'd convert JSON to User object
            // For now, we'll pass nullptr as the user since we can't create Firebase objects
            callback(nullptr, "");
        } else {
            callback(nullptr, response["error"].toString());
        }
    });
}

void BackendClient::registerUser(const QString& email, const QString& password,
                                 const QString& accountType, const QString& username,
                                 std::function<void(firebase::auth::User*, const QString&)> callback) {
    QJsonObject request;
    request["type"] = "register";
    request["email"] = email;
    request["password"] = password;
    request["accountType"] = accountType;
    request["username"] = username;
    
    sendRequest(request, [callback](const QJsonObject& response) {
        if (response["status"].toString() == "success") {
            // In a real implementation, you'd convert JSON to User object
            callback(nullptr, "");
        } else {
            callback(nullptr, response["error"].toString());
        }
    });
}

void BackendClient::signOut(std::function<void(bool)> callback) {
    QJsonObject request;
    request["type"] = "signout";
    
    sendRequest(request, [callback](const QJsonObject& response) {
        bool success = (response["status"].toString() == "success");
        callback(success);
    });
}

void BackendClient::isHiringManager(const QString& uid, std::function<void(bool)> callback) {
    QJsonObject request;
    request["type"] = "isHiringManager";
    request["uid"] = uid;
    
    sendRequest(request, [callback](const QJsonObject& response) {
        bool isManager = response["isHiringManager"].toBool(false);
        callback(isManager);
    });
}

void BackendClient::sendRequest(const QJsonObject& request, std::function<void(const QJsonObject&)> callback) {
    if (!connected) {
        QJsonObject errorResponse;
        errorResponse["status"] = "error";
        errorResponse["error"] = "Not connected to server";
        callback(errorResponse);
        return;
    }
    
    // Convert request to JSON and send
    QJsonDocument doc(request);
    QByteArray jsonData = doc.toJson();
    
    // Add a size header to the message (4 bytes for message length)
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);
    
    stream << jsonData.size();
    message.append(jsonData);
    
    socket->write(message);
    
    // In a real implementation, you would track the callback and match with response
    // This is a simplified version
    
    // For now, we'll use a lambda that captures the callback to handle the response
    // This is demo code - you'd need a proper message ID system for production
    connect(this, &BackendClient::serverResponseReceived, [callback](const QJsonObject& response) {
        callback(response);
    });
}

void BackendClient::onConnected() {
    connected = true;
    qDebug() << "Connected to backend server";
}

void BackendClient::onDisconnected() {
    connected = false;
    qDebug() << "Disconnected from backend server";
}

void BackendClient::onError(QAbstractSocket::SocketError socketError) {
    QString errorMsg = socket->errorString();
    qDebug() << "Socket error: " << errorMsg;
    emit connectionError(errorMsg);
}

void BackendClient::onReadyRead() {
    // In a real implementation, you'd parse the incoming data properly
    // This is a simplified version
    
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    
    // Read the message size
    quint32 messageSize;
    if (socket->bytesAvailable() < sizeof(quint32))
        return;
        
    in >> messageSize;
    
    // Wait until we have the full message
    if (socket->bytesAvailable() < messageSize)
        return;
        
    // Read the JSON data
    QByteArray jsonData = socket->read(messageSize);
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject response = doc.object();
    
    // Emit signal with the response
    emit serverResponseReceived(response);
}