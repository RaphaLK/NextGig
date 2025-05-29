#include "HiringManagerPortal.h"
#include "client.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QSpacerItem>
#include "HiringManagerProfileEdit.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include "UserManager.h"
#include "JobFeed.h"
#include <QTimer>

HiringManagerPortal::HiringManagerPortal(QWidget *parent) : QWidget(parent), currentUser(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(renderHiringManagerPortal());
    setLayout(mainLayout);

    // Get the current user from UserManager
    currentUser = UserManager::getInstance()->getCurrentUser();

    // Debug logging
    if (currentUser)
    {
        qDebug() << "HiringManagerPortal: Current user set to" << QString::fromStdString(currentUser->getName());
        fetchProfileFromFirebase();
        updateProfileInfo();
    }
    else
    {
        qDebug() << "HiringManagerPortal: No user set in constructor";
    }
}

void HiringManagerPortal::setCurrentUser(User *user)
{
    currentUser = user;
    UserManager::getInstance()->setCurrentUser(user);
    updateProfileInfo();

    if (currentUser)
    {
        fetchProfileFromFirebase();
    }
}

void HiringManagerPortal::updateProfileInfo()
{
    if (!currentUser)
    {
        qDebug() << "updateProfileInfo: currentUser is null";
        // Try to get the current user from UserManager as a fallback
        currentUser = UserManager::getInstance()->getCurrentUser();

        if (!currentUser)
        {
            qDebug() << "Could not retrieve user from UserManager either";
            return;
        }
        qDebug() << "Retrieved user from UserManager: " << QString::fromStdString(currentUser->getName());
    }

    try
    {
        // Debug the current user data
        qDebug() << "Updating profile UI with user data:";
        qDebug() << "  Name:" << QString::fromStdString(currentUser->getName());
        qDebug() << "  Email:" << QString::fromStdString(currentUser->getEmail());
        qDebug() << "  Description:" << QString::fromStdString(currentUser->getDescription());

        // Update profile info in the profile tab
        if (nameLabel && nameLabel != nullptr)
            nameLabel->setText(QString::fromStdString(currentUser->getName()));
        if (emailLabel && emailLabel != nullptr)
            emailLabel->setText(QString::fromStdString(currentUser->getEmail()));
        if (descriptionTextEdit && descriptionTextEdit != nullptr)
            descriptionTextEdit->setPlainText(QString::fromStdString(currentUser->getDescription()));

        qDebug() << "Successful UpdateProfileInfo HiringManager";
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in updateProfileInfo:" << e.what();
    }
    catch (...)
    {
        qDebug() << "Unknown exception in updateProfileInfo";
    }
}

QWidget *HiringManagerPortal::renderHiringManagerPortal()
{
    QTabWidget *tabWidget = new QTabWidget();

    // Dashboard Tab (Jobs Management)
    QWidget *dashboardTab = createDashboardTab();
    tabWidget->addTab(dashboardTab, "Dashboard");

    // Profile Tab
    QWidget *profileTab = createProfileTab();
    tabWidget->addTab(profileTab, "My Profile");

    // Post Job Tab
    QWidget *postJobTab = createPostJobTab();
    tabWidget->addTab(postJobTab, "Post New Job");

    // Messages Tab
    QWidget *messagesTab = createMessagesTab();
    tabWidget->addTab(messagesTab, "Inbox");

    // Ratings Tab
    QWidget *ratingsTab = createRatingsTab();
    tabWidget->addTab(ratingsTab, "Ratings & Reviews");

    // Main Layout with header
    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Header with welcome and logout
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *welcomeLabel = new QLabel("Welcome, Hiring Manager!");
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

QWidget *HiringManagerPortal::createDashboardTab()
{
    QWidget *dashboardWidget = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardWidget);

    // Jobs status summary
    QGroupBox *summaryGroup = new QGroupBox("Jobs Summary");
    QHBoxLayout *summaryLayout = new QHBoxLayout();

    // Active Jobs
    QVBoxLayout *activeJobsLayout = new QVBoxLayout();
    QLabel *activeJobsLabel = new QLabel("Active Jobs");
    activeJobsLabel->setStyleSheet("font-weight: bold;");
    QLabel *activeJobsCount = new QLabel("0");
    activeJobsCount->setStyleSheet("font-size: 24px; color: #007bff;");
    activeJobsLayout->addWidget(activeJobsLabel, 0, Qt::AlignCenter);
    activeJobsLayout->addWidget(activeJobsCount, 0, Qt::AlignCenter);

    // Pending Proposals
    QVBoxLayout *pendingProposalsLayout = new QVBoxLayout();
    QLabel *pendingProposalsLabel = new QLabel("Pending Proposals");
    pendingProposalsLabel->setStyleSheet("font-weight: bold;");
    QLabel *pendingProposalsCount = new QLabel("0");
    pendingProposalsCount->setStyleSheet("font-size: 24px; color: #ffc107;");
    pendingProposalsLayout->addWidget(pendingProposalsLabel, 0, Qt::AlignCenter);
    pendingProposalsLayout->addWidget(pendingProposalsCount, 0, Qt::AlignCenter);

    // Completed Jobs
    QVBoxLayout *completedJobsLayout = new QVBoxLayout();
    QLabel *completedJobsLabel = new QLabel("Completed Jobs");
    completedJobsLabel->setStyleSheet("font-weight: bold;");
    QLabel *completedJobsCount = new QLabel("0");
    completedJobsCount->setStyleSheet("font-size: 24px; color: #28a745;");
    completedJobsLayout->addWidget(completedJobsLabel, 0, Qt::AlignCenter);
    completedJobsLayout->addWidget(completedJobsCount, 0, Qt::AlignCenter);

    summaryLayout->addLayout(activeJobsLayout);
    summaryLayout->addLayout(pendingProposalsLayout);
    summaryLayout->addLayout(completedJobsLayout);
    summaryGroup->setLayout(summaryLayout);

    // Split view for proposals
    QHBoxLayout *proposalsLayout = new QHBoxLayout();

    // Left side: List of job proposals
    QGroupBox *jobsGroup = new QGroupBox("Incoming Job Proposals");
    QVBoxLayout *jobsLayout = new QVBoxLayout();

    incomingPrpsls = new QListWidget();
    incomingPrpsls->setStyleSheet("QListWidget::item { padding: 8px; border-bottom: 1px solid #eaeaea; }");

    // Add a refresh button
    QPushButton *refreshProposalsBtn = new QPushButton("Refresh Proposals");
    refreshProposalsBtn->setStyleSheet("background-color: #6c757d; color: white; padding: 5px;");

    // Connect refresh button
    connect(refreshProposalsBtn, &QPushButton::clicked, [this]()
            { loadProposalsForHiringManager(); });

    jobsLayout->addWidget(refreshProposalsBtn, 0, Qt::AlignRight);
    jobsLayout->addWidget(incomingPrpsls);

    // Action buttons
    QHBoxLayout *jobActionsLayout = new QHBoxLayout();
    QPushButton *viewDetailsBtn = new QPushButton("View Details");
    viewDetailsBtn->setStyleSheet("background-color: #17a2b8; color: white;");
    QPushButton *acceptPrpslBtn = new QPushButton("Accept Proposal");
    acceptPrpslBtn->setStyleSheet("background-color: #28a745; color: white;");
    QPushButton *rejectPrpslBtn = new QPushButton("Reject Proposal");
    rejectPrpslBtn->setStyleSheet("background-color: #dc3545; color: white;");

    jobActionsLayout->addWidget(viewDetailsBtn);
    jobActionsLayout->addWidget(acceptPrpslBtn);
    jobActionsLayout->addWidget(rejectPrpslBtn);

    // Initially disable action buttons until a proposal is selected
    viewDetailsBtn->setEnabled(false);
    acceptPrpslBtn->setEnabled(false);
    rejectPrpslBtn->setEnabled(false);

    jobsLayout->addLayout(jobActionsLayout);
    jobsGroup->setLayout(jobsLayout);

    // Right side: Proposal details view
    QGroupBox *proposalsGroup = new QGroupBox("Proposal Details");
    QVBoxLayout *proposalDetailsLayout = new QVBoxLayout();

    // Job details section
    QGroupBox *jobDetailsSection = new QGroupBox("Job Information");
    QFormLayout *jobDetailsForm = new QFormLayout();

    QLabel *jobTitleLabel = new QLabel("No job selected");
    jobTitleLabel->setStyleSheet("font-weight: bold;");
    QLabel *jobDescLabel = new QLabel("Select a proposal to view details");
    jobDescLabel->setWordWrap(true);
    QLabel *jobPaymentLabel = new QLabel("");

    jobDetailsForm->addRow("Job Title:", jobTitleLabel);
    jobDetailsForm->addRow("Description:", jobDescLabel);
    jobDetailsForm->addRow("Payment Offered:", jobPaymentLabel);
    jobDetailsSection->setLayout(jobDetailsForm);

    // Freelancer profile section
    QGroupBox *freelancerSection = new QGroupBox("Freelancer Profile");
    QFormLayout *freelancerForm = new QFormLayout();

    QLabel *freelancerNameLabel = new QLabel("");
    QLabel *freelancerEmailLabel = new QLabel("");
    QLabel *freelancerSkillsLabel = new QLabel("");
    freelancerSkillsLabel->setWordWrap(true);

    freelancerForm->addRow("Name:", freelancerNameLabel);
    freelancerForm->addRow("Email:", freelancerEmailLabel);
    freelancerForm->addRow("Skills:", freelancerSkillsLabel);
    freelancerSection->setLayout(freelancerForm);

    // Proposal content section
    QGroupBox *proposalContentSection = new QGroupBox("Proposal");
    QVBoxLayout *proposalContentLayout = new QVBoxLayout();

    QTextEdit *proposalText = new QTextEdit();
    proposalText->setReadOnly(true);
    proposalText->setPlaceholderText("The freelancer's proposal will appear here");
    proposalText->setStyleSheet("background-color: #f8f9fa; border-radius: 4px;");
    proposalText->setMinimumHeight(150);

    QLabel *requestedBudgetLabel = new QLabel("Requested Budget: Not specified");
    requestedBudgetLabel->setStyleSheet("font-weight: bold; color: #28a745;");

    QLabel *proposalStatusLabel = new QLabel("Status: Pending");
    proposalStatusLabel->setStyleSheet("font-weight: bold; color: #ffc107;");

    proposalContentLayout->addWidget(proposalText);
    proposalContentLayout->addWidget(requestedBudgetLabel);
    proposalContentLayout->addWidget(proposalStatusLabel);
    proposalContentSection->setLayout(proposalContentLayout);

    // Add all sections to the proposal details layout
    proposalDetailsLayout->addWidget(jobDetailsSection);
    proposalDetailsLayout->addWidget(freelancerSection);
    proposalDetailsLayout->addWidget(proposalContentSection);

    proposalsGroup->setLayout(proposalDetailsLayout);

    // Add both panels to the split layout
    proposalsLayout->addWidget(jobsGroup, 1);
    proposalsLayout->addWidget(proposalsGroup, 2);

    // Connect list widget selection to details view
    connect(incomingPrpsls, &QListWidget::currentItemChanged,
            [=](QListWidgetItem *current, QListWidgetItem *previous)
            {
                if (current)
                {
                    // Enable action buttons
                    viewDetailsBtn->setEnabled(true);
                    acceptPrpslBtn->setEnabled(true);
                    rejectPrpslBtn->setEnabled(true);

                    // Get proposal data from the item's data role
                    QVariantMap proposalData = current->data(Qt::UserRole).toMap();

                    // Update job details
                    jobTitleLabel->setText(proposalData["jobTitle"].toString());
                    jobDescLabel->setText(proposalData["jobDescription"].toString());
                    jobPaymentLabel->setText("$" + proposalData["payment"].toString());

                    // Update freelancer details
                    freelancerNameLabel->setText(proposalData["freelancerName"].toString());
                    freelancerEmailLabel->setText(proposalData["freelancerEmail"].toString());
                    freelancerSkillsLabel->setText(proposalData["freelancerSkills"].toString());

                    // Update proposal content
                    proposalText->setText(proposalData["proposalDescription"].toString());
                    requestedBudgetLabel->setText("Requested Budget: " + proposalData["requestedBudget"].toString());

                    QString status = proposalData["status"].toString();
                    if (status == "pending")
                    {
                        proposalStatusLabel->setText("Status: Pending");
                        proposalStatusLabel->setStyleSheet("font-weight: bold; color: #ffc107;");
                        acceptPrpslBtn->setEnabled(true);
                        rejectPrpslBtn->setEnabled(true);
                    }
                    else if (status == "accepted")
                    {
                        proposalStatusLabel->setText("Status: Accepted");
                        proposalStatusLabel->setStyleSheet("font-weight: bold; color: #28a745;");
                        acceptPrpslBtn->setEnabled(false);
                        rejectPrpslBtn->setEnabled(true);
                    }
                    else if (status == "rejected")
                    {
                        proposalStatusLabel->setText("Status: Rejected");
                        proposalStatusLabel->setStyleSheet("font-weight: bold; color: #dc3545;");
                        acceptPrpslBtn->setEnabled(true);
                        rejectPrpslBtn->setEnabled(false);
                    }
                }
                else
                {
                    // Disable action buttons if no selection
                    viewDetailsBtn->setEnabled(false);
                    acceptPrpslBtn->setEnabled(false);
                    rejectPrpslBtn->setEnabled(false);
                }
            });

    // Connect action buttons
    connect(viewDetailsBtn, &QPushButton::clicked, [=]()
            {
        if (incomingPrpsls->currentItem()) {
            // Get the selected proposal data
            QVariantMap proposalData = incomingPrpsls->currentItem()->data(Qt::UserRole).toMap();
            
            // Show detailed dialog or expand the right panel
            QMessageBox::information(this, "Proposal Details", 
                "Proposal from: " + proposalData["freelancerName"].toString() + "\n\n" +
                proposalData["proposalDescription"].toString() + "\n\n" +
                "Requested Budget: " + proposalData["requestedBudget"].toString());
        } });

    connect(acceptPrpslBtn, &QPushButton::clicked, [=]()
            {
        if (incomingPrpsls->currentItem()) {
            QVariantMap proposalData = incomingPrpsls->currentItem()->data(Qt::UserRole).toMap();
            
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, "Accept Proposal",
                "Are you sure you want to accept the proposal from " + 
                proposalData["freelancerName"].toString() + "?",
                QMessageBox::Yes | QMessageBox::No);
                
            if (confirm == QMessageBox::Yes) {
                // Update proposal status in the backend
                updateProposalStatus(
                    proposalData["jobId"].toString(),
                    proposalData["freelancerId"].toString(),
                    "accepted"
                );
                
                // Update UI
                proposalStatusLabel->setText("Status: Accepted");
                proposalStatusLabel->setStyleSheet("font-weight: bold; color: #28a745;");
                acceptPrpslBtn->setEnabled(false);
                
                // Update the item in the list
                QVariantMap updatedData = proposalData;
                updatedData["status"] = "accepted";
                incomingPrpsls->currentItem()->setData(Qt::UserRole, updatedData);
                
                // Add visual indication in the list
                incomingPrpsls->currentItem()->setText(
                    "✓ " + proposalData["jobTitle"].toString() + " - " + 
                    proposalData["freelancerName"].toString()
                );
                incomingPrpsls->currentItem()->setBackground(QBrush(QColor("#d4edda")));
            }
        } });

    connect(rejectPrpslBtn, &QPushButton::clicked, [=]()
            {
        if (incomingPrpsls->currentItem()) {
            QVariantMap proposalData = incomingPrpsls->currentItem()->data(Qt::UserRole).toMap();
            
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, "Reject Proposal",
                "Are you sure you want to reject the proposal from " + 
                proposalData["freelancerName"].toString() + "?",
                QMessageBox::Yes | QMessageBox::No);
                
            if (confirm == QMessageBox::Yes) {
                // Update proposal status in the backend
                updateProposalStatus(
                    proposalData["jobId"].toString(),
                    proposalData["freelancerId"].toString(),
                    "rejected"
                );
                
                // Update UI
                proposalStatusLabel->setText("Status: Rejected");
                proposalStatusLabel->setStyleSheet("font-weight: bold; color: #dc3545;");
                rejectPrpslBtn->setEnabled(false);
                
                // Update the item in the list
                QVariantMap updatedData = proposalData;
                updatedData["status"] = "rejected";
                incomingPrpsls->currentItem()->setData(Qt::UserRole, updatedData);
                
                // Add visual indication in the list
                incomingPrpsls->currentItem()->setText(
                    "✗ " + proposalData["jobTitle"].toString() + " - " + 
                    proposalData["freelancerName"].toString()
                );
                incomingPrpsls->currentItem()->setBackground(QBrush(QColor("#f8d7da")));
            }
        } });

    // Arrange everything in the dashboard
    dashboardLayout->addWidget(summaryGroup);
    dashboardLayout->addLayout(proposalsLayout);

    // Load proposals when the dashboard is created
    QTimer::singleShot(100, this, &HiringManagerPortal::loadProposalsForHiringManager);

    return dashboardWidget;
}
QWidget *HiringManagerPortal::createProfileTab()
{
    QWidget *profileWidget = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);

    // Company/User Information
    QGroupBox *infoGroup = new QGroupBox("Company Information");
    QFormLayout *infoLayout = new QFormLayout();

    UserManager *userManager = UserManager::getInstance();
    HiringManager *hiringManager = userManager->getCurrentHiringManager();

    // Create default labels
    nameLabel = new QLabel("Please log in");
    emailLabel = new QLabel("");
    descriptionTextEdit = new QTextEdit();
    descriptionTextEdit->setReadOnly(true);
    descriptionTextEdit->setMaximumHeight(100);
    companyNameLabel = new QLabel("");

    // Only try to populate them if we have a hiring manager
    if (hiringManager)
    {
        nameLabel->setText(QString::fromStdString(hiringManager->getName()));
        emailLabel->setText(QString::fromStdString(hiringManager->getEmail()));
        descriptionTextEdit->setText(QString::fromStdString(hiringManager->getCompanyDescription()));
        companyNameLabel->setText(QString::fromStdString(hiringManager->getCompanyName()));
    }
    infoLayout->addRow("Name:", nameLabel);
    infoLayout->addRow("Email:", emailLabel);
    infoLayout->addRow("Company Description:", descriptionTextEdit);
    infoLayout->addRow("Company Name:", companyNameLabel);
    infoGroup->setLayout(infoLayout);

    QGroupBox *accomplishmentsGroup = new QGroupBox("Company Accomplishments");
    QVBoxLayout *accomplishmentsLayout = new QVBoxLayout();

    accomplishmentsList = new QListWidget();
    accomplishmentsLayout->addWidget(accomplishmentsList);
    accomplishmentsGroup->setLayout(accomplishmentsLayout);

    QPushButton *editProfileBtn = new QPushButton("Edit Profile");
    editProfileBtn->setStyleSheet("background-color: #6c757d; color: white; padding: 8px;");

    connect(editProfileBtn, &QPushButton::clicked, [this]()
            {
        UserManager* userManager = UserManager::getInstance();
        
        // Debug info
        qDebug() << "Edit Profile clicked";
        qDebug() << "UserManager says user is logged in:" << userManager->isUserLoggedIn();
        
        if (currentUser) {
            qDebug() << "HiringManagerPortal has currentUser:" << QString::fromStdString(currentUser->getName());
        } else {
            qDebug() << "HiringManagerPortal currentUser is null";
            currentUser = userManager->getCurrentUser(); // Try to get current user from UserManager
            if (!currentUser) {
                QMessageBox::warning(this, "Error", "User data could not be retrieved. Please try logging in again.");
                return;
            }
        }
        
        if (!userManager->isUserLoggedIn()) {
            QMessageBox::warning(this, "Not Logged In", "You need to be logged in to edit your profile.");
            return;
        }
        HiringManager* hiringManager = userManager->getCurrentHiringManager();
        if (!hiringManager && currentUser) {
            hiringManager = dynamic_cast<HiringManager*>(currentUser);
            if (hiringManager) {
                // Fix the UserManager if it's out of sync
                userManager->setCurrentUser(hiringManager);
            }
         }
        
        if (!hiringManager) {
            QMessageBox::warning(this, "Error", "Could not retrieve hiring manager data.");
            return;
        }
        
        // Open the profile edit dialog
        HiringManagerProfileEdit* dialog = new HiringManagerProfileEdit(hiringManager, this);
        
        connect(dialog, &HiringManagerProfileEdit::profileUpdated, [this, hiringManager]() {
            // Make sure currentUser is set to the updated hiringManager
            currentUser = UserManager::getInstance()->getCurrentUser();
            // Also update in UserManager
            // Refresh the UI with updated info
            updateProfileInfo();
        });
        
        dialog->exec();
        delete dialog; });

    // Company/User stats
    QGroupBox *statsGroup = new QGroupBox("Your Statistics");
    QFormLayout *statsLayout = new QFormLayout();

    statsLayout->addRow("Jobs Posted:", new QLabel("15"));
    statsLayout->addRow("Jobs Completed:", new QLabel("12"));
    statsLayout->addRow("Average Rating:", new QLabel("4.8/5.0"));
    statsLayout->addRow("Average Response Time:", new QLabel("6 hours"));

    statsGroup->setLayout(statsLayout);
    // Arrange everything in the profile tab
    profileLayout->addWidget(infoGroup);
    profileLayout->addWidget(editProfileBtn, 0, Qt::AlignRight);
    // profileLayout->addWidget(preferencesGroup);
    profileLayout->addWidget(statsGroup);
    profileLayout->addStretch();

    return profileWidget;
}

QWidget *HiringManagerPortal::createPostJobTab()
{
    return new JobFeed(this, false);
}

QWidget *HiringManagerPortal::createMessagesTab()
{
    QWidget *messagesWidget = new QWidget();
    QVBoxLayout *messagesLayout = new QVBoxLayout(messagesWidget);

    // Split view: Message list and message content
    QHBoxLayout *splittedLayout = new QHBoxLayout();

    // Left side: conversation list
    QGroupBox *conversationsGroup = new QGroupBox("Conversations");
    QVBoxLayout *conversationsLayout = new QVBoxLayout();

    // Filter inbox
    QLineEdit *searchMessages = new QLineEdit();
    searchMessages->setPlaceholderText("Search conversations...");

    // People list
    QListWidget *conversationsList = new QListWidget();
    conversationsList->addItem("John Doe (Frontend Developer - React.js)");
    conversationsList->addItem("Jane Smith (UI/UX Designer)");
    conversationsList->addItem("Mike Johnson (Backend Developer - Node.js)");

    conversationsLayout->addWidget(searchMessages);
    conversationsLayout->addWidget(conversationsList);
    conversationsGroup->setLayout(conversationsLayout);

    // Right side: message thread
    QGroupBox *messageThreadGroup = new QGroupBox("Messages");
    QVBoxLayout *messageThreadLayout = new QVBoxLayout();

    // Thread header
    QLabel *threadHeaderLabel = new QLabel("Conversation with John Doe");
    threadHeaderLabel->setStyleSheet("font-weight: bold;");

    // Messages area (in a real app, this would be a custom widget with speech bubbles)
    QTextEdit *messagesArea = new QTextEdit();
    messagesArea->setReadOnly(true);
    messagesArea->append("<div style='background-color: #f1f0f0; padding: 8px; margin-bottom: 10px; border-radius: 8px;'><b>John Doe:</b> Hi, I'm interested in your React.js job posting. I have 3 years of experience with React and related technologies.</div>");
    messagesArea->append("<div style='background-color: #dcf8c6; padding: 8px; margin: 10px 0px 10px 40px; border-radius: 8px; text-align: right;'><b>You:</b> Thanks for your interest, John! Could you tell me more about your experience?</div>");
    messagesArea->append("<div style='background-color: #f1f0f0; padding: 8px; margin-bottom: 10px; border-radius: 8px;'><b>John Doe:</b> Sure! I've worked on several e-commerce projects, including a major retail platform that serves over 1M users. I'm proficient with Redux, React Router, and modern JavaScript.</div>");

    // New message area
    QHBoxLayout *newMessageLayout = new QHBoxLayout();
    QTextEdit *newMessageEdit = new QTextEdit();
    newMessageEdit->setMaximumHeight(80);
    newMessageEdit->setPlaceholderText("Type your message here...");

    QPushButton *sendBtn = new QPushButton("Send");
    sendBtn->setStyleSheet("background-color: #007bff; color: white;");

    newMessageLayout->addWidget(newMessageEdit);
    newMessageLayout->addWidget(sendBtn, 0, Qt::AlignBottom);

    // Connect send button
    connect(sendBtn, &QPushButton::clicked, [=]()
            {
        if (!newMessageEdit->toPlainText().isEmpty()) {
            messagesArea->append(QString("<div style='background-color: #dcf8c6; padding: 8px; margin: 10px 0px 10px 40px; border-radius: 8px; text-align: right;'><b>You:</b> %1</div>").arg(newMessageEdit->toPlainText()));
            newMessageEdit->clear();
        } });

    messageThreadLayout->addWidget(threadHeaderLabel);
    messageThreadLayout->addWidget(messagesArea);
    messageThreadLayout->addLayout(newMessageLayout);
    messageThreadGroup->setLayout(messageThreadLayout);

    // Arrange the split view
    splittedLayout->addWidget(conversationsGroup, 1);
    splittedLayout->addWidget(messageThreadGroup, 2);

    messagesLayout->addLayout(splittedLayout);

    // Connect conversation selection to update messages
    connect(conversationsList, &QListWidget::currentTextChanged, [=](const QString &text)
            {
                threadHeaderLabel->setText("Conversation with " + text.split(" (").first());
                // In a real app, this would load actual messages from the backend
            });

    return messagesWidget;
}

QWidget *HiringManagerPortal::createRatingsTab()
{
    QWidget *ratingsWidget = new QWidget();
    QVBoxLayout *ratingsLayout = new QVBoxLayout(ratingsWidget);

    // Your overall rating
    QGroupBox *overallGroup = new QGroupBox("Your Overall Rating");
    QVBoxLayout *overallLayout = new QVBoxLayout();

    QHBoxLayout *starsLayout = new QHBoxLayout();
    QLabel *bigStarLabel = new QLabel("★★★★☆");
    bigStarLabel->setStyleSheet("font-size: 32px; color: gold;");
    QLabel *ratingLabel = new QLabel("4.8");
    ratingLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    starsLayout->addWidget(bigStarLabel);
    starsLayout->addWidget(ratingLabel);
    starsLayout->addStretch();

    QHBoxLayout *ratingDetailsLayout = new QHBoxLayout();
    QFormLayout *ratingBreakdownForm = new QFormLayout();
    ratingBreakdownForm->addRow("Communication:", new QLabel("★★★★★ 5.0"));
    ratingBreakdownForm->addRow("Clarity of Requirements:", new QLabel("★★★★☆ 4.7"));
    ratingBreakdownForm->addRow("Payment Promptness:", new QLabel("★★★★★ 5.0"));
    ratingBreakdownForm->addRow("Professionalism:", new QLabel("★★★★☆ 4.6"));

    QLabel *totalReviewsLabel = new QLabel("Based on 27 reviews");

    ratingDetailsLayout->addLayout(ratingBreakdownForm);
    ratingDetailsLayout->addStretch();
    ratingDetailsLayout->addWidget(totalReviewsLabel, 0, Qt::AlignBottom);

    overallLayout->addLayout(starsLayout);
    overallLayout->addLayout(ratingDetailsLayout);
    overallGroup->setLayout(overallLayout);

    // Recent feedback
    QGroupBox *feedbackGroup = new QGroupBox("Recent Feedback from Freelancers");
    QVBoxLayout *feedbackLayout = new QVBoxLayout();

    // Sample feedback items - in a real app these would come from the database
    auto addFeedbackItem = [&feedbackLayout](const QString &name, const QString &job, const QString &rating, const QString &comment)
    {
        QGroupBox *feedbackItem = new QGroupBox();
        QVBoxLayout *itemLayout = new QVBoxLayout();

        QHBoxLayout *headerLayout = new QHBoxLayout();
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("font-weight: bold;");
        QLabel *jobLabel = new QLabel("(" + job + ")");
        jobLabel->setStyleSheet("color: gray;");
        QLabel *starLabel = new QLabel(rating);
        starLabel->setStyleSheet("color: gold;");

        headerLayout->addWidget(nameLabel);
        headerLayout->addWidget(jobLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(starLabel);

        QLabel *commentLabel = new QLabel(comment);
        commentLabel->setWordWrap(true);

        itemLayout->addLayout(headerLayout);
        itemLayout->addWidget(commentLabel);

        feedbackItem->setLayout(itemLayout);
        feedbackLayout->addWidget(feedbackItem);
    };

    addFeedbackItem("John Doe", "Frontend Developer", "★★★★★",
                    "Great client to work with! Requirements were clear, and payment was prompt. Would happily work with them again.");

    addFeedbackItem("Jane Smith", "UI/UX Designer", "★★★★☆",
                    "Good experience overall. There were some changes to the scope during the project but we managed to work through them.");

    addFeedbackItem("Mike Johnson", "Backend Developer", "★★★★★",
                    "One of the best clients I've worked with. Very professional and respectful of my time.");

    feedbackGroup->setLayout(feedbackLayout);

    // Pending ratings (jobs to rate)
    QGroupBox *pendingGroup = new QGroupBox("Rate Completed Jobs");
    QVBoxLayout *pendingLayout = new QVBoxLayout();

    QLabel *instructionLabel = new QLabel("Rate freelancers who have completed jobs for you:");

    QListWidget *pendingRatingsList = new QListWidget();
    pendingRatingsList->addItem("Alex Williams - Mobile App Development (Completed 3 days ago)");
    pendingRatingsList->addItem("Sarah Chen - Logo Design (Completed 1 week ago)");

    QPushButton *rateFreelancerBtn = new QPushButton("Rate Selected Freelancer");
    rateFreelancerBtn->setStyleSheet("background-color: #007bff; color: white;");

    // Connect rate button
    connect(rateFreelancerBtn, &QPushButton::clicked, [=]()
            {
        if (pendingRatingsList->currentItem()) {
            // In a real app, this would open a rating dialog
            QMessageBox::information(this, "Rate Freelancer", 
                                    "This would open a rating form for: " + 
                                    pendingRatingsList->currentItem()->text());
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a freelancer to rate.");
        } });

    pendingLayout->addWidget(instructionLabel);
    pendingLayout->addWidget(pendingRatingsList);
    pendingLayout->addWidget(rateFreelancerBtn, 0, Qt::AlignRight);
    pendingGroup->setLayout(pendingLayout);

    // Arrange everything in the ratings tab
    ratingsLayout->addWidget(overallGroup);
    ratingsLayout->addWidget(feedbackGroup);
    ratingsLayout->addWidget(pendingGroup);

    return ratingsWidget;
}

void HiringManagerPortal::fetchProfileFromFirebase()
{
    if (!currentUser)
    {
        qDebug() << "Cannot fetch profile: No user is set";
        return;
    }

    BackendClient *client = BackendClient::getInstance();

    // User ID to fetch
    QString uid = QString::fromStdString(currentUser->getUid());
    qDebug() << "Fetching profile for UID:" << uid;

    // Create request to get user profile
    QJsonObject requestObj;
    requestObj["type"] = QString("getProfile");
    requestObj["uid"] = uid;

    client->sendRequest(requestObj, [this](const QJsonObject &responseObj)
                        {
        qDebug() << "Got profile response:" << QJsonDocument(responseObj).toJson();
        
        if (responseObj["status"].toString() == "success") {
            try {
                if (!currentUser) {
                    qDebug() << "User became null during profile fetch";
                    return;
                }
                
                if (responseObj.contains("description")) {
                    QString description = responseObj["description"].toString();
                    currentUser->setDescription(description.toStdString());
                }
                
                if (responseObj.contains("name")) {
                    QString name = responseObj["name"].toString();
                    currentUser->setName(name.toStdString());
                    qDebug() << "Name from server:" << name;
                }
                
                if (responseObj.contains("email")) {
                    QString email = responseObj["email"].toString();
                    currentUser->setEmail(email.toStdString());
                }
                
                // HiringManager specific fields
                HiringManager* hiringManager = dynamic_cast<HiringManager*>(currentUser);
                if (hiringManager) {
                    if (responseObj.contains("companyName")) {
                        QString companyName = responseObj["companyName"].toString();
                        hiringManager->setCompanyName(companyName.toStdString());
                    }
                    
                    if (responseObj.contains("companyDescription")) {
                        QString companyDesc = responseObj["companyDescription"].toString();
                        hiringManager->setCompanyDescription(companyDesc.toStdString());
                    }
                    
                    // Parse tags if available
                    // if (responseObj.contains("tags")) {
                    //     QJsonArray tagsArray = responseObj["tags"].toArray();
                        
                    //     // Clear existing tags
                    //     std::vector<std::string> currentTags = hiringManager->getTags();
                    //     for (const auto& tag : currentTags) {
                    //         hiringManager->removeTags(tag);
                    //     }
                        
                    //     // Add new tags
                    //     for (const auto &tagValue : tagsArray) {
                    //         hiringManager->addTags(tagValue.toString().toStdString());
                    //     }
                    // }
                    
                    // Parse accomplishments if available
                    if (responseObj.contains("accomplishments")) {
                        QJsonArray accArray = responseObj["accomplishments"].toArray();
                        
                        // Clear existing accomplishments
                        std::vector<std::string> currentAccomplishments = hiringManager->getAccomplishments();
                        for (const auto& acc : currentAccomplishments) {
                            hiringManager->removeAccomplishment(acc);
                        }
                        
                        // Add new accomplishments
                        for (const auto &accValue : accArray) {
                            hiringManager->addAccomplishment(accValue.toString().toStdString());
                        }
                    }
                    
                    // Parse job history if available
                    if (responseObj.contains("jobHistory")) {
                        QJsonArray jobArray = responseObj["jobHistory"].toArray();
                        
                        // Clear existing job history
                        std::vector<User::experience> currentJobs = hiringManager->getJobHistory();
                        for (const auto& job : currentJobs) {
                            hiringManager->removeJobHistory(job.jobTitle);
                        }
                        
                        // Add new job history
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
                    UserManager::getInstance()->setCurrentUser(hiringManager);
                    this->currentUser = hiringManager;
                }
                
                // Update the UI with the fetched data
                updateProfileInfo();
                qDebug() << "Profile updated successfully from Firebase";
            }
            catch (const std::exception& e) {
                qDebug() << "Exception during profile update:" << e.what();
                QMessageBox::warning(this, "Profile Update Error", 
                    QString("Error updating profile: %1").arg(e.what()));
            }
            catch (...) {
                qDebug() << "Unknown exception during profile update";
                QMessageBox::warning(this, "Profile Update Error", 
                    "An unknown error occurred while updating the profile");
            }
        } else {
            QString errorMsg = "Failed to load profile data";
            if (responseObj.contains("error")) {
                errorMsg += ": " + responseObj["error"].toString();
            }
            QMessageBox::warning(this, "Profile Load Error", errorMsg);
        } });
}

// Add to HiringManagerPortal.h in the private section:
void loadProposalsForHiringManager();
void updateProposalStatus(const QString &jobId, const QString &freelancerId, const QString &status);

// Add to HiringManagerPortal.cpp:
void HiringManagerPortal::loadProposalsForHiringManager()
{
    // Clear previous proposals
    incomingPrpsls->clear();

    // Get the current hiring manager
    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
    if (!hiringManager)
    {
        qDebug() << "No hiring manager found, can't load proposals";
        return;
    }

    // Get proposals using employer ID
    QString employerId = QString::fromStdString(hiringManager->getName());
    BackendClient::getInstance()->getProposals(employerId,
        [this](bool success, const QJsonArray &proposals)
        {
        if (success)
        {
        int pendingCount = 0;

        for (const QJsonValue &proposalValue : proposals)
        {
            QJsonObject proposalObj = proposalValue.toObject();

            // Create display item for the list
            QString freelancerName = proposalObj["freelancerName"].toString();
            QString jobTitle = proposalObj["jobTitle"].toString();
            QString status = proposalObj["status"].toString();

            // Create list item with status indicator
            QListWidgetItem *item = new QListWidgetItem();
            QString displayText = jobTitle + " - " + freelancerName;

            if (status == "pending")
            {
                item->setText(displayText);
                pendingCount++;
            }
            else if (status == "accepted")
            {
                item->setText("✓ " + displayText);
                item->setBackground(QBrush(QColor("#d4edda"))); // Light green
            }
            else if (status == "rejected")
            {
                item->setText("✗ " + displayText);
                item->setBackground(QBrush(QColor("#f8d7da"))); // Light red
            }

            // Store all proposal data in the item's user role
            QVariantMap proposalData;
            for (auto it = proposalObj.constBegin(); it != proposalObj.constEnd(); ++it)
            {
                proposalData[it.key()] = it.value().toVariant();
            }

            item->setData(Qt::UserRole, proposalData);
            incomingPrpsls->addItem(item);
        }

        // Update the summary count
        QList<QLabel *> labels = findChildren<QLabel *>();
        for (QLabel *label : labels)
        {
            if (label->styleSheet().contains("color: #ffc107"))
            { // Yellow color for pending
                label->setText(QString::number(pendingCount));
                break;
            }
        }
        }
        else
        {
        qDebug() << "Failed to load proposals";
        QListWidgetItem *errorItem = new QListWidgetItem("Failed to load proposals");
        errorItem->setFlags(errorItem->flags() & ~Qt::ItemIsEnabled);
        incomingPrpsls->addItem(errorItem);
        }
        });
}

void HiringManagerPortal::updateProposalStatus(const QString &jobId, const QString &freelancerId, const QString &status)
{
    BackendClient *client = BackendClient::getInstance();

    QJsonObject request;
    request["type"] = "updateProposalStatus";
    request["jobId"] = jobId;
    request["freelancerId"] = freelancerId;
    request["status"] = status;

    client->sendRequest(request, [this, status](const QJsonObject &response)
                        {
        if (response["status"].toString() == "success") {
            QString message = status == "accepted" ? "Proposal accepted successfully!" : "Proposal rejected successfully!";
            QMessageBox::information(this, "Success", message);
            
            // If you accepted a proposal, refresh to update other proposals that might need to be rejected
            if (status == "accepted") {
                loadProposalsForHiringManager();
            }
        } else {
            QMessageBox::warning(this, "Error", 
                "Failed to update proposal status: " + response["error"].toString());
        } });
}