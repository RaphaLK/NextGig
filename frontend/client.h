#pragma once
#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QtNetwork/QTcpSocket>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include "../backend/server.h"
#include "User.h"
#include "Freelancer.h"
#include "HiringManager.h"
#include "Job.h"
#include "Proposal.h"

// Structure to represent a queued request
struct QueuedRequest {
    QJsonObject request;
    std::function<void(const QJsonObject &)> callback;
};

class BackendClient : public QObject
{
    Q_OBJECT

private:
    static BackendClient *instance;
    QTcpSocket *socket;
    bool connected;
    User *currentUser;
    
    // Request queue members
    std::queue<QueuedRequest> requestQueue;
    std::mutex queueMutex;
    bool requestInProgress;
    
    // Private constructor for singleton
    explicit BackendClient(QObject *parent = nullptr);
    
    // Process the next request in the queue
    void processNextRequest();

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
    
    // Queue-based sendRequest that replaces the old one
    void sendRequest(const QJsonObject &request, std::function<void(const QJsonObject &)> callback);
    User *getCurrentUser() { return currentUser; }
    // TODO: Implement
    void getProfile(const QString &uid, std::function<void(const QJsonObject &)> callback);
    // User data methods
    void isHiringManager(const QString &uid, std::function<void(bool)> callback);

    void updateFreelancerProfile(Freelancer *freelancer, std::function<void(bool)> callback);
    void updateHiringManagerProfile(HiringManager *hiringManager, std::function<void(bool)> callback);

    void addJob(Job &job, std::function<void(bool)> callback);
    void removeJob(const std::string &jobId, std::function<void(bool)> callback);
    void getJobs(std::function<void(bool, std::vector<Job>)> callback);
    void applyForJob(Job &job, Proposal &proposal, std::function<void(bool)> callback);

    void getProposals(const QString &employerId, std::function<void(bool, const QJsonArray &)> callback);
    void getApprovedJobs(const QString &freelancerId, std::function<void(bool, const QJsonArray &)> callback);
    void getAppliedJobs(const QString &freelancerId, std::function<void(bool, const QJsonArray &)> callback);
    void getHiringManagerProfile(const QString &employerId, std::function<void(bool, const QJsonArray &)> callback);
    void respondToProposal(const QString &jobId,
                                      const QString &freelancerId,
                                      bool accept,
                                      std::function<void(bool)> callback);
    void updateJobStatus(const QString &jobId,
                                    const QString &newStatus,
                                    std::function<void(bool)> callback);
    void updateCompletedJobs(const QString &userId,
                                        const QString &jobId,
                                        const QString &hiringManagerId,
                                        const QString &freelancerId,
                                        const QString &jobName,
                                        const QString &jobDescription,
                                        double budgetRequested,
                                        std::function<void(bool)> callback);
    void getCompletedJobs(const QString &userId,
                                     bool asHiringManager,
                                     std::function<void(bool, const QJsonArray &)> callback);
    void rateUser(const QString &fromUserId,
                                const QString &colleagueId,
                                int rating,
                                const QString &comment,
                                std::function<void(bool)> callback);
signals:
    void connectionError(const QString &errorMessage);
    void serverResponseReceived(const QJsonObject &response);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
};