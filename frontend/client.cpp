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
    connect(socket, &QTcpSocket::errorOccurred, this, &BackendClient::onError);

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
void BackendClient::disconnectFromServer() {
    if (connected) {
        // Disconnect any pending signal connections to avoid memory leaks
        this->disconnect(this, &BackendClient::serverResponseReceived, nullptr, nullptr);
        
        // Close the socket connection
        socket->disconnectFromHost();
        
        // If the socket is still in a state where it's waiting for more data,
        // force it to close after a brief timeout
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            if (!socket->waitForDisconnected(1000)) {
                socket->abort(); // Force socket closed if it doesn't disconnect cleanly
            }
        }
        
        connected = false;
        qDebug() << "Client disconnected from server";
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
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    qDebug() << "Sending request:" << doc.toJson();

    jsonData.append('\n');

    // Send the data
    socket->write(jsonData);
    socket->flush();
      
    connect(this, &BackendClient::serverResponseReceived, this, [callback, this](const QJsonObject& response) {
        qDebug() << "Received response:" << response;
        callback(response);
        // Disconnect after handling this response
        disconnect(this, &BackendClient::serverResponseReceived, this, nullptr);
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
    // Read all available data
    QByteArray jsonData = socket->readAll();
    qDebug() << "Received data:" << jsonData;
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON received";
        return;
    }
    
    QJsonObject response = doc.object();
    emit serverResponseReceived(response);
}