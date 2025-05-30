#include "client.h"
#include "UserManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>

// Initialize static instance
BackendClient *BackendClient::instance = nullptr;

BackendClient::BackendClient(QObject *parent) : QObject(parent), 
    socket(new QTcpSocket(this)), 
    connected(false), 
    currentUser(nullptr),
    requestInProgress(false) 
{
    socket = new QTcpSocket(this);

    // Connect socket signals
    connect(socket, &QTcpSocket::connected, this, &BackendClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &BackendClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &BackendClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &BackendClient::onError);
}

void BackendClient::sendRequest(const QJsonObject &request, std::function<void(const QJsonObject &)> callback)
{
    // Create a new request and add it to the queue
    QueuedRequest queuedRequest{request, callback};
    
    {
        // Lock the queue while modifying it
        std::lock_guard<std::mutex> lock(queueMutex);
        requestQueue.push(queuedRequest);
    }
    
    // If no request is in progress, start processing
    if (!requestInProgress) {
        processNextRequest();
    }
}

void BackendClient::processNextRequest()
{
    QueuedRequest currentRequest;
    
    {
        // Lock the queue while accessing it
        std::lock_guard<std::mutex> lock(queueMutex);
        
        // If queue is empty, we're done
        if (requestQueue.empty()) {
            requestInProgress = false;
            return;
        }
        
        // Get next request and mark as in progress
        currentRequest = requestQueue.front();
        requestQueue.pop();
        requestInProgress = true;
    }
    
    // Check connection first
    if (!isConnected()) {
        qDebug() << "Cannot send request: not connected to server";
        currentRequest.callback(QJsonObject{{"status", "error"}, {"message", "Not connected to server"}});
        
        // Process the next request
        processNextRequest();
        return;
    }
    
    // Connect to the response signal just for this request
    QMetaObject::Connection connection = connect(this, &BackendClient::serverResponseReceived,
            [this, callback = currentRequest.callback, connection = QMetaObject::Connection()](const QJsonObject &response) mutable {
                // Disconnect after receiving the response
                disconnect(connection);
                
                // Pass the response to the callback
                callback(response);
                
                // Process next request in the queue
                processNextRequest();
            });
    
    // Convert request to JSON
    QJsonDocument doc(currentRequest.request);
    QByteArray jsonData = doc.toJson();
    
    // Add size header (assuming the server expects it)
    QByteArray sizeHeader = QByteArray::number(jsonData.size());
    sizeHeader.append('\n');
    
    // Send the data
    socket->write(sizeHeader);
    socket->write(jsonData);
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
        qDebug() << "Client disconnected from server" << Qt::endl;
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

            User* user = nullptr;
            bool isHiringManager = response["isHiringManager"].toBool();
            
            if (isHiringManager) {
                HiringManager* hiringManager = new HiringManager();
                // Fill basic fields
                hiringManager->setUid(response["uid"].toString().toStdString());
                hiringManager->setEmail(response["email"].toString().toStdString());
                hiringManager->setName(response.value("username").toString().toStdString());
                
                // Fill specific fields if available
                if (response.contains("name")) {
                    hiringManager->setName(response["name"].toString().toStdString());
                }
                if (response.contains("description")) {
                    hiringManager->setDescription(response["description"].toString().toStdString());
                }
                if (response.contains("companyName")) {
                    hiringManager->setCompanyName(response["companyName"].toString().toStdString());
                }
                if (response.contains("companyDescription")) {
                    hiringManager->setCompanyDescription(response["companyDescription"].toString().toStdString());
                }
                
                // Parse arrays
                if (response.contains("tags") && response["tags"].isArray()) {
                    QJsonArray tagsArray = response["tags"].toArray();
                    for (const auto &tag : tagsArray) {
                        hiringManager->addTags(tag.toString().toStdString());
                    }
                }
                
                if (response.contains("accomplishments") && response["accomplishments"].isArray()) {
                    QJsonArray accArray = response["accomplishments"].toArray();
                    for (const auto &acc : accArray) {
                        hiringManager->addAccomplishment(acc.toString().toStdString());
                    }
                }
                
                if (response.contains("jobHistory") && response["jobHistory"].isArray()) {
                    QJsonArray jobArray = response["jobHistory"].toArray();
                    for (const auto &jobValue : jobArray) {
                        QJsonObject jobObj = jobValue.toObject();
                        hiringManager->addJobHistory(
                            jobObj["jobTitle"].toString().toStdString(),
                            jobObj["startDate"].toString().toStdString(),
                            jobObj["endDate"].toString().toStdString(),
                            jobObj["description"].toString().toStdString()
                        );
                    }
                }
                
                user = hiringManager;
            } else {
                // Create Freelancer object and populate similarly
                Freelancer* freelancer = new Freelancer();
                
                // MISSING CODE: You need to set UID and basic fields for Freelancer too!
                freelancer->setUid(response["uid"].toString().toStdString());
                freelancer->setEmail(response["email"].toString().toStdString());
                freelancer->setName(response.value("username").toString().toStdString());
                
                // Same field processing as for HiringManager for common fields
                if (response.contains("name")) {
                    freelancer->setName(response["name"].toString().toStdString());
                }
                if (response.contains("description")) {
                    freelancer->setDescription(response["description"].toString().toStdString());
                }
                
                // Parse Freelancer-specific fields
                if (response.contains("hourlyRate")) {
                    freelancer->setHourlyRate(response["hourlyRate"].toDouble());
                }
                
                // Parse education if available
                if (response.contains("education") && response["education"].isObject()) {
                    QJsonObject eduObj = response["education"].toObject();
                    User::education edu;
                    edu.school = eduObj["school"].toString().toStdString();
                    edu.degreeLvl = eduObj["degree_lvl"].toString().toStdString();
                    freelancer->setEducation(edu);
                }
                
                // Parse skills if available
                if (response.contains("skills") && response["skills"].isArray()) {
                    QJsonArray skillsArray = response["skills"].toArray();
                    for (const auto &skill : skillsArray) {
                        freelancer->addSkill(skill.toString().toStdString());
                    }
                }
                
                // Parse tags (same as for HiringManager)
                if (response.contains("tags") && response["tags"].isArray()) {
                    QJsonArray tagsArray = response["tags"].toArray();
                    for (const auto &tag : tagsArray) {
                        freelancer->addTags(tag.toString().toStdString());
                    }
                }
                
                // Parse accomplishments (same as for HiringManager)
                if (response.contains("accomplishments") && response["accomplishments"].isArray()) {
                    QJsonArray accArray = response["accomplishments"].toArray();
                    for (const auto &acc : accArray) {
                        freelancer->addAccomplishment(acc.toString().toStdString());
                    }
                }
                
                // Parse job history (same as for HiringManager)
                if (response.contains("jobHistory") && response["jobHistory"].isArray()) {
                    QJsonArray jobArray = response["jobHistory"].toArray();
                    for (const auto &jobValue : jobArray) {
                        QJsonObject jobObj = jobValue.toObject();
                        freelancer->addJobHistory(
                            jobObj["jobTitle"].toString().toStdString(),
                            jobObj["startDate"].toString().toStdString(),
                            jobObj["endDate"].toString().toStdString(),
                            jobObj["description"].toString().toStdString()
                        );
                    }
                }
                user = freelancer;
            }
            
            // Store the user in UserManager
            UserManager::getInstance()->setCurrentUser(user);
            
            // Call the callback with the populated user
            callback(user, "");
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
        currentUser = nullptr; // Reset pointer
                // Make sure UserManager also clears its references
        if (success) {
            UserManager::getInstance()->clearCurrentUser();
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

// void BackendClient::sendRequest(const QJsonObject &request, std::function<void(const QJsonObject &)> callback)
// {
//     if (!connected)
//     {
//         QJsonObject errorResponse;
//         errorResponse["status"] = "error";
//         errorResponse["error"] = "Not connected to server";
//         callback(errorResponse);
//         return;
//     }

//     // Convert request to JSON and send
//     QJsonDocument doc(request);
//     QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

//     qDebug() << "Sending request:" << doc.toJson() << Qt::endl;

//     jsonData.append('\n');

//     // Send the data
//     socket->write(jsonData);
//     socket->flush();

//     connect(this, &BackendClient::serverResponseReceived, this, [callback, this](const QJsonObject &response)
//             {
//         qDebug() << "Received response:" << response << Qt::endl;
//         callback(response);
//         // Disconnect after handling this response
//         disconnect(this, &BackendClient::serverResponseReceived, this, nullptr); });
// }

void BackendClient::onConnected()
{
    connected = true;
    qDebug() << "Connected to backend server" << Qt::endl;
}

void BackendClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    connected = false;
    
    // Reset the request in progress flag
    requestInProgress = false;
    
    // Clear the queue on disconnect
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::queue<QueuedRequest> empty;
        std::swap(requestQueue, empty);
    }
}

void BackendClient::onError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage = socket->errorString();
    qDebug() << "Socket error:" << errorMessage;
    emit connectionError(errorMessage);
    
    // In case of error, reset the requestInProgress flag and process next request
    requestInProgress = false;
    processNextRequest();
}

void BackendClient::onReadyRead()
{
    // Buffer for accumulated data
    static QByteArray buffer;
    
    // Append new data to buffer
    buffer.append(socket->readAll());
    qDebug() << "Buffer now contains:" << buffer.size() << "bytes";
    
    // Process complete JSON objects in the buffer
    int processedLength = 0;
    
    while (!buffer.isEmpty()) {
        // Try to find the end of a JSON object
        int endPos = buffer.indexOf('\n');
        if (endPos == -1) {
            // No complete response yet, wait for more data
            break;
        }
        
        // Extract one complete JSON object
        QByteArray jsonData = buffer.left(endPos).trimmed();
        
        // Remove the processed data from the buffer
        buffer.remove(0, endPos + 1);
        
        // Skip empty lines
        if (jsonData.isEmpty()) {
            continue;
        }
        
        qDebug() << "Processing JSON response:" << jsonData;
        
        // Parse JSON response
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        
        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "Invalid JSON received:" << parseError.errorString() << ", data:" << jsonData;
            continue;
        }
        
        QJsonObject response = doc.object();
        emit serverResponseReceived(response);
    }
}

void BackendClient::updateFreelancerProfile(Freelancer *freelancer, std::function<void(bool)> callback)
{
    if (!freelancer || freelancer->getUid().empty())
    {
        qDebug() << "Error: Cannot update profile with empty UID" << Qt::endl;
        if (callback)
        {
            callback(false);
        }
        return;
    }

    qDebug() << "Updating freelancer profile for UID:" << QString::fromStdString(freelancer->getUid()) << Qt::endl;

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

void BackendClient::updateHiringManagerProfile(HiringManager *hiringManager, std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "updateProfile";
    request["uid"] = QString::fromStdString(hiringManager->getUid());
    request["description"] = QString::fromStdString(hiringManager->getDescription());
    request["companyName"] = QString::fromStdString(hiringManager->getCompanyName());
    request["companyDescription"] = QString::fromStdString(hiringManager->getCompanyDescription());

    // Add tags
    QJsonArray tagsArray;
    for (const auto &tag : hiringManager->getTags())
    {
        tagsArray.append(QString::fromStdString(tag));
    }
    request["tags"] = tagsArray;

    // Add accomplishments
    QJsonArray accomplishmentsArray;
    for (const auto &acc : hiringManager->getAccomplishments())
    {
        accomplishmentsArray.append(QString::fromStdString(acc));
    }
    request["accomplishments"] = accomplishmentsArray;

    // Add job history
    QJsonArray jobHistoryArray;
    for (const auto &job : hiringManager->getJobHistory())
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

// void BackendClient::addJob(Job *Job, std::function<void(bool)> callback)
// {
//     QJsonObject request;
//     request["type"] = "addJob";
//     request["uid"] = QString::fromStdString(Job->getJobId());
//     request["date_created"] = QString::fromStdString(Job->getDateCreated());
//     request["pay"] = QString::fromStdString(Job->getPayment());
//     request["jobDescription"] = QString::fromStdString(Job->getJobDescription());
//     QJsonArray jobSkillsArray;
//     for (const auto &skill : Job->getRequiredSkills())
//     {
//         QJsonObject jobObj;
//         jobObj["skill"] = QString::fromStdString(skill);

//         jobSkillsArray.append(jobObj);
//     }
//     request["jobSkills"] = jobSkillsArray;}

// ADD JOB
void BackendClient::addJob(Job &job, std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "addJob";
    request["jobId"] = QString::fromStdString(job.getJobId());
    request["jobTitle"] = QString::fromStdString(job.getJobTitle());
    request["jobDescription"] = QString::fromStdString(job.getJobDescription());
    request["employerName"] = QString::fromStdString(job.getEmployer());
    request["dateCreated"] = QString::fromStdString(job.getDateCreated());
    request["expiryDate"] = QString::fromStdString(job.getExpiryDate());
    request["payment"] = QString::fromStdString(job.getPayment());

    QJsonArray skillsArray;
    for (const auto &skill : job.getRequiredSkills())
        skillsArray.append(QString::fromStdString(skill));
    request["requiredSkills"] = skillsArray;

    sendRequest(request, [callback](const QJsonObject &response)
                {
        bool success = (response["status"].toString() == "success");
        callback(success); });
}

// BackendClient.cpp

// —————————————————
// REMOVE JOB
// —————————————————
// Now takes a full Job object and uses job.getJobId() internally.
void BackendClient::removeJob(const std::string &jobId,
                              std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "deleteJob";
    request["jobId"] = QString::fromStdString(jobId);

    sendRequest(request,
                [callback](const QJsonObject &response)
                {
                    bool success = (response["status"].toString() == "success");
                    callback(success);
                });
}

// —————————————————
// GET ALL JOBS
// —————————————————
// Deserializes every field of your Job class.
void BackendClient::getJobs(std::function<void(bool, std::vector<Job>)> callback)
{
    QJsonObject request;
    request["type"] = "getJobs";
    qDebug() << "get Jobs backend request" << Qt::endl;
    sendRequest(request,
                [callback](const QJsonObject &response)
                {
                    bool success = (response["status"].toString() == "success");
                    std::vector<Job> jobs;

                    if (success)
                    {
                        QJsonArray arr = response["jobs"].toArray();
                        jobs.reserve(arr.size());

                        qDebug() << "Parsing jobs, count:" << arr.size() << Qt::endl;
                        for (auto v : arr)
                        {
                            QJsonObject o = v.toObject();

                            // Primitive fields
                            std::string jobId = o["jobId"].toString().toStdString();
                            std::string title = o["jobTitle"].toString().toStdString();
                            std::string desc = o["jobDescription"].toString().toStdString();
                            std::string employer = o["employerName"].toString().toStdString();
                            std::string created = o["dateCreated"].toString().toStdString();
                            std::string expiry = o["expiryDate"].toString("").toStdString();
                            std::string payment = o["payment"].toString().toStdString();

                            // Arrays
                            std::vector<std::string> skills;
                            for (auto s : o["requiredSkills"].toArray())
                                skills.push_back(s.toString().toStdString());

                            jobs.push_back(Job(
                                jobId,
                                title,
                                desc,
                                employer,
                                created,
                                expiry,
                                skills,
                                payment));
                        }
                        qDebug() << "Jobs parsed:" << jobs.size() << Qt::endl;
                    }
                    else
                    {
                        qDebug() << "Request failed - getJobs" << Qt::endl;
                    }

                    callback(success, jobs);
                });
}

// —————————————————
// APPLY FOR JOB
// —————————————————
// Extend Job document in Firebase with Proposal
void BackendClient::applyForJob(Job &job, Proposal &proposal, std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "applyForJob";
    qDebug() << "Apply Jobs backend request" << Qt::endl;
    request["jobId"] = QString::fromStdString(job.getJobId());
    request["AcceptedFreelancer"] = QString::fromStdString(job.getAcceptedFreelancerUid());
    request["proposalDescription"] = QString::fromStdString(proposal.getProposalText());
    request["budgetRequest"] = QString::fromStdString(proposal.getBudgetRequested());
    request["freelancerUid"] = QString::fromStdString(proposal.getUid());

    sendRequest(request,
                [callback](const QJsonObject &response)
                {
                    bool success = (response["status"].toString() == "success");
                    callback(success);
                });
}

/*************
 * GET ALL PROPOSALS
 * -------------------
 * Look for every job with a uid field that matches the employer's username. Then, retrieve the entire job back
 */
void BackendClient::getProposals(const QString &employerId, std::function<void(bool, const QJsonArray &)> callback)
{
    QJsonObject request;
    request["type"] = "getProposals";
    request["employerId"] = employerId;

    qDebug() << "Getting proposals for employer ID:" << employerId << Qt::endl;

    sendRequest(request,
                [callback](const QJsonObject &response)
                {
                    bool success = (response["status"].toString() == "success");

                    if (success && response.contains("proposals"))
                    {
                        QJsonArray proposals = response["proposals"].toArray();
                        qDebug() << "Retrieved" << proposals.size() << "proposals" << Qt::endl;
                        callback(true, proposals);
                    }
                    else
                    {
                        qDebug() << "Failed to get proposals:" << response["error"].toString() << Qt::endl;
                        callback(false, QJsonArray());
                    }
                });
}
/***************
 * GET FREELANCER PROFILE
 * --------------------------------
 * Retrieve the freelancer profile based on a UID
 */

/****
 * GET FREELANCER's APPLIED JOBS
 * --------
 * Given Freelancer UID -- Find jobs where that they are APPLIED for for will show in Proposal list
 *  -- show if rejected/pending
 * do not show if accepted
 * 
 * Applied jobs can be checked through the proposals array field in each job and the freelancer's UID needs to be matched in the proposal
 */

void BackendClient::getAppliedJobs(const QString &freelancerId, std::function<void(bool, const QJsonArray &)> callback)
{
    QJsonObject request;
    request["type"] = "getAppliedJobs";
    request["freelancerId"] = freelancerId;
    qDebug() << "Getting all Applied Jobs for:" << freelancerId << Qt::endl;

    sendRequest(request,
                [callback](const QJsonObject &response)
                {
                    bool success = (response["status"].toString() == "success");
                    
                    if (success && response.contains("appliedJobs"))
                    {
                        QJsonArray appliedJobs = response["appliedJobs"].toArray();
                        qDebug() << "Retrieved" << appliedJobs.size() << "applied jobs" << Qt::endl;
                        callback(true, appliedJobs);
                    }
                    else
                    {
                        qDebug() << "Failed to get applied jobs:" << response["error"].toString() << Qt::endl;
                        callback(false, QJsonArray());
                    }
                });
}
/******
 *
 *
 * GET FREELANCER's APPROVED JOBS
 * ---------
 * Given Freelancer UID -- Find jobs that they APPROVED for will be in proposal's approvedFreelancer
 */

void BackendClient::getApprovedJobs(const QString &freelancerId, std::function<void(bool, const QJsonArray &)> callback)
{
    QJsonObject request;
    request["type"] = "getApprovedJobs";
    request["freelancerId"] = freelancerId;
    qDebug() << "Getting all Approved Jobs for:" << freelancerId << Qt::endl;
    sendRequest(request,
            [callback](const QJsonObject &response)
            {
                qDebug() << "BOOM BOOM BOOM" << Qt::endl;
                bool success = (response["status"].toString() == "success");

                if (success && response.contains("approvedJobs"))
                {
                    QJsonArray approvedJobs = response["approvedJobs"].toArray();
                    qDebug() << "Retrieved" << approvedJobs.size() << "approvedJobs" << Qt::endl;
                    callback(true, approvedJobs);
                }
                else
                {
                    qDebug() << "Failed to get approved jobs:" << response["error"].toString() << Qt::endl;
                    callback(false, QJsonArray());
                }
            });

}

/**********************
 * GET HIRINGMANAGER PROFILE
 * ---------------------------
 * Retrieve the HiringManager profile based on UID
 */

void BackendClient::getHiringManagerProfile(const QString &employerId, std::function<void(bool, const QJsonArray &)> callback)
{
    QJsonObject request;
    request["type"]        = "getProfile";
    request["employerId"]  = employerId;

    qDebug() << "Getting HiringManagerProfile for employer ID:" << employerId << Qt::endl;

    sendRequest(request,
        [callback](const QJsonObject &response) {
            bool success = (response["status"].toString() == "success");

            if (!success) {
                qWarning() << "getProfile failed:" << response["error"].toString();
                callback(false, QJsonArray());
                return;
            }

            QJsonArray profileArray;
            profileArray.append(response);
            
            // Return the array to the caller
            callback(true, profileArray);
        });
}


/***********************
 * RESPOND TO PROPOSAL
 * ---------------------
 * Given a proposal of a certain JobId, set the approved freelancer and clear the proposals array
 * OR delete the proposal from a certain freelancer
 */

 void BackendClient::respondToProposal(const QString &jobId,
                                      const QString &freelancerId,
                                      bool accept,
                                      std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"]         = "updateProposalStatus";
    request["jobId"]        = jobId;
    request["freelancerId"] = freelancerId;
    request["status"]       = accept ? "accepted" : "rejected";

    sendRequest(request, [callback](const QJsonObject &resp) {
        bool ok = (resp["status"].toString() == "success");
        callback(ok);
    });
}

/*********************
 * UPDATE JOB STATUS
 * ---------------------------
 * HiringManager can mark a job as pending, in-progress, or complete
 * If complete, add this to the Freelancer's and Hiring Manager's "Completed Jobs" array
 * TODO: update the "Fetch ALL Jobs to fetch only pending jobs"
 */

void BackendClient::updateJobStatus(const QString &jobId,
                                    const QString &newStatus,
                                    std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "updateJobStatus";
    request["jobId"] = jobId;
    request["status"] = newStatus;

    sendRequest(request, [callback](const QJsonObject &resp)
                {
        bool ok = (resp["status"].toString() == "success");
        callback(ok); });
}

/*********************
 * UPDATE COMPLETED JOBS
 * ----------------------------
 * Update user document to add a Job to their Job Completion Array. Should have:
 * - JobId
 * - HiringManagerId
 * - FreelancerId
 * - Job Name
 * - Job Description
 * - Budget Requested
 */

void BackendClient::updateCompletedJobs(const QString &userId,
                                        const QString &jobId,
                                        const QString &hiringManagerId,
                                        const QString &freelancerId,
                                        const QString &jobName,
                                        const QString &jobDescription,
                                        double budgetRequested,
                                        std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "updateCompletedJobs";
    request["userId"] = userId;
    request["jobId"] = jobId;
    request["hiringManagerId"] = hiringManagerId;
    request["freelancerId"] = freelancerId;
    request["jobName"] = jobName;
    request["jobDescription"] = jobDescription;
    request["budgetRequested"] = budgetRequested;

    sendRequest(request, [callback](const QJsonObject &resp)
                {
        bool ok = (resp["status"].toString() == "success");
        callback(ok); });
}

/*********************
 * RETRIEVE COMPLETED JOBS
 * ----------------------------
 * Depending if the user is a Freelancer or HiringManager, retrieve the HiringManagerId or FreelancerId since we need
 * one type of user to rate the other. Name this under "ColleagueId". Return the entire array.
 */
void BackendClient::getCompletedJobs(const QString &userId,
                                     bool asHiringManager,
                                     std::function<void(bool, const QJsonArray &)> callback)
{
    QJsonObject request;
    request["type"] = "getCompletedJobs";
    if (asHiringManager)
        request["hiringManagerId"] = userId;
    else
        request["freelancerId"] = userId;

    sendRequest(request, [callback](const QJsonObject &resp)
                {
        bool success = (resp["status"].toString() == "success");
        QJsonArray arr;
        if (success && resp.contains("completedJobs") && resp["completedJobs"].isArray())
            arr = resp["completedJobs"].toArray();
        callback(success, arr); });
}

/************
 * RATE OTHER USER
 * ---------------------
 * Allows user to RATE another user through this.
 */

void BackendClient::rateUser(const QString &fromUserId,
                             const QString &colleagueId,
                             int rating,
                             const QString &comment,
                             std::function<void(bool)> callback)
{
    QJsonObject request;
    request["type"] = "rateUser";
    request["fromUserId"] = fromUserId;
    request["colleagueId"] = colleagueId;
    request["rating"] = rating;
    request["comment"] = comment;

    sendRequest(request, [callback](const QJsonObject &resp)
                {
        bool ok = (resp["status"].toString() == "success");
        callback(ok); });
}
