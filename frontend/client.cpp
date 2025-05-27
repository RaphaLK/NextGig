#include "client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>

// Initialize static instance
BackendClient *BackendClient::instance = nullptr;

BackendClient::BackendClient(QObject *parent) : QObject(parent), connected(false)
{
    socket = new QTcpSocket(this);

    // Connect socket signals
    connect(socket, &QTcpSocket::connected, this, &BackendClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &BackendClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &BackendClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &BackendClient::onError);
}

BackendClient *BackendClient::getInstance()
{
    if (!instance)
    {
        instance = new BackendClient();
    }
    return instance;
}

bool BackendClient::connectToServer(const QString &host, quint16 port)
{
    if (connected)
        return true;

    socket->connectToHost(host, port);
    return socket->waitForConnected(5000); // 5 second timeout
}

void BackendClient::disconnectFromServer()
{
    if (connected)
    {
        // Disconnect any pending signal connections to avoid memory leaks
        this->disconnect(this, &BackendClient::serverResponseReceived, nullptr, nullptr);

        // Close the socket connection
        socket->disconnectFromHost();

        // If the socket is still in a state where it's waiting for more data,
        // force it to close after a brief timeout
        if (socket->state() != QAbstractSocket::UnconnectedState)
        {
            if (!socket->waitForDisconnected(1000))
            {
                socket->abort(); // Force socket closed if it doesn't disconnect cleanly
            }
        }

        connected = false;
        qDebug() << "Client disconnected from server";
    }
}

bool BackendClient::isConnected() const
{
    return connected;
}

void BackendClient::registerUser(const QString &email, const QString &password,
                                 const QString &accountType, const QString &username,
                                 std::function<void(firebase::auth::User *, const QString &)> callback)
{
    QJsonObject request;
    request["type"] = "register";
    request["email"] = email;
    request["password"] = password;
    request["accountType"] = accountType;
    request["username"] = username;

    sendRequest(request, [callback](const QJsonObject &response)
                {
        if (response["status"].toString() == "success") {
            // In a real implementation, you'd convert JSON to User object
            callback(nullptr, "");
        } else {
            callback(nullptr, response["error"].toString());
        } });
}

void BackendClient::signIn(const QString &email, const QString &password,
                           std::function<void(User *, const QString &)> callback)
{
    QJsonObject request;
    request["type"] = "signin";
    request["email"] = email;
    request["password"] = password;

    sendRequest(request, [this, callback](const QJsonObject &response)
                {
        if (response["status"].toString() == "success") {
            // Extract user data from response
            QString uid = response["uid"].toString();
            QString email = response["email"].toString();
            QString name = response["name"].toString();
            QString description = response["description"].toString();
            bool isHiringManager = response["isHiringManager"].toBool();
            
            // Parse tags if available
            std::vector<std::string> tags;
            if (response.contains("tags") && response["tags"].isArray()) {
                QJsonArray tagsArray = response["tags"].toArray();
                for (const auto &tag : tagsArray) {
                    tags.push_back(tag.toString().toStdString());
                }
            }
            
            // Parse education if available
            User::education edu = {"", ""};
            if (response.contains("education") && response["education"].isObject()) {
                QJsonObject eduObj = response["education"].toObject();
                edu.school = eduObj["school"].toString().toStdString();
                edu.degreeLvl = eduObj["degree_lvl"].toString().toStdString();
            }
            
            // Parse accomplishments if available
            std::vector<std::string> accomplishments;
            if (response.contains("accomplishments") && response["accomplishments"].isArray()) {
                QJsonArray accArray = response["accomplishments"].toArray();
                for (const auto &acc : accArray) {
                    accomplishments.push_back(acc.toString().toStdString());
                }
            }
            
            // Parse job history if available
            std::vector<User::experience> jobHistory;
            if (response.contains("jobHistory") && response["jobHistory"].isArray()) {
                QJsonArray jobArray = response["jobHistory"].toArray();
                for (const auto &jobValue : jobArray) {
                    QJsonObject jobObj = jobValue.toObject();
                    User::experience job;
                    job.jobTitle = jobObj["jobTitle"].toString().toStdString();
                    job.startDate = jobObj["startDate"].toString().toStdString();
                    job.endDate = jobObj["endDate"].toString().toStdString();
                    job.description = jobObj["description"].toString().toStdString();
                    jobHistory.push_back(job);
                }
            }
            
            // Create the appropriate user object based on type
            if (isHiringManager) {
                QString companyName = response["companyName"].toString();
                QString companyDesc = response["companyDescription"].toString();
                
                currentUser = new HiringManager(
                    uid.toStdString(), email.toStdString(), name.toStdString(),
                    description.toStdString(), tags,
                    accomplishments, jobHistory,
                    companyName.toStdString(), companyDesc.toStdString()
                );
            } else {
                float hourlyRate = response["hourlyRate"].toDouble(0);
                
                currentUser = new Freelancer(
                    uid.toStdString(), email.toStdString(), name.toStdString(),
                    description.toStdString(), tags, 
                    accomplishments, jobHistory, edu, hourlyRate
                );
            }
            
            currentUser->setAuthStatus(true);
            callback(currentUser, "");
        } else {
            callback(nullptr, response["error"].toString());
        } });
}

void BackendClient::signOut(std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "signout";

    sendRequest(request, [this, callback](const QJsonObject &response)
                {
        bool success = (response["status"].toString() == "success");
        
        // Clean up the current user object
        if (currentUser) {
            delete currentUser; // Free memory
            currentUser = nullptr; // Reset pointer
        }
        
        callback(success); });
}

void BackendClient::isHiringManager(const QString &uid, std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "isHiringManager";
    request["uid"] = uid;

    sendRequest(request, [callback](const QJsonObject &response)
                {
        bool isManager = response["isHiringManager"].toBool();
        callback(isManager); });
}

void BackendClient::sendRequest(const QJsonObject &request, std::function<void(const QJsonObject &)> callback)
{
    if (!connected)
    {
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

    connect(this, &BackendClient::serverResponseReceived, this, [callback, this](const QJsonObject &response)
            {
        qDebug() << "Received response:" << response;
        callback(response);
        // Disconnect after handling this response
        disconnect(this, &BackendClient::serverResponseReceived, this, nullptr); });
}

void BackendClient::onConnected()
{
    connected = true;
    qDebug() << "Connected to backend server";
}

void BackendClient::onDisconnected()
{
    connected = false;
    qDebug() << "Disconnected from backend server";
}

void BackendClient::onError(QAbstractSocket::SocketError socketError)
{
    QString errorMsg = socket->errorString();
    qDebug() << "Socket error: " << errorMsg;
    emit connectionError(errorMsg);
}

void BackendClient::onReadyRead()
{
    // Read all available data
    QByteArray jsonData = socket->readAll();
    qDebug() << "Received data:" << jsonData;

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        qDebug() << "Invalid JSON received";
        return;
    }

    QJsonObject response = doc.object();
    emit serverResponseReceived(response);
}

void BackendClient::updateFreelancerProfile(Freelancer *freelancer, std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "updateProfile";
    request["uid"] = QString::fromStdString(freelancer->getUid());
    request["description"] = QString::fromStdString(freelancer->getDescription());
    request["hourlyRate"] = freelancer->getHourlyRate();

    // Add skills
    QJsonArray skillsArray;
    for (const auto &skill : freelancer->getSkills())
    {
        skillsArray.append(QString::fromStdString(skill));
    }
    request["skills"] = skillsArray;

    // Add tags
    QJsonArray tagsArray;
    for (const auto &tag : freelancer->getTags())
    {
        tagsArray.append(QString::fromStdString(tag));
    }
    request["tags"] = tagsArray;

    // Add education
    QJsonObject educationObj;
    User::education edu = freelancer->getEducation();
    educationObj["school"] = QString::fromStdString(edu.school);
    educationObj["degree_lvl"] = QString::fromStdString(edu.degreeLvl);
    request["education"] = educationObj;

    // Add accomplishments
    QJsonArray accomplishmentsArray;
    for (const auto &acc : freelancer->getAccomplishments())
    {
        accomplishmentsArray.append(QString::fromStdString(acc));
    }
    request["accomplishments"] = accomplishmentsArray;

    // Add job history
    QJsonArray jobHistoryArray;
    for (const auto &job : freelancer->getJobHistory())
    {
        QJsonObject jobObj;
        jobObj["jobTitle"] = QString::fromStdString(job.jobTitle);
        jobObj["startDate"] = QString::fromStdString(job.startDate);
        jobObj["endDate"] = QString::fromStdString(job.endDate);
        jobObj["description"] = QString::fromStdString(job.description);
        jobHistoryArray.append(jobObj);
    }
    request["jobHistory"] = jobHistoryArray;

    sendRequest(request, [callback](const QJsonObject &response)
                {
        bool success = (response["status"].toString() == "success");
        callback(success); });
}