#pragma once
#include <QObject>
#include <QString>
#include <QtNetwork/QTcpSocket>
#include <functional>
#include <memory>
#include "../backend/server.h"
#include "User.h"
#include "Freelancer.h"
#include "HiringManager.h"
class BackendClient : public QObject
{
    Q_OBJECT

private:
    static BackendClient *instance;
    QTcpSocket *socket;
    bool connected;
    User *currentUser;
    // Private constructor for singleton
    explicit BackendClient(QObject *parent = nullptr);

public:
    // Get singleton instance
    static BackendClient *getInstance();

    // Connection methods
    bool connectToServer(const QString &host = "localhost", quint16 port = 8080);
    void disconnectFromServer();
    bool isConnected() const;

    // Authentication methods
    void signIn(const QString &email, const QString &password,
                std::function<void(User *, const QString &)> callback);

    void registerUser(const QString &email, const QString &password,
                      const QString &accountType, const QString &username,
                      std::function<void(firebase::auth::User *, const QString &)> callback);

    void signOut(std::function<void(bool)> callback);
    User *getCurrentUser() { return currentUser; }

    // User data methods
    void isHiringManager(const QString &uid, std::function<void(bool)> callback);

    // Helper for sending/receiving JSON data
    void sendRequest(const QJsonObject &request, std::function<void(const QJsonObject &)> callback);

signals:
    void connectionError(const QString &errorMessage);
    void serverResponseReceived(const QJsonObject &response);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
};