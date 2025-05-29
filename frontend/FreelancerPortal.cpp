// Home page for the freelancer
#include "FreelancerPortal.h"
#include "client.h"
// #include "../src/models/User.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QTabWidget>
#include <QMessageBox>
#include "FreelancerProfileEdit.h"
#include "UserManager.h"
#include "JobFeed.h"
#include "Proposal.h"
#include <QTimer>

FreelancerPortal::FreelancerPortal(QWidget *parent) : QWidget(parent), currentUser(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(renderFreelancerPortal());
    setLayout(mainLayout);
    currentUser = UserManager::getInstance()->getCurrentUser();
    if (currentUser)
    {
        updateProfileInfo();
    }
}

void FreelancerPortal::setCurrentUser(User *user)
{
    currentUser = user;
    // Also update the UserManager
    UserManager::getInstance()->setCurrentUser(user);
    updateProfileInfo();
}

void FreelancerPortal::updateProfileInfo()
{
    if (!currentUser)
        return;

    Freelancer *currentUser = UserManager::getInstance()->getCurrentFreelancer();

    nameLabel->setText(QString::fromStdString(currentUser->getName()));
    descriptionTextEdit->setPlainText(QString::fromStdString(currentUser->getDescription()));

    // Update tags/skills
    skillsListWidget->clear();
    for (const auto &skill : currentUser->getSkills())
    {
        skillsListWidget->addItem(QString::fromStdString(skill));
    }

    // Update job history
    jobHistoryList->clear();
    for (const auto &job : currentUser->getJobHistory())
    {
        QString jobEntry = QString::fromStdString(job.jobTitle) + " (" +
                           QString::fromStdString(job.startDate) + " - " +
                           QString::fromStdString(job.endDate) + ")";
        jobHistoryList->addItem(jobEntry);
    }

    // Update accomplishments
    accomplishmentsList->clear();
    for (const auto &accomplishment : currentUser->getAccomplishments())
    {
        accomplishmentsList->addItem(QString::fromStdString(accomplishment));
    }
}

QWidget *FreelancerPortal::renderFreelancerPortal()
{
    QTabWidget *tabWidget = new QTabWidget();

    // Jobs Tab
    QWidget *jobsTab = createJobsTab();
    tabWidget->addTab(jobsTab, "Available Jobs");

    // Profile Tab
    QWidget *profileTab = createProfileTab();
    tabWidget->addTab(profileTab, "My Profile");

    // Messages Tab
    QWidget *messagesTab = createMessagesTab();
    tabWidget->addTab(messagesTab, "Current Jobs");

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Header with welcome and logout
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *welcomeLabel = new QLabel("Welcome, Freelancer!");
    QFont welcomeFont;
    welcomeFont.setPointSize(16);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);

    QPushButton *logoutBtn = new QPushButton("Logout");
    logoutBtn->setStyleSheet("padding: 8px; font-size: 13px; color: #fff; background: #d9534f;");
    connect(logoutBtn, &QPushButton::clicked, [this]()
            {
        BackendClient* client = BackendClient::getInstance();
        // Logout from server
        client->signOut([this](bool success) {
            if (success) {
                // Clear the current user in the UserManager
                UserManager::getInstance()->clearCurrentUser();
                currentUser = nullptr;
                emit returnToHomeRequested();
            } else {
                QMessageBox::warning(this, "Logout Error", 
                                     "Failed to log out. Please try again.");
            }
        }); });

    headerLayout->addWidget(welcomeLabel, 1);
    headerLayout->addWidget(logoutBtn, 0, Qt::AlignRight);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(tabWidget);

    QWidget *container = new QWidget();
    container->setLayout(mainLayout);
    return container;
}

QWidget *FreelancerPortal::createJobsTab()
{
    JobFeed *jobFeed = new JobFeed(this, true);
    return jobFeed;
}

QWidget *FreelancerPortal::createProfileTab()
{
    QWidget *profileWidget = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);

    // User info section
    QGroupBox *userInfoGroup = new QGroupBox("Personal Information");
    QVBoxLayout *userInfoLayout = new QVBoxLayout();

    // Profile header with name and rating
    QHBoxLayout *profileHeaderLayout = new QHBoxLayout();
    nameLabel = new QLabel("Your Name");
    nameLabel->setStyleSheet("font-weight: bold; font-size: 18px;");

    // Rating widget (simplified - would need a custom star rating widget in real app)
    QHBoxLayout *ratingLayout = new QHBoxLayout();
    QLabel *ratingLabel = new QLabel("Your Rating: ");
    QLabel *ratingStars = new QLabel("★★★★☆ 4.0");
    ratingStars->setStyleSheet("font-size: 16px; color: gold;");

    ratingLayout->addWidget(ratingLabel);
    ratingLayout->addWidget(ratingStars);
    ratingLayout->addStretch();

    profileHeaderLayout->addWidget(nameLabel);
    profileHeaderLayout->addLayout(ratingLayout);

    // Description
    QLabel *descriptionLabel = new QLabel("About Me:");
    descriptionTextEdit = new QTextEdit();
    descriptionTextEdit->setReadOnly(true);
    descriptionTextEdit->setMaximumHeight(100);

    userInfoLayout->addLayout(profileHeaderLayout);
    userInfoLayout->addWidget(descriptionLabel);
    userInfoLayout->addWidget(descriptionTextEdit);
    userInfoGroup->setLayout(userInfoLayout);

    // Skills section
    QGroupBox *skillsGroup = new QGroupBox("Skills & Expertise");
    QVBoxLayout *skillsLayout = new QVBoxLayout();

    skillsListWidget = new QListWidget();
    skillsListWidget->setMaximumHeight(120);

    skillsLayout->addWidget(skillsListWidget);
    skillsGroup->setLayout(skillsLayout);

    // Job history section
    QGroupBox *historyGroup = new QGroupBox("Job History");
    QVBoxLayout *historyLayout = new QVBoxLayout();

    jobHistoryList = new QListWidget();

    historyLayout->addWidget(jobHistoryList);
    historyGroup->setLayout(historyLayout);

    // Accomplishments section
    QGroupBox *accomplishmentsGroup = new QGroupBox("Accomplishments");
    QVBoxLayout *accomplishmentsLayout = new QVBoxLayout();

    accomplishmentsList = new QListWidget();

    accomplishmentsLayout->addWidget(accomplishmentsList);
    accomplishmentsGroup->setLayout(accomplishmentsLayout);

    // Edit profile button
    QPushButton *editProfileBtn = new QPushButton("Edit Profile");
    editProfileBtn->setStyleSheet("padding: 8px; background-color: #6c757d; color: white;");
    connect(editProfileBtn, &QPushButton::clicked, [this]()
            {
    UserManager* userManager = UserManager::getInstance();
    
    // Debug info
    qDebug() << "Edit Profile clicked";
    qDebug() << "UserManager says user is logged in:" << userManager->isUserLoggedIn();
    
    if (currentUser) {
        qDebug() << "FreelancerPortal has currentUser:" << QString::fromStdString(currentUser->getName());
        qDebug() << "currentUser is Freelancer:" << (dynamic_cast<Freelancer*>(currentUser) != nullptr);
    } else {
        qDebug() << "FreelancerPortal currentUser is null";
    }
    
    if (!userManager->isUserLoggedIn()) {
        QMessageBox::warning(this, "Not Logged In", "You need to be logged in to edit your profile.");
        return;
    }
    
    // Try using local currentUser if UserManager's user is not valid
    Freelancer* freelancer = userManager->getCurrentFreelancer();
    if (!freelancer && currentUser) {
        freelancer = dynamic_cast<Freelancer*>(currentUser);
        if (freelancer) {
            // Fix the UserManager if it's out of sync
            userManager->setCurrentUser(freelancer);
        }
    }
    
    if (!freelancer) {
        QMessageBox::warning(this, "Invalid User Type", "Only freelancer accounts can edit this profile.");
        return;
    }
    
    FreelancerProfileEdit* dialog = new FreelancerProfileEdit(freelancer, this);
    
    connect(dialog, &FreelancerProfileEdit::profileUpdated, [this]() {
        updateProfileInfo(); // Refresh the UI with updated info
    });
    
    dialog->exec();
    delete dialog; });
    // Arrange everything in the profile tab
    profileLayout->addWidget(userInfoGroup);
    profileLayout->addWidget(skillsGroup);
    profileLayout->addWidget(historyGroup);
    profileLayout->addWidget(accomplishmentsGroup);
    profileLayout->addWidget(editProfileBtn, 0, Qt::AlignRight);

    return profileWidget;
}

QWidget *FreelancerPortal::createMessagesTab()
{
    QWidget *messagesWidget = new QWidget();
    QGridLayout *messagesLayout = new QGridLayout(messagesWidget);

    // 1. Top Left: Jobs Applied For
    QGroupBox *appliedJobsGroup = new QGroupBox("Jobs Applied For");
    appliedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *appliedJobsLayout = new QVBoxLayout();

    appliedJobsList = new QListWidget();
    
    // Add placeholder while loading
    appliedJobsList->addItem("Loading applications...");
    
    appliedJobsLayout->addWidget(appliedJobsList);
    appliedJobsGroup->setLayout(appliedJobsLayout);

    // 2. Top Right: Approved Jobs
    QGroupBox *approvedJobsGroup = new QGroupBox("Approved Applications");
    approvedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *approvedJobsLayout = new QVBoxLayout();

    QListWidget *approvedJobsList = new QListWidget();
    approvedJobsList->addItem("Loading approved jobs...");
    approvedJobsList->setStyleSheet("QListWidget::item { padding: 8px; color: #28a745; }");

    approvedJobsLayout->addWidget(approvedJobsList);
    approvedJobsGroup->setLayout(approvedJobsLayout);

    // 3. Bottom Left: Job Information + Proposal
    QGroupBox *jobInfoGroup = new QGroupBox("Job Information & Your Proposal");
    jobInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *jobInfoLayout = new QVBoxLayout();

    // Job title and details
    QLabel *jobTitleLabel = new QLabel("Job Title: Not Selected");
    jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");

    QLabel *jobDescLabel = new QLabel("Job Description:");
    jobDescLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    QTextEdit *jobDescText = new QTextEdit();
    jobDescText->setReadOnly(true);
    jobDescText->setMaximumHeight(100);
    jobDescText->setStyleSheet("background-color: #f8f9fa; border-radius: 4px;");

    QLabel *paymentLabel = new QLabel("Payment Offer: Not Selected");
    paymentLabel->setStyleSheet("font-weight: bold; color: #28a745;");

    // Proposal section
    QLabel *yourProposalLabel = new QLabel("Your Proposal:");
    yourProposalLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    QTextEdit *proposalText = new QTextEdit();
    proposalText->setReadOnly(true);
    proposalText->setStyleSheet("background-color: #f8f9fa; border-radius: 4px;");

    QLabel *requestedAmountLabel = new QLabel("Your Requested Payment: Not Available");
    requestedAmountLabel->setStyleSheet("font-weight: bold; color: #17a2b8;");

    QLabel *statusLabel = new QLabel("Status: Pending");
    statusLabel->setStyleSheet("font-weight: bold; color: #ffc107; margin-top: 10px;");

    jobInfoLayout->addWidget(jobTitleLabel);
    jobInfoLayout->addWidget(jobDescLabel);
    jobInfoLayout->addWidget(jobDescText);
    jobInfoLayout->addWidget(paymentLabel);
    jobInfoLayout->addWidget(yourProposalLabel);
    jobInfoLayout->addWidget(proposalText);
    jobInfoLayout->addWidget(requestedAmountLabel);
    jobInfoLayout->addWidget(statusLabel);

    jobInfoGroup->setLayout(jobInfoLayout);

    // 4. Bottom Right: Employer Details
    QGroupBox *employerGroup = new QGroupBox("Employer Details");
    employerGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *employerLayout = new QVBoxLayout();

    QLabel *employerNameLabel = new QLabel("Company: Not Selected");
    employerNameLabel->setStyleSheet("font-weight: bold; font-size: 15px;");

    QLabel *contactLabel = new QLabel("Contact:");
    contactLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    QLabel *emailLabel = new QLabel("Email: Not Available");

    QLabel *companyInfoLabel = new QLabel("Company Profile:");
    companyInfoLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    QTextEdit *companyInfoText = new QTextEdit();
    companyInfoText->setReadOnly(true);
    companyInfoText->setMaximumHeight(150);
    companyInfoText->setStyleSheet("background-color: #f8f9fa; border-radius: 4px;");

    QLabel *ratingLabel = new QLabel("Employer Rating: ★★★★☆ (4.2)");
    ratingLabel->setStyleSheet("color: #f39c12; margin-top: 10px;");

    // Message employer button
    QPushButton *messageBtn = new QPushButton("Message Employer");
    messageBtn->setStyleSheet("background-color: #007bff; color: white; padding: 8px; border-radius: 4px;");

    employerLayout->addWidget(employerNameLabel);
    employerLayout->addWidget(contactLabel);
    employerLayout->addWidget(emailLabel);
    employerLayout->addWidget(companyInfoLabel);
    employerLayout->addWidget(companyInfoText);
    employerLayout->addWidget(ratingLabel);
    employerLayout->addWidget(messageBtn);
    employerLayout->addStretch();

    employerGroup->setLayout(employerLayout);

    // Add all widgets to the grid layout
    messagesLayout->addWidget(appliedJobsGroup, 0, 0);
    messagesLayout->addWidget(approvedJobsGroup, 0, 1);
    messagesLayout->addWidget(jobInfoGroup, 1, 0);
    messagesLayout->addWidget(employerGroup, 1, 1);

    // Set column and row stretching
    messagesLayout->setColumnStretch(0, 1);
    messagesLayout->setColumnStretch(1, 1);
    messagesLayout->setRowStretch(0, 1);
    messagesLayout->setRowStretch(1, 2); // Give more space to bottom row
    messagesLayout->setRowStretch(0, 1);
    messagesLayout->setRowStretch(1, 2); // Give more space to bottom row

    // Load data AFTER UI is completely set up - this prevents segfault
    QTimer::singleShot(100, this, [this, approvedJobsList, jobTitleLabel, jobDescText, paymentLabel, 
                                   proposalText, requestedAmountLabel, statusLabel, 
                                   employerNameLabel, emailLabel, companyInfoText]() {
        try {
            BackendClient *client = BackendClient::getInstance();
            if (!client) {
                qDebug() << "Error: BackendClient instance is null in timer lambda.";
                if (appliedJobsList) appliedJobsList->addItem("Error: Cannot connect to backend.");
                return;
            }
            
            User *userFromBackend = client->getCurrentUser();
            User* userToQuery = nullptr;

            qDebug() << "FreelancerPortal::createMessagesTab timer: Portal currentUser:" << this->currentUser;

            // Determine the correct user pointer to use

            userToQuery = this->currentUser; 

            // Final check on the chosen userToQuery
            if (userToQuery == nullptr) {
                qDebug() << "Error: Could not determine a valid user to query after checks. Aborting data load.";
                 if (appliedJobsList) {
                    appliedJobsList->clear();
                    appliedJobsList->addItem("Session error. Please log in again.");
                }
                if (approvedJobsList) {
                    approvedJobsList->clear();
                    approvedJobsList->addItem("Session error. Please log in again.");
                }
                return;
            }
            
            std::string userIdStr;
            try {
                qDebug() << "FreelancerPortal::createMessagesTab timer: Attempting to get UID for user:" << userToQuery;
                userIdStr = userToQuery->getUid(); 
                if (userIdStr.empty()) {
                    qDebug() << "Error: User UID is empty after retrieval in timer lambda for user:" << userToQuery;
                    if (appliedJobsList) {
                        appliedJobsList->clear();
                        appliedJobsList->addItem("Invalid user data (empty UID). Please log in again.");
                    }
                    return;
                }
            } catch (const std::exception& e) {
                qDebug() << "Exception when getting user UID in timer lambda for user:" << userToQuery << "Error:" << e.what();
                if (appliedJobsList) {
                    appliedJobsList->clear();
                    appliedJobsList->addItem("Error accessing user data. Please log in again.");
                }
                return;
            } catch (...) {
                qDebug() << "Unknown exception when getting user UID in timer lambda for user:" << userToQuery;
                if (appliedJobsList) {
                    appliedJobsList->clear();
// ...existing code...
                    appliedJobsList->addItem("Unknown error accessing user data. Please log in again.");
                }
                return;
            }
            
            QString freelancerId = QString::fromStdString(userIdStr);
            
            // Ensure client and appliedJobsList are valid before use in nested lambdas
            if (client && appliedJobsList) {
                client->getAppliedJobs(freelancerId, [this](bool success, const QJsonArray &jobsArray) { // Capture appliedJobsList
                    if (!this->currentUser) { 
                        qDebug() << "getAppliedJobs callback: Portal user logged out. Not updating UI.";
                        return;
                    }
                    if (success) {
                        appliedJobsList->clear();
                        if (jobsArray.isEmpty()) {
                            appliedJobsList->addItem("No applications submitted yet");
                        } else {
                            for (const QJsonValue& jobValue : jobsArray) {
                                QJsonObject job = jobValue.toObject();
                                QString jobTitle = job["jobTitle"].toString();
                                QString employer = job["employerId"].toString();
                                
                                QString displayText = jobTitle + " at " + employer;
                                QListWidgetItem* item = new QListWidgetItem(displayText);
                                item->setData(Qt::UserRole, QVariant(job));
                                appliedJobsList->addItem(item);
                            }
                        }
                    } else {
                        appliedJobsList->clear();
                        appliedJobsList->addItem("Failed to load applied jobs");
                    }
                });
            }
            
            // Load approved jobs (ensure client and approvedJobsList are valid)
            if (client && approvedJobsList) {
                client->getApprovedJobs(freelancerId, [this, approvedJobsList](bool success, const QJsonArray &jobsArray) {
                     if (!this->currentUser) { 
                        qDebug() << "getApprovedJobs callback: Portal user logged out. Not updating UI.";
                        return;
                    }
                    if (success) {
                        approvedJobsList->clear();
                        if (jobsArray.isEmpty()) {
                            approvedJobsList->addItem("No approved jobs yet");
                        } else {
                            for (const QJsonValue& jobValue : jobsArray) {
                                QJsonObject job = jobValue.toObject();
                                QString jobTitle = job["jobTitle"].toString();
                                QString employer = job["employerId"].toString();
                                QString displayText = "✓ " + jobTitle + " at " + employer;
                                QListWidgetItem* item = new QListWidgetItem(displayText);
                                item->setData(Qt::UserRole, QVariant(job));
                                approvedJobsList->addItem(item);
                            }
                        }
                    } else {
                        approvedJobsList->clear();
                        approvedJobsList->addItem("Failed to load approved jobs");
                    }
                });
            }
            
        } catch (const std::exception& e) {
            qDebug() << "Outer exception in createMessagesTab timer callback:" << e.what();
            if (appliedJobsList) {
                appliedJobsList->clear();
                appliedJobsList->addItem("Error loading data. Please try again.");
            }
        } catch (...) {
            qDebug() << "Unknown outer exception in createMessagesTab timer callback";
            if (appliedJobsList) {
                appliedJobsList->clear();
                appliedJobsList->addItem("Unknown error loading data. Please try again.");
            }
        }
    });

    return messagesWidget;
}