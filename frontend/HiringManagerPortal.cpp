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
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QDate>
#include <QDateTime>
#include <QTimer>
#include <QButtonGroup>
#include <QSplitter>
#include <QSlider>

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

        QTimer::singleShot(1000, this, &HiringManagerPortal::loadHMCompletedJobs);

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

    // Completed Jobs Tab (replaces Messages)
    QWidget *completedJobsTab = createCompletedJobsTab();
    tabWidget->addTab(completedJobsTab, "Completed Jobs");

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
    incomingPrpsls->setStyleSheet(
        "QListWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #eaeaea; "
        "} "
        "QListWidget::item:selected { "
        "background-color: #403d39; " // Semi-transparent blue
        "border: 2px solid #007bff; "
        "} "
        "QListWidget::item:hover { "
        "background-color: rgba(108, 117, 125, 0.1); " // Light gray on hover
        "}");
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
                // In the connect(incomingPrpsls, &QListWidget::currentItemChanged, ...) section:
                connect(incomingPrpsls, &QListWidget::currentItemChanged,
                        [=](QListWidgetItem *current, QListWidgetItem *previous)
                        {
                            if (current)
                            {
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

                                    // Show accept/reject buttons for pending proposals
                                    viewDetailsBtn->setEnabled(true);
                                    acceptPrpslBtn->setEnabled(true);
                                    acceptPrpslBtn->setText("Accept Proposal");
                                    acceptPrpslBtn->setStyleSheet("background-color: #28a745; color: white;");
                                    rejectPrpslBtn->setEnabled(true);
                                    rejectPrpslBtn->setText("Reject Proposal");
                                    rejectPrpslBtn->setStyleSheet("background-color: #dc3545; color: white;");
                                }
                                else if (status == "accepted")
                                {
                                    proposalStatusLabel->setText("Status: Accepted - In Progress");
                                    proposalStatusLabel->setStyleSheet("font-weight: bold; color: #28a745;");

                                    // Show complete job button for accepted proposals
                                    viewDetailsBtn->setEnabled(true);
                                    acceptPrpslBtn->setEnabled(true);
                                    acceptPrpslBtn->setText("Complete Job");
                                    acceptPrpslBtn->setStyleSheet("background-color: #17a2b8; color: white;"); // Blue color
                                    rejectPrpslBtn->setEnabled(false);
                                    rejectPrpslBtn->setText("Reject Proposal");
                                    rejectPrpslBtn->setStyleSheet("background-color: #6c757d; color: white;"); // Disabled gray
                                }
                                else if (status == "rejected")
                                {
                                    proposalStatusLabel->setText("Status: Rejected");
                                    proposalStatusLabel->setStyleSheet("font-weight: bold; color: #dc3545;");

                                    // Allow re-accepting rejected proposals
                                    viewDetailsBtn->setEnabled(true);
                                    acceptPrpslBtn->setEnabled(true);
                                    acceptPrpslBtn->setText("Accept Proposal");
                                    acceptPrpslBtn->setStyleSheet("background-color: #28a745; color: white;");
                                    rejectPrpslBtn->setEnabled(false);
                                    rejectPrpslBtn->setText("Reject Proposal");
                                    rejectPrpslBtn->setStyleSheet("background-color: #6c757d; color: white;");
                                }
                                else if (status == "completed")
                                {
                                    proposalStatusLabel->setText("Status: Completed");
                                    proposalStatusLabel->setStyleSheet("font-weight: bold; color: #6f42c1;"); // Purple

                                    // No actions available for completed jobs
                                    viewDetailsBtn->setEnabled(true);
                                    acceptPrpslBtn->setEnabled(false);
                                    acceptPrpslBtn->setText("Job Completed");
                                    acceptPrpslBtn->setStyleSheet("background-color: #6c757d; color: white;");
                                    rejectPrpslBtn->setEnabled(false);
                                    rejectPrpslBtn->setText("Reject Proposal");
                                    rejectPrpslBtn->setStyleSheet("background-color: #6c757d; color: white;");
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
        if (incomingPrpsls->currentItem())
        {
            QVariantMap proposalData = incomingPrpsls->currentItem()->data(Qt::UserRole).toMap();
            QString currentStatus = proposalData["status"].toString();

            if (currentStatus == "pending" || currentStatus == "rejected")
            {
                // Accept proposal functionality
                QMessageBox::StandardButton confirm = QMessageBox::question(
                    this, "Accept Proposal",
                    "Are you sure you want to accept the proposal from " +
                        proposalData["freelancerName"].toString() + "?",
                    QMessageBox::Yes | QMessageBox::No);

                if (confirm == QMessageBox::Yes)
                {
                    // Update proposal status in the backend
                    updateProposalStatus(
                        proposalData["jobId"].toString(),
                        proposalData["freelancerId"].toString(),
                        "accepted");

                    // Update UI
                    proposalStatusLabel->setText("Status: Accepted - In Progress");
                    proposalStatusLabel->setStyleSheet("font-weight: bold; color: #28a745;");

                    // Change button to "Complete Job"
                    acceptPrpslBtn->setText("Complete Job");
                    acceptPrpslBtn->setStyleSheet("background-color: #17a2b8; color: white;");
                    rejectPrpslBtn->setEnabled(false);

                    // Update the item in the list
                    QVariantMap updatedData = proposalData;
                    updatedData["status"] = "accepted";
                    incomingPrpsls->currentItem()->setData(Qt::UserRole, updatedData);

                    // Update visual indication in the list
                    incomingPrpsls->currentItem()->setText(
                        "✓ " + proposalData["jobTitle"].toString() + " - " +
                        proposalData["freelancerName"].toString());
                    incomingPrpsls->currentItem()->setBackground(QBrush(QColor("#d4edda")));
                }
            }
            else if (currentStatus == "accepted")
            {
                // Complete job functionality
                QMessageBox::StandardButton confirm = QMessageBox::question(
                    this, "Complete Job",
                    "Are you sure you want to mark this job as completed?\n\n"
                    "Job: " +
                        proposalData["jobTitle"].toString() + "\n"
                                                              "Freelancer: " +
                        proposalData["freelancerName"].toString() + "\n\n"
                                                                    "This will move the job to completed jobs and remove it from active listings.",
                    QMessageBox::Yes | QMessageBox::No);

                if (confirm == QMessageBox::Yes)
                {
                    // Get current hiring manager
                    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
                    if (!hiringManager)
                    {
                        QMessageBox::warning(this, "Error", "Could not retrieve hiring manager information.");
                        return;
                    }

                    // Complete the job (this will handle moving to completed jobs and deleting from jobs collection)
                    completeJob(
                        proposalData["jobId"].toString(),
                        QString::fromStdString(hiringManager->getUid()),
                        proposalData["freelancerId"].toString(),
                        proposalData["jobTitle"].toString(),
                        proposalData["jobDescription"].toString(),
                        proposalData["requestedBudget"].toString().toDouble());
                }
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
    QWidget *postJobWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(postJobWidget);

    // Create a scrollable area for the form
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *formWidget = new QWidget();
    QVBoxLayout *formLayout = new QVBoxLayout(formWidget);

    // Job posting form
    QGroupBox *jobFormGroup = new QGroupBox("Post New Job");
    QFormLayout *jobFormLayout = new QFormLayout();

    // Job title
    QLineEdit *jobTitleEdit = new QLineEdit();
    jobTitleEdit->setPlaceholderText("Enter job title...");
    jobFormLayout->addRow("Job Title*:", jobTitleEdit);

    // Job description
    QTextEdit *jobDescEdit = new QTextEdit();
    jobDescEdit->setPlaceholderText("Describe the job requirements, responsibilities, and any other relevant details...");
    jobDescEdit->setMinimumHeight(150);
    jobFormLayout->addRow("Job Description*:", jobDescEdit);

    // Payment/Budget
    QLineEdit *paymentEdit = new QLineEdit();
    paymentEdit->setPlaceholderText("e.g., 2500");
    jobFormLayout->addRow("Budget ($)*:", paymentEdit);

    // Required skills
    QLineEdit *skillsEdit = new QLineEdit();
    skillsEdit->setPlaceholderText("e.g., React, TypeScript, Node.js (comma separated)");
    jobFormLayout->addRow("Required Skills*:", skillsEdit);

    // Expiry date (optional)
    QLineEdit *expiryEdit = new QLineEdit();
    expiryEdit->setPlaceholderText("YYYY-MM-DD (optional)");
    jobFormLayout->addRow("Expiry Date:", expiryEdit);

    jobFormGroup->setLayout(jobFormLayout);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *postJobBtn = new QPushButton("Post Job");
    postJobBtn->setStyleSheet("background-color: #28a745; color: white; padding: 10px 20px; font-size: 14px;");

    QPushButton *clearFormBtn = new QPushButton("Clear Form");
    clearFormBtn->setStyleSheet("background-color: #6c757d; color: white; padding: 10px 20px; font-size: 14px;");

    buttonLayout->addStretch();
    buttonLayout->addWidget(clearFormBtn);
    buttonLayout->addWidget(postJobBtn);

    // Connect clear button
    connect(clearFormBtn, &QPushButton::clicked, [=]()
            {
        jobTitleEdit->clear();
        jobDescEdit->clear();
        paymentEdit->clear();
        skillsEdit->clear();
        expiryEdit->clear(); });

    // Connect post job button - THIS IS WHERE WE CALL addJob
    connect(postJobBtn, &QPushButton::clicked, [=]()
            {
        // Validate required fields
        if (jobTitleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Job title is required.");
            return;
        }
        
        if (jobDescEdit->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Job description is required.");
            return;
        }
        
        if (paymentEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Budget is required.");
            return;
        }
        
        if (skillsEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Required skills are required.");
            return;
        }
        
        // Get current hiring manager
        HiringManager* hiringManager = UserManager::getInstance()->getCurrentHiringManager();
        if (!hiringManager) {
            QMessageBox::warning(this, "Error", "You must be logged in as a hiring manager to post jobs.");
            return;
        }
        
        // Parse skills (comma separated)
        QStringList skillsList = skillsEdit->text().split(",", Qt::SkipEmptyParts);
        std::vector<std::string> requiredSkills;
        for (const QString &skill : skillsList) {
            requiredSkills.push_back(skill.trimmed().toStdString());
        }
        
        // Generate unique job ID (you might want a better method)
        QString jobId = QString("job_%1_%2")
            .arg(QDateTime::currentDateTime().toSecsSinceEpoch())
            .arg(QRandomGenerator::global()->bounded(1000, 9999));
        
        // Get current date
        QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
        
        // Create Job object
        Job newJob(
            jobId.toStdString(),                           // jobId
            jobTitleEdit->text().trimmed().toStdString(),  // title
            jobDescEdit->toPlainText().trimmed().toStdString(), // description
            hiringManager->getName(),                      // employer (hiring manager name)
            currentDate.toStdString(),                     // dateCreated
            expiryEdit->text().trimmed().toStdString(),    // expiryDate (can be empty)
            requiredSkills,                                // requiredSkills
            paymentEdit->text().trimmed().toStdString()    // payment
        );
        
        // Disable the post button while processing
        postJobBtn->setEnabled(false);
        postJobBtn->setText("Posting...");
        
        // Call the addJob method from BackendClient
        BackendClient::getInstance()->addJob(newJob, [=](bool success) {
            // Re-enable the button
            postJobBtn->setEnabled(true);
            postJobBtn->setText("Post Job");
            
            if (success) {
                QMessageBox::information(this, "Success", 
                    "Job posted successfully!\n\nJob ID: " + jobId);
                
                // Clear the form after successful posting
                jobTitleEdit->clear();
                jobDescEdit->clear();
                paymentEdit->clear();
                skillsEdit->clear();
                expiryEdit->clear();
                
                // Optionally refresh any job lists
                // You might want to emit a signal here to refresh other components
                
            } else {
                QMessageBox::warning(this, "Error", 
                    "Failed to post job. Please check your connection and try again.");
            }
        }); });

    // Add everything to the form layout
    formLayout->addWidget(jobFormGroup);
    formLayout->addLayout(buttonLayout);
    formLayout->addStretch();

    // Set up scroll area
    scrollArea->setWidget(formWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mainLayout->addWidget(scrollArea);

    return postJobWidget;
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

QWidget *HiringManagerPortal::createCompletedJobsTab()
{
    QWidget *completedJobsWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(completedJobsWidget);
    
    // Title
    QLabel *titleLabel = new QLabel("Completed Jobs");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Create main splitter for the layout
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left side: Completed jobs list
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Completed Jobs section
    QGroupBox *completedJobsGroup = new QGroupBox("Your Completed Jobs");
    completedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *completedLayout = new QVBoxLayout();
    
    hmCompletedJobsList = new QListWidget();
    hmCompletedJobsList->setStyleSheet(
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #e0e0e0; }"
        "QListWidget::item:selected { background-color: #4a5759; }"
        "QListWidget::item:hover { background-color: #b0c4b1; }"
    );
    completedLayout->addWidget(hmCompletedJobsList);
    completedJobsGroup->setLayout(completedLayout);
    
    leftLayout->addWidget(completedJobsGroup);
    
    // Right side: Job details and rating panel
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    // Job Information section for completed jobs
    hmCompletedJobInfoGroup = new QGroupBox("Completed Job Information");
    hmCompletedJobInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupHMCompletedJobInfoPanel();
    
    // Freelancer Rating section
    hmRatingGroup = new QGroupBox("Rate Freelancer");
    hmRatingGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupHMRatingPanel();
    
    rightLayout->addWidget(hmCompletedJobInfoGroup);
    rightLayout->addWidget(hmRatingGroup);
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 500});
    
    mainLayout->addWidget(mainSplitter);
    
    // Connect signals
    connect(hmCompletedJobsList, &QListWidget::currentItemChanged, 
            this, &HiringManagerPortal::onHMCompletedJobSelected);
    
    // Add refresh button
    QPushButton *refreshButton = new QPushButton("Refresh Completed Jobs");
    refreshButton->setStyleSheet("background-color: #28a745; color: white; padding: 8px; border-radius: 4px;");
    connect(refreshButton, &QPushButton::clicked, [this]() {
        loadHMCompletedJobs();
    });
    mainLayout->addWidget(refreshButton);
    
    return completedJobsWidget;
}

void HiringManagerPortal::setupHMCompletedJobInfoPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(hmCompletedJobInfoGroup);
    
    // Create scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    
    // Job details labels for completed jobs
    hmCompletedJobTitleLabel = new QLabel("Select a completed job to view details");
    hmCompletedJobTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    hmCompletedJobTitleLabel->setWordWrap(true);
    
    hmCompletedJobFreelancerLabel = new QLabel();
    hmCompletedJobFreelancerLabel->setStyleSheet("color: #495057; font-size: 14px;");
    
    hmCompletedJobPaymentLabel = new QLabel();
    hmCompletedJobPaymentLabel->setStyleSheet("color: #28a745; font-size: 14px; font-weight: bold;");
    
    hmCompletedJobDateLabel = new QLabel();
    hmCompletedJobDateLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    
    hmCompletedJobDescriptionLabel = new QLabel();
    hmCompletedJobDescriptionLabel->setWordWrap(true);
    hmCompletedJobDescriptionLabel->setStyleSheet(
        "background-color: #f8f9fa; border-radius: 6px; padding: 10px; margin: 5px 0;"
    );
    
    hmCompletedJobStatusLabel = new QLabel();
    hmCompletedJobStatusLabel->setStyleSheet("color: #28a745; font-weight: bold; font-size: 14px;");
    
    contentLayout->addWidget(hmCompletedJobTitleLabel);
    contentLayout->addWidget(hmCompletedJobFreelancerLabel);
    contentLayout->addWidget(hmCompletedJobPaymentLabel);
    contentLayout->addWidget(hmCompletedJobDateLabel);
    contentLayout->addWidget(new QLabel("Description:"));
    contentLayout->addWidget(hmCompletedJobDescriptionLabel);
    contentLayout->addWidget(hmCompletedJobStatusLabel);
    contentLayout->addStretch();
    
    scrollArea->setWidget(content);
    layout->addWidget(scrollArea);
}

void HiringManagerPortal::setupHMRatingPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(hmRatingGroup);
    
    // Freelancer info
    hmRatingFreelancerNameLabel = new QLabel("Select a completed job to rate the freelancer");
    hmRatingFreelancerNameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    layout->addWidget(hmRatingFreelancerNameLabel);
    
    hmRatingFreelancerEmailLabel = new QLabel();
    hmRatingFreelancerEmailLabel->setStyleSheet("color: #495057; font-size: 14px;");
    layout->addWidget(hmRatingFreelancerEmailLabel);
    
    // Rating section
    QLabel *ratingLabel = new QLabel("Rate the freelancer's work:");
    ratingLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(ratingLabel);
    
    // Star rating buttons
    QHBoxLayout *starsLayout = new QHBoxLayout();
    hmRatingButtons = new QButtonGroup(this);
    
    for (int i = 1; i <= 5; i++) {
        QPushButton *starBtn = new QPushButton(QString("★ %1").arg(i));
        starBtn->setCheckable(true);
        starBtn->setStyleSheet(
            "QPushButton { background-color: #f8f9fa; border: 2px solid #dee2e6; "
            "padding: 8px; border-radius: 4px; font-size: 14px; }"
            "QPushButton:checked { background-color: #ffc107; border-color: #ffc107; color: white; }"
            "QPushButton:hover { background-color: #e2e6ea; }"
        );
        hmRatingButtons->addButton(starBtn, i);
        starsLayout->addWidget(starBtn);
    }
    layout->addLayout(starsLayout);
    
    // Comment section
    QLabel *commentLabel = new QLabel("Additional comments (optional):");
    commentLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(commentLabel);
    
    hmRatingCommentEdit = new QTextEdit();
    hmRatingCommentEdit->setMaximumHeight(100);
    hmRatingCommentEdit->setPlaceholderText("Share your experience working with this freelancer...");
    hmRatingCommentEdit->setStyleSheet("border: 1px solid #dee2e6; border-radius: 4px; padding: 8px;");
    layout->addWidget(hmRatingCommentEdit);
    
    // Submit rating button
    hmSubmitRatingBtn = new QPushButton("Submit Rating");
    hmSubmitRatingBtn->setStyleSheet(
        "background-color: #007bff; color: white; padding: 10px 20px; "
        "font-weight: bold; border-radius: 6px; font-size: 14px;"
    );
    hmSubmitRatingBtn->setEnabled(false);
    connect(hmSubmitRatingBtn, &QPushButton::clicked, this, &HiringManagerPortal::submitHMRating);
    layout->addWidget(hmSubmitRatingBtn);
    
    // Connect rating buttons
    connect(hmRatingButtons, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int rating) {
                hmCurrentRating = rating;
                hmSubmitRatingBtn->setEnabled(rating > 0);
            });
    
    layout->addStretch();
}

void HiringManagerPortal::loadHMCompletedJobs()
{
    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
    
    if (!hiringManager) {
        return;
    }
    
    QString hiringManagerId = QString::fromStdString(hiringManager->getUid());
    
    hmCompletedJobsList->clear();
    hmCompletedJobsList->addItem("Loading completed jobs...");
    
    BackendClient::getInstance()->getCompletedJobs(hiringManagerId, true, // true = as hiring manager
        [this](bool success, const QJsonArray &jobs) {
            hmCompletedJobsList->clear();
            
            if (!success) {
                hmCompletedJobsList->addItem("Failed to load completed jobs");
                return;
            }
            
            if (jobs.isEmpty()) {
                hmCompletedJobsList->addItem("No completed jobs found");
                return;
            }
            
            for (const auto &jobValue : jobs) {
                QJsonObject jobObj = jobValue.toObject();
                QString jobTitle = jobObj["jobName"].toString();
                if (jobTitle.isEmpty()) {
                    jobTitle = jobObj["jobTitle"].toString();
                }
                
                QString freelancerId = jobObj["freelancerId"].toString();
                QString payment = jobObj["budgetRequested"].toString();
                if (payment.isEmpty()) {
                    double budgetValue = jobObj["budgetRequested"].toDouble();
                    payment = QString::number(budgetValue);
                }
                
                QString displayText = QString("%1\nFreelancer: %2\nPayment: $%3")
                                       .arg(jobTitle)
                                       .arg(freelancerId.isEmpty() ? "Unknown" : "Loading...")
                                       .arg(payment);
                
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, jobValue);
                hmCompletedJobsList->addItem(item);
                
                // Fetch freelancer name for display
                if (!freelancerId.isEmpty()) {
                    BackendClient::getInstance()->getHiringManagerProfile(freelancerId,
                        [this, item](bool success, const QJsonArray &profileArray) {
                            if (success && !profileArray.isEmpty()) {
                                QJsonObject profile = profileArray[0].toObject();
                                QString freelancerName = profile["name"].toString();
                                if (freelancerName.isEmpty()) {
                                    freelancerName = profile["username"].toString();
                                }
                                if (freelancerName.isEmpty()) {
                                    freelancerName = "Unknown Freelancer";
                                }
                                
                                // Update the display text
                                QJsonObject jobData = item->data(Qt::UserRole).toJsonObject();
                                QString jobTitle = jobData["jobName"].toString();
                                if (jobTitle.isEmpty()) {
                                    jobTitle = jobData["jobTitle"].toString();
                                }
                                
                                QString payment = jobData["budgetRequested"].toString();
                                if (payment.isEmpty()) {
                                    double budgetValue = jobData["budgetRequested"].toDouble();
                                    payment = QString::number(budgetValue);
                                }
                                
                                QString updatedText = QString("%1\nFreelancer: %2\nPayment: $%3")
                                                       .arg(jobTitle)
                                                       .arg(freelancerName)
                                                       .arg(payment);
                                
                                item->setText(updatedText);
                            }
                        });
                }
            }
        });
}

void HiringManagerPortal::onHMCompletedJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current || current->data(Qt::UserRole).isNull()) {
        clearHMCompletedJobDetails();
        clearHMRatingPanel();
        return;
    }
    
    QJsonObject jobObj = current->data(Qt::UserRole).toJsonObject();
    updateHMCompletedJobDetails(jobObj);
    
    // Get the freelancer ID
    QString freelancerId = jobObj["freelancerId"].toString();
    loadFreelancerForRating(freelancerId, jobObj);
}

void HiringManagerPortal::updateHMCompletedJobDetails(const QJsonObject &jobObj)
{
    // Get job title - try both possible field names
    QString jobTitle = jobObj["jobName"].toString();
    if (jobTitle.isEmpty()) {
        jobTitle = jobObj["jobTitle"].toString();
    }
    
    hmCompletedJobTitleLabel->setText(jobTitle);
    
    // For now, show the freelancer ID until we fetch the name
    QString freelancerId = jobObj["freelancerId"].toString();
    hmCompletedJobFreelancerLabel->setText("Freelancer: Loading...");
    
    // Get budget
    QString budget = jobObj["budgetRequested"].toString();
    if (budget.isEmpty()) {
        double budgetValue = jobObj["budgetRequested"].toDouble();
        budget = QString::number(budgetValue);
    }
    hmCompletedJobPaymentLabel->setText("Payment: $" + budget);
    
    hmCompletedJobDescriptionLabel->setText(jobObj["jobDescription"].toString());
    hmCompletedJobStatusLabel->setText("Status: ✓ Completed");
    
    // You might want to add completion date if available
    if (jobObj.contains("completedAt")) {
        hmCompletedJobDateLabel->setText("Completed: " + jobObj["completedAt"].toString());
    } else {
        hmCompletedJobDateLabel->setText("Completion date not available");
    }
    
    // Now fetch the freelancer name
    if (!freelancerId.isEmpty()) {
        BackendClient::getInstance()->getHiringManagerProfile(freelancerId,
            [this](bool success, const QJsonArray &profileArray) {
                if (success && !profileArray.isEmpty()) {
                    QJsonObject profile = profileArray[0].toObject();
                    QString freelancerName = profile["name"].toString();
                    if (freelancerName.isEmpty()) {
                        freelancerName = profile["username"].toString();
                    }
                    if (freelancerName.isEmpty()) {
                        freelancerName = "Unknown Freelancer";
                    }
                    
                    // Update the label with the actual name
                    hmCompletedJobFreelancerLabel->setText("Freelancer: " + freelancerName);
                } else {
                    hmCompletedJobFreelancerLabel->setText("Freelancer: Unknown");
                }
            });
    }
}

void HiringManagerPortal::loadFreelancerForRating(const QString &freelancerId, const QJsonObject &jobObj)
{
    if (freelancerId.isEmpty()) {
        clearHMRatingPanel();
        return;
    }
    
    // Store current job data for rating submission
    hmCurrentCompletedJob = jobObj;
    
    // Show loading state
    hmRatingFreelancerNameLabel->setText("Loading freelancer details...");
    hmRatingFreelancerEmailLabel->setText("");
    
    BackendClient::getInstance()->getHiringManagerProfile(freelancerId,
        [this, freelancerId](bool success, const QJsonArray &profileArray) {
            if (!success || profileArray.isEmpty()) {
                hmRatingFreelancerNameLabel->setText("Freelancer details not available");
                hmRatingFreelancerEmailLabel->setText("");
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            updateHMRatingPanelDetails(profile);
            
            // Check for existing rating after updating the panel
            checkHMExistingRating(freelancerId);
        });
}

void HiringManagerPortal::updateHMRatingPanelDetails(const QJsonObject &profile)
{
    QString name = profile["name"].toString();
    QString email = profile["email"].toString();
    
    hmRatingFreelancerNameLabel->setText(name.isEmpty() ? "Name not available" : "Rate: " + name);
    hmRatingFreelancerEmailLabel->setText(email.isEmpty() ? "" : "Email: " + email);
    
    // Reset rating form
    hmRatingButtons->setExclusive(false);
    for (auto button : hmRatingButtons->buttons()) {
        button->setChecked(false);
    }
    hmRatingButtons->setExclusive(true);
    
    hmRatingCommentEdit->clear();
    hmCurrentRating = 0;
    hmSubmitRatingBtn->setEnabled(false);
}

void HiringManagerPortal::checkHMExistingRating(const QString &freelancerId)
{
    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
    
    if (!hiringManager) {
        return;
    }
    
    QString hiringManagerId = QString::fromStdString(hiringManager->getUid());
    
    // Get the freelancer's profile to check existing ratings
    BackendClient::getInstance()->getHiringManagerProfile(freelancerId,
        [this, hiringManagerId](bool success, const QJsonArray &profileArray) {
            if (!success || profileArray.isEmpty()) {
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            QJsonArray ratings = profile["ratings"].toArray();
            
            // Check if this hiring manager has already rated this freelancer
            bool hasRated = false;
            int existingRating = 0;
            QString existingComment;
            
            for (const auto &ratingValue : ratings) {
                QJsonObject rating = ratingValue.toObject();
                if (rating["fromUserId"].toString() == hiringManagerId) {
                    hasRated = true;
                    existingRating = rating["rating"].toInt();
                    existingComment = rating["comment"].toString();
                    break;
                }
            }
            
            if (hasRated) {
                // Pre-fill the form with existing rating
                QAbstractButton *button = hmRatingButtons->button(existingRating);
                if (button) {
                    button->setChecked(true);
                    hmCurrentRating = existingRating;
                }
                hmRatingCommentEdit->setText(existingComment);
                hmSubmitRatingBtn->setText("Update Rating");
                hmSubmitRatingBtn->setEnabled(true);
                
                // Add a note that this is an update
                QLabel *updateNote = new QLabel("Note: You have already rated this freelancer. Submitting will update your previous rating.");
                updateNote->setStyleSheet("color: #6c757d; font-style: italic; margin: 5px 0;");
                updateNote->setWordWrap(true);
                
                // Add the note to the rating group layout
                QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(hmRatingGroup->layout());
                if (layout) {
                    layout->insertWidget(layout->count() - 1, updateNote); // Insert before the stretch
                }
            } else {
                hmSubmitRatingBtn->setText("Submit Rating");
            }
        });
}

void HiringManagerPortal::submitHMRating()
{
    if (hmCurrentRating == 0 || hmCurrentCompletedJob.isEmpty()) {
        QMessageBox::warning(this, "Invalid Rating", "Please select a rating before submitting.");
        return;
    }
    
    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
    
    if (!hiringManager) {
        QMessageBox::warning(this, "Error", "User not found.");
        return;
    }
    
    QString fromUserId = QString::fromStdString(hiringManager->getUid());
    QString freelancerId = hmCurrentCompletedJob["freelancerId"].toString();
    QString comment = hmRatingCommentEdit->toPlainText().trimmed();
    
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this, "Submit Rating",
        QString("Are you sure you want to submit a %1-star rating for this freelancer?")
            .arg(hmCurrentRating),
        QMessageBox::Yes | QMessageBox::No);
    
    if (confirm == QMessageBox::Yes) {
        hmSubmitRatingBtn->setEnabled(false);
        hmSubmitRatingBtn->setText("Submitting...");
        
        BackendClient::getInstance()->rateUser(fromUserId, freelancerId, hmCurrentRating, comment,
            [this](bool success) {
                hmSubmitRatingBtn->setText("Submit Rating");
                
                if (success) {
                    QMessageBox::information(this, "Success", "Rating submitted successfully!");
                    
                    // Disable the rating form to prevent duplicate ratings
                    for (auto button : hmRatingButtons->buttons()) {
                        button->setEnabled(false);
                    }
                    hmRatingCommentEdit->setEnabled(false);
                    hmSubmitRatingBtn->setText("Rating Submitted");
                } else {
                    QMessageBox::warning(this, "Error", "Failed to submit rating. Please try again.");
                    hmSubmitRatingBtn->setEnabled(true);
                }
            });
    }
}

void HiringManagerPortal::clearHMCompletedJobDetails()
{
    hmCompletedJobTitleLabel->setText("Select a completed job to view details");
    hmCompletedJobFreelancerLabel->setText("");
    hmCompletedJobPaymentLabel->setText("");
    hmCompletedJobDateLabel->setText("");
    hmCompletedJobDescriptionLabel->setText("");
    hmCompletedJobStatusLabel->setText("");
}

void HiringManagerPortal::clearHMRatingPanel()
{
    hmRatingFreelancerNameLabel->setText("Select a completed job to rate the freelancer");
    hmRatingFreelancerEmailLabel->setText("");
    
    // Reset rating form
    hmRatingButtons->setExclusive(false);
    for (auto button : hmRatingButtons->buttons()) {
        button->setChecked(false);
        button->setEnabled(true);
    }
    hmRatingButtons->setExclusive(true);
    
    hmRatingCommentEdit->clear();
    hmRatingCommentEdit->setEnabled(true);
    hmCurrentRating = 0;
    hmSubmitRatingBtn->setEnabled(false);
    hmSubmitRatingBtn->setText("Submit Rating");
    
    hmCurrentCompletedJob = QJsonObject();
}

QWidget *HiringManagerPortal::createRatingsTab()
{
    QWidget *ratingsWidget = new QWidget();
    QVBoxLayout *ratingsLayout = new QVBoxLayout(ratingsWidget);

    // Your overall rating
    QGroupBox *overallGroup = new QGroupBox("Your Overall Rating");
    QVBoxLayout *overallLayout = new QVBoxLayout();

    QHBoxLayout *starsLayout = new QHBoxLayout();
    overallStarLabel = new QLabel("★★★★☆");
    overallStarLabel->setStyleSheet("font-size: 32px; color: gold;");
    overallRatingLabel = new QLabel("4.8");
    overallRatingLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    starsLayout->addWidget(overallStarLabel);
    starsLayout->addWidget(overallRatingLabel);
    starsLayout->addStretch();

    QHBoxLayout *ratingDetailsLayout = new QHBoxLayout();
    QFormLayout *ratingBreakdownForm = new QFormLayout();
    
    communicationRatingLabel = new QLabel("★★★★★ 5.0");
    clarityRatingLabel = new QLabel("★★★★☆ 4.7");
    paymentRatingLabel = new QLabel("★★★★★ 5.0");
    professionalismRatingLabel = new QLabel("★★★★☆ 4.6");
    
    ratingBreakdownForm->addRow("Communication:", communicationRatingLabel);
    ratingBreakdownForm->addRow("Clarity of Requirements:", clarityRatingLabel);
    ratingBreakdownForm->addRow("Payment Promptness:", paymentRatingLabel);
    ratingBreakdownForm->addRow("Professionalism:", professionalismRatingLabel);

    totalReviewsLabel = new QLabel("Based on 0 reviews");

    ratingDetailsLayout->addLayout(ratingBreakdownForm);
    ratingDetailsLayout->addStretch();
    ratingDetailsLayout->addWidget(totalReviewsLabel, 0, Qt::AlignBottom);

    overallLayout->addLayout(starsLayout);
    overallLayout->addLayout(ratingDetailsLayout);
    overallGroup->setLayout(overallLayout);

    // Recent feedback from freelancers
    QGroupBox *feedbackGroup = new QGroupBox("Recent Feedback from Freelancers");
    QVBoxLayout *feedbackLayout = new QVBoxLayout();
    
    // Scroll area for feedback
    QScrollArea *feedbackScrollArea = new QScrollArea();
    feedbackScrollArea->setWidgetResizable(true);
    feedbackScrollArea->setFrameShape(QFrame::NoFrame);
    
    feedbackContentWidget = new QWidget();
    feedbackContentLayout = new QVBoxLayout(feedbackContentWidget);
    
    // Initially show loading message
    QLabel *loadingLabel = new QLabel("Loading feedback...");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("color: #6c757d; font-style: italic; padding: 20px;");
    feedbackContentLayout->addWidget(loadingLabel);
    
    feedbackScrollArea->setWidget(feedbackContentWidget);
    feedbackLayout->addWidget(feedbackScrollArea);
    feedbackGroup->setLayout(feedbackLayout);

    // Arrange everything in the ratings tab
    ratingsLayout->addWidget(overallGroup);
    ratingsLayout->addWidget(feedbackGroup);

    // Load ratings when tab is created
    QTimer::singleShot(100, this, &HiringManagerPortal::loadHiringManagerRatings);

    return ratingsWidget;
}

void HiringManagerPortal::loadHiringManagerRatings()
{
    HiringManager *hiringManager = UserManager::getInstance()->getCurrentHiringManager();
    if (!hiringManager) {
        return;
    }
    
    QString hiringManagerId = QString::fromStdString(hiringManager->getUid());
    
    // Get the hiring manager's profile to load ratings
    BackendClient::getInstance()->getHiringManagerProfile(hiringManagerId,
        [this](bool success, const QJsonArray &profileArray) {
            if (!success || profileArray.isEmpty()) {
                updateRatingsDisplay(QJsonArray());
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            QJsonArray ratings = profile["ratings"].toArray();
            updateRatingsDisplay(ratings);
        });
}

void HiringManagerPortal::updateRatingsDisplay(const QJsonArray &ratings)
{
    // Clear existing feedback content
    QLayoutItem *child;
    while ((child = feedbackContentLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    if (ratings.isEmpty()) {
        QLabel *noRatingsLabel = new QLabel("No ratings received yet.");
        noRatingsLabel->setAlignment(Qt::AlignCenter);
        noRatingsLabel->setStyleSheet("color: #6c757d; font-style: italic; padding: 20px;");
        feedbackContentLayout->addWidget(noRatingsLabel);
        
        // Update overall rating display
        overallStarLabel->setText("☆☆☆☆☆");
        overallRatingLabel->setText("N/A");
        totalReviewsLabel->setText("No reviews yet");
        
        // Update breakdown ratings
        communicationRatingLabel->setText("No ratings");
        clarityRatingLabel->setText("No ratings");
        paymentRatingLabel->setText("No ratings");
        professionalismRatingLabel->setText("No ratings");
        
        return;
    }
    
    // Calculate overall rating
    double totalRating = 0.0;
    int ratingCount = ratings.size();
    
    for (const auto &ratingValue : ratings) {
        QJsonObject rating = ratingValue.toObject();
        totalRating += rating["rating"].toInt();
    }
    
    double averageRating = totalRating / ratingCount;
    
    // Update overall rating display
    QString stars = generateStarString(averageRating);
    overallStarLabel->setText(stars);
    overallRatingLabel->setText(QString::number(averageRating, 'f', 1));
    totalReviewsLabel->setText(QString("Based on %1 review%2").arg(ratingCount).arg(ratingCount == 1 ? "" : "s"));
    
    // For now, use the same rating for all categories (you could enhance this with category-specific ratings)
    QString categoryRating = QString("%1 %2").arg(stars).arg(QString::number(averageRating, 'f', 1));
    communicationRatingLabel->setText(categoryRating);
    clarityRatingLabel->setText(categoryRating);
    paymentRatingLabel->setText(categoryRating);
    professionalismRatingLabel->setText(categoryRating);
    
    // Add individual feedback items
    for (const auto &ratingValue : ratings) {
        QJsonObject rating = ratingValue.toObject();
        addFeedbackItem(rating);
    }
    
    feedbackContentLayout->addStretch();
}

void HiringManagerPortal::addFeedbackItem(const QJsonObject &rating)
{
    QGroupBox *feedbackItem = new QGroupBox();
    feedbackItem->setStyleSheet("QGroupBox { border: 1px solid #dee2e6; border-radius: 6px; margin: 5px 0; padding-top: 10px; }");
    QVBoxLayout *itemLayout = new QVBoxLayout();

    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    // Get freelancer name (you might want to fetch this from the freelancer's profile)
    QString fromUserId = rating["fromUserId"].toString();
    QLabel *nameLabel = new QLabel("Freelancer"); // Default name
    nameLabel->setStyleSheet("font-weight: bold;");
    
    // Fetch the actual freelancer name
    BackendClient::getInstance()->getHiringManagerProfile(fromUserId,
        [nameLabel](bool success, const QJsonArray &profileArray) {
            if (success && !profileArray.isEmpty()) {
                QJsonObject profile = profileArray[0].toObject();
                QString freelancerName = profile["name"].toString();
                if (freelancerName.isEmpty()) {
                    freelancerName = profile["username"].toString();
                }
                if (!freelancerName.isEmpty()) {
                    nameLabel->setText(freelancerName);
                }
            }
        });
    
    int ratingValue = rating["rating"].toInt();
    QString starString = generateStarString(ratingValue);
    QLabel *starLabel = new QLabel(starString);
    starLabel->setStyleSheet("color: gold; font-size: 16px;");
    
    // Add timestamp if available
    QLabel *timeLabel = new QLabel();
    if (rating.contains("timestamp")) {
        // You might want to format the timestamp properly
        timeLabel->setText("Recent");
        timeLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    }

    headerLayout->addWidget(nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(timeLabel);
    headerLayout->addWidget(starLabel);

    QString comment = rating["comment"].toString();
    QLabel *commentLabel = new QLabel();
    if (!comment.isEmpty()) {
        commentLabel->setText(comment);
    } else {
        commentLabel->setText("No additional comments provided.");
        commentLabel->setStyleSheet("color: #6c757d; font-style: italic;");
    }
    commentLabel->setWordWrap(true);
    commentLabel->setStyleSheet(commentLabel->styleSheet() + " padding: 5px 0;");

    itemLayout->addLayout(headerLayout);
    itemLayout->addWidget(commentLabel);

    feedbackItem->setLayout(itemLayout);
    feedbackContentLayout->addWidget(feedbackItem);
}

QString HiringManagerPortal::generateStarString(double rating)
{
    QString stars;
    int fullStars = static_cast<int>(rating);
    bool hasHalfStar = (rating - fullStars) >= 0.5;
    
    for (int i = 0; i < fullStars; i++) {
        stars += "★";
    }
    
    if (hasHalfStar) {
        stars += "☆"; // You could use a half-star character if available
    }
    
    int remainingStars = 5 - fullStars - (hasHalfStar ? 1 : 0);
    for (int i = 0; i < remainingStars; i++) {
        stars += "☆";
    }
    
    return stars;
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
    request["type"] = "updateJobStatus";
    request["freelancerId"] = freelancerId;
    request["jobId"] = jobId;
    request["status"] = status;

    client->sendRequest(request, [this, status](const QJsonObject &response)
                        {
        if (response["status"].toString() == "success") {
            QString message = status;
            if (message == "accepted")
                message = "Proposal accepted successfully!";
            else
                message = "Proposal rejected successfully!";
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

// Add to HiringManagerPortal.cpp:
void HiringManagerPortal::completeJob(const QString &jobId,
                                      const QString &hiringManagerId,
                                      const QString &freelancerId,
                                      const QString &jobTitle,
                                      const QString &jobDescription,
                                      double budgetRequested)
{
    BackendClient *client = BackendClient::getInstance();

    // Call the backend to complete the job
    client->completeJob(jobId, hiringManagerId, freelancerId, jobTitle, jobDescription, budgetRequested,
                        [this, jobTitle](bool success)
                        {
                            if (success)
                            {
                                QMessageBox::information(this, "Job Completed",
                                                         QString("Job '%1' has been completed successfully!\n\n").arg(jobTitle) +
                                                             "The job has been moved to completed jobs for both you and the freelancer.\n" +
                                                             "You can now rate the freelancer in the Ratings & Reviews tab.");

                                // Refresh the proposals list to remove the completed job
                                loadProposalsForHiringManager();
                            }
                            else
                            {
                                QMessageBox::warning(this, "Error",
                                                     "Failed to complete the job. Please try again.");
                            }
                        });
}