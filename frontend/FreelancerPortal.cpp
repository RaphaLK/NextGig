#include "FreelancerPortal.h"
#include "UserManager.h"
#include "FreelancerProfileEdit.h"
#include "client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QListWidget>
#include <QTextEdit>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QSplitter>
#include <QFrame>
#include <QFormLayout>
#include <QJsonObject>
#include <QJsonArray>

QDateTime JobFeed::lastRequestTime = QDateTime();


FreelancerPortal::FreelancerPortal(QWidget *parent)
    : QWidget(parent),
      currentSelectedJob(),
      hasSelectedJob(false),
      currentSelectedAppliedJob(),
      hasSelectedAppliedJob(false),
      currentSelectedApprovedJob(),
      hasSelectedApprovedJob(false)
{
    qDebug() << "FreelancerPortal constructor called - instance:" << this;
    if (uiSetup) {
        qDebug() << "setupUI already called, skipping";
        return;
    }
    uiSetup = true;
    setupUI();
    loadUserData();
    
    // Load data for current jobs tab
    QTimer::singleShot(500, this, &FreelancerPortal::loadAppliedJobs);
    QTimer::singleShot(1000, this, &FreelancerPortal::loadApprovedJobs);
}

void FreelancerPortal::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Setup navigation bar first
    setupNavigationBar();
    mainLayout->addWidget(navigationBar);
    
    // Create tab widget
    tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #c0c0c0; }"
        "QTabBar::tab { background: #f0f0f0; padding: 10px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #ffffff; border-bottom: 2px solid #007bff; }"
    );
    
    // Create the three tabs
    setupAvailableJobsTab();
    setupCurrentJobsTab();
    setupProfileTab();
    
    mainLayout->addWidget(tabWidget);
}

void FreelancerPortal::setupNavigationBar()
{
    navigationBar = new QWidget();
    navigationBar->setStyleSheet(
        "QWidget { background-color: #2c3e50; padding: 10px; }"
        "QLabel { color: white; font-weight: bold; font-size: 16px; }"
        "QPushButton { background-color: #e74c3c; color: white; padding: 8px 16px; "
        "border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );
    
    QHBoxLayout *navLayout = new QHBoxLayout(navigationBar);
    
    // Welcome message
    welcomeLabel = new QLabel();
    UserManager *userManager = UserManager::getInstance();
    Freelancer *freelancer = userManager->getCurrentFreelancer();
    if (freelancer) {
        welcomeLabel->setText(QString("Welcome, %1 - Freelancer Portal")
                              .arg(QString::fromStdString(freelancer->getName())));
    } else {
        welcomeLabel->setText("Freelancer Portal");
    }
    
    // Logout button
    logoutButton = new QPushButton("Logout");
    connect(logoutButton, &QPushButton::clicked, this, &FreelancerPortal::onLogoutClicked);
    
    navLayout->addWidget(welcomeLabel);
    navLayout->addStretch();
    navLayout->addWidget(logoutButton);
}

FreelancerPortal::~FreelancerPortal()
{
    if (availableJobsFeed) {
        availableJobsFeed->isDestroying = true;
    }
}

void FreelancerPortal::setupAvailableJobsTab()
{
    qDebug() << "setupAvailableJobsTab() called";
    
    // Only create if not already created
    if (availableJobsFeed) {
        qDebug() << "JobFeed already exists, reusing";
        return;
    }
    
    QWidget *availableJobsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(availableJobsWidget);
    
    // Add title
    QLabel *titleLabel = new QLabel("Available Jobs");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    // Create JobFeed only once
    availableJobsFeed = new JobFeed(availableJobsWidget, true);
    layout->addWidget(availableJobsFeed);
    
    tabWidget->addTab(availableJobsWidget, "Available Jobs");
}

void FreelancerPortal::setupCurrentJobsTab()
{
    QWidget *currentJobsWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(currentJobsWidget);
    
    // Title
    QLabel *titleLabel = new QLabel("Current Jobs");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Create main splitter for the layout
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left side: Jobs lists
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Applied Jobs section
    QGroupBox *appliedJobsGroup = new QGroupBox("Applied Jobs (Pending)");
    appliedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *appliedLayout = new QVBoxLayout();
    
    appliedJobsList = new QListWidget();
    appliedJobsList->setStyleSheet(
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #e0e0e0; }"
        "QListWidget::item:selected { background-color: #4a5759; }"
        "QListWidget::item:hover { background-color: #b0c4b1; }"
    );
    appliedLayout->addWidget(appliedJobsList);
    appliedJobsGroup->setLayout(appliedLayout);
    
    // Approved Jobs section
    QGroupBox *approvedJobsGroup = new QGroupBox("Approved Jobs (In Progress)");
    approvedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *approvedLayout = new QVBoxLayout();
    
    approvedJobsList = new QListWidget();
    approvedJobsList->setStyleSheet(
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #e0e0e0; }"
        "QListWidget::item:selected { background-color: #4a5759; }"
        "QListWidget::item:hover { background-color: #b0c4b1; }"
    );
    approvedLayout->addWidget(approvedJobsList);
    approvedJobsGroup->setLayout(approvedLayout);
    
    leftLayout->addWidget(appliedJobsGroup);
    leftLayout->addWidget(approvedJobsGroup);
    
    // Right side: Details panels
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    // Job Information section
    jobInfoGroup = new QGroupBox("Job Information");
    jobInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupJobInfoPanel();
    
    // Hiring Manager Information section
    hiringManagerInfoGroup = new QGroupBox("Hiring Manager Information");
    hiringManagerInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupHiringManagerInfoPanel();
    
    rightLayout->addWidget(jobInfoGroup);
    rightLayout->addWidget(hiringManagerInfoGroup);
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 500});
    
    mainLayout->addWidget(mainSplitter);
    
    // Connect signals
    connect(appliedJobsList, &QListWidget::currentItemChanged, 
            this, &FreelancerPortal::onAppliedJobSelected);
    connect(approvedJobsList, &QListWidget::currentItemChanged, 
            this, &FreelancerPortal::onApprovedJobSelected);
    
    // Add refresh button
    QPushButton *refreshButton = new QPushButton("Refresh Current Jobs");
    refreshButton->setStyleSheet("background-color: #17a2b8; color: white; padding: 8px; border-radius: 4px;");
    connect(refreshButton, &QPushButton::clicked, [this]() {
        loadAppliedJobs();
        loadApprovedJobs();
    });
    mainLayout->addWidget(refreshButton);
    
    tabWidget->addTab(currentJobsWidget, "Current Jobs");
}

void FreelancerPortal::setupJobInfoPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(jobInfoGroup);
    
    // Create scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    
    // Job details labels
    jobInfoTitleLabel = new QLabel("Select a job to view details");
    jobInfoTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    jobInfoTitleLabel->setWordWrap(true);
    
    jobInfoEmployerLabel = new QLabel();
    jobInfoEmployerLabel->setStyleSheet("color: #495057; font-size: 14px;");
    
    jobInfoPaymentLabel = new QLabel();
    jobInfoPaymentLabel->setStyleSheet("color: #28a745; font-size: 14px; font-weight: bold;");
    
    jobInfoDateLabel = new QLabel();
    jobInfoDateLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    
    jobInfoDescriptionLabel = new QLabel();
    jobInfoDescriptionLabel->setWordWrap(true);
    jobInfoDescriptionLabel->setStyleSheet(
        "background-color: #f8f9fa; border-radius: 6px; padding: 10px; margin: 5px 0;"
    );
    
    jobInfoSkillsLabel = new QLabel();
    jobInfoSkillsLabel->setWordWrap(true);
    jobInfoSkillsLabel->setStyleSheet(
        "background-color: #e9ecef; border-radius: 6px; padding: 8px; margin: 5px 0;"
    );
    
    contentLayout->addWidget(jobInfoTitleLabel);
    contentLayout->addWidget(jobInfoEmployerLabel);
    contentLayout->addWidget(jobInfoPaymentLabel);
    contentLayout->addWidget(jobInfoDateLabel);
    contentLayout->addWidget(new QLabel("Description:"));
    contentLayout->addWidget(jobInfoDescriptionLabel);
    contentLayout->addWidget(new QLabel("Required Skills:"));
    contentLayout->addWidget(jobInfoSkillsLabel);
    contentLayout->addStretch();
    
    scrollArea->setWidget(content);
    layout->addWidget(scrollArea);
}

void FreelancerPortal::setupHiringManagerInfoPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(hiringManagerInfoGroup);
    
    // Create scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    
    // Hiring manager details
    hmNameLabel = new QLabel("Select a job to view hiring manager details");
    hmNameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    
    hmEmailLabel = new QLabel();
    hmEmailLabel->setStyleSheet("color: #495057; font-size: 14px;");
    
    hmCompanyLabel = new QLabel();
    hmCompanyLabel->setStyleSheet("color: #007bff; font-size: 14px; font-weight: bold;");
    
    hmDescriptionLabel = new QLabel();
    hmDescriptionLabel->setWordWrap(true);
    hmDescriptionLabel->setStyleSheet(
        "background-color: #f8f9fa; border-radius: 6px; padding: 10px; margin: 5px 0;"
    );
    
    hmCompanyDescLabel = new QLabel();
    hmCompanyDescLabel->setWordWrap(true);
    hmCompanyDescLabel->setStyleSheet(
        "background-color: #e3f2fd; border-radius: 6px; padding: 10px; margin: 5px 0;"
    );
    
    contentLayout->addWidget(hmNameLabel);
    contentLayout->addWidget(hmEmailLabel);
    contentLayout->addWidget(hmCompanyLabel);
    contentLayout->addWidget(new QLabel("About:"));
    contentLayout->addWidget(hmDescriptionLabel);
    contentLayout->addStretch();
    
    scrollArea->setWidget(content);
    layout->addWidget(scrollArea);
}

void FreelancerPortal::setupProfileTab()
{
    QWidget *profileWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(profileWidget);
    
    // Title
    QLabel *titleLabel = new QLabel("My Profile");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    // Create scroll area for profile content
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *profileContent = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileContent);
    
    // Basic info section
    QGroupBox *basicInfoGroup = new QGroupBox("Basic Information");
    QFormLayout *basicInfoLayout = new QFormLayout();
    
    profileNameLabel = new QLabel();
    profileEmailLabel = new QLabel();
    profileHourlyRateLabel = new QLabel();
    profileDescriptionLabel = new QLabel();
    profileDescriptionLabel->setWordWrap(true);
    profileDescriptionLabel->setStyleSheet("background-color: #f8f9fa; padding: 10px; border-radius: 6px;");
    
    basicInfoLayout->addRow("Name:", profileNameLabel);
    basicInfoLayout->addRow("Email:", profileEmailLabel);
    basicInfoLayout->addRow("Hourly Rate:", profileHourlyRateLabel);
    basicInfoLayout->addRow("About Me:", profileDescriptionLabel);
    
    basicInfoGroup->setLayout(basicInfoLayout);
    
    // Skills section
    QGroupBox *skillsGroup = new QGroupBox("Skills & Expertise");
    QVBoxLayout *skillsLayout = new QVBoxLayout();
    
    profileSkillsLabel = new QLabel();
    profileSkillsLabel->setWordWrap(true);
    profileSkillsLabel->setStyleSheet("background-color: #e9ecef; padding: 10px; border-radius: 6px;");
    skillsLayout->addWidget(profileSkillsLabel);
    
    skillsGroup->setLayout(skillsLayout);
    
    // Education section
    QGroupBox *educationGroup = new QGroupBox("Education");
    QFormLayout *educationLayout = new QFormLayout();
    
    profileEducationLabel = new QLabel();
    profileDegreeLabel = new QLabel();
    
    educationLayout->addRow("Institution:", profileEducationLabel);
    educationLayout->addRow("Degree:", profileDegreeLabel);
    
    educationGroup->setLayout(educationLayout);
    
    // Accomplishments section
    QGroupBox *accomplishmentsGroup = new QGroupBox("Accomplishments");
    QVBoxLayout *accomplishmentsLayout = new QVBoxLayout();
    
    profileAccomplishmentsLabel = new QLabel();
    profileAccomplishmentsLabel->setWordWrap(true);
    profileAccomplishmentsLabel->setStyleSheet("background-color: #fff3cd; padding: 10px; border-radius: 6px;");
    accomplishmentsLayout->addWidget(profileAccomplishmentsLabel);
    
    accomplishmentsGroup->setLayout(accomplishmentsLayout);
    
    // Job History section
    QGroupBox *jobHistoryGroup = new QGroupBox("Job History");
    QVBoxLayout *jobHistoryLayout = new QVBoxLayout();
    
    profileJobHistoryList = new QListWidget();
    profileJobHistoryList->setMaximumHeight(150);
    profileJobHistoryList->setStyleSheet(
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #e0e0e0; }"
    );
    jobHistoryLayout->addWidget(profileJobHistoryList);
    
    jobHistoryGroup->setLayout(jobHistoryLayout);
    
    // Add all sections to profile layout
    profileLayout->addWidget(basicInfoGroup);
    profileLayout->addWidget(skillsGroup);
    profileLayout->addWidget(educationGroup);
    profileLayout->addWidget(accomplishmentsGroup);
    profileLayout->addWidget(jobHistoryGroup);
    profileLayout->addStretch();
    
    scrollArea->setWidget(profileContent);
    layout->addWidget(scrollArea);
    
    // Edit profile button
    QPushButton *editProfileBtn = new QPushButton("Edit Profile");
    editProfileBtn->setStyleSheet(
        "background-color: #007bff; color: white; padding: 12px 24px; "
        "font-weight: bold; border-radius: 6px; font-size: 14px;"
    );
    connect(editProfileBtn, &QPushButton::clicked, this, &FreelancerPortal::editProfile);
    layout->addWidget(editProfileBtn);
    
    tabWidget->addTab(profileWidget, "My Profile");
}

void FreelancerPortal::loadUserData()
{
    UserManager *userManager = UserManager::getInstance();
    Freelancer *freelancer = userManager->getCurrentFreelancer();
    
    if (!freelancer) {
        QMessageBox::warning(this, "Error", "No freelancer user found. Please log in.");
        return;
    }
    
    // Update welcome label
    if (welcomeLabel) {
        welcomeLabel->setText(QString("Welcome, %1 - Freelancer Portal")
                              .arg(QString::fromStdString(freelancer->getName())));
    }
    
    // Populate profile tab
    profileNameLabel->setText(QString::fromStdString(freelancer->getName()));
    profileEmailLabel->setText(QString::fromStdString(freelancer->getEmail()));
    profileHourlyRateLabel->setText(QString("$%1/hour").arg(freelancer->getHourlyRate(), 0, 'f', 2));
    profileDescriptionLabel->setText(QString::fromStdString(freelancer->getDescription()));
    
    // Skills
    QStringList skillsList;
    for (const auto &skill : freelancer->getSkills()) {
        skillsList << QString::fromStdString(skill);
    }
    profileSkillsLabel->setText(skillsList.isEmpty() ? "No skills added yet" : skillsList.join(" • "));
    
    // Education
    User::education edu = freelancer->getEducation();
    profileEducationLabel->setText(QString::fromStdString(edu.school));
    profileDegreeLabel->setText(QString::fromStdString(edu.degreeLvl));
    
    // Accomplishments
    QStringList accomplishmentsList;
    for (const auto &acc : freelancer->getAccomplishments()) {
        accomplishmentsList << QString::fromStdString(acc);
    }
    profileAccomplishmentsLabel->setText(accomplishmentsList.isEmpty() ? 
        "No accomplishments added yet" : accomplishmentsList.join("\n• "));
    
    // Job History
    profileJobHistoryList->clear();
    for (const auto &job : freelancer->getJobHistory()) {
        QString jobText = QString("%1 (%2 - %3)")
                           .arg(QString::fromStdString(job.jobTitle))
                           .arg(QString::fromStdString(job.startDate))
                           .arg(QString::fromStdString(job.endDate));
        profileJobHistoryList->addItem(jobText);
    }
    
    if (freelancer->getJobHistory().empty()) {
        profileJobHistoryList->addItem("No job history added yet");
    }
}

void FreelancerPortal::loadAppliedJobs()
{
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        return;
    }
    
    QString freelancerId = QString::fromStdString(freelancer->getUid());
    
    appliedJobsList->clear();
    appliedJobsList->addItem("Loading applied jobs...");
    
    BackendClient::getInstance()->getAppliedJobs(freelancerId, 
        [this](bool success, const QJsonArray &jobs) {
            appliedJobsList->clear();
            
            if (!success) {
                appliedJobsList->addItem("Failed to load applied jobs");
                return;
            }
            
            if (jobs.isEmpty()) {
                appliedJobsList->addItem("No applied jobs found");
                return;
            }
            
            for (const auto &jobValue : jobs) {
                QJsonObject jobObj = jobValue.toObject();
                QString jobTitle = jobObj["jobTitle"].toString();
                QString employer = jobObj["employerName"].toString();
                QString status = jobObj["status"].toString("pending");
                
                QString displayText = QString("%1\nEmployer: %2\nStatus: %3")
                                       .arg(jobTitle)
                                       .arg(employer)
                                       .arg(status);
                
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, jobValue);
                appliedJobsList->addItem(item);
            }
        });
}

void FreelancerPortal::loadApprovedJobs()
{
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        return;
    }
    
    QString freelancerId = QString::fromStdString(freelancer->getUid());
    
    approvedJobsList->clear();
    approvedJobsList->addItem("Loading approved jobs...");
    
    BackendClient::getInstance()->getApprovedJobs(freelancerId, 
        [this](bool success, const QJsonArray &jobs) {
            approvedJobsList->clear();
            
            if (!success) {
                approvedJobsList->addItem("Failed to load approved jobs");
                return;
            }
            
            if (jobs.isEmpty()) {
                approvedJobsList->addItem("No approved jobs found");
                return;
            }
            
            for (const auto &jobValue : jobs) {
                QJsonObject jobObj = jobValue.toObject();
                QString jobTitle = jobObj["jobTitle"].toString();
                QString employer = jobObj["employerName"].toString();
                QString payment = jobObj["payment"].toString();
                
                QString displayText = QString("%1\nEmployer: %2\nPayment: %3")
                                       .arg(jobTitle)
                                       .arg(employer)
                                       .arg(payment);
                
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, jobValue);
                approvedJobsList->addItem(item);
            }
        });
}

void FreelancerPortal::onAppliedJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current || current->data(Qt::UserRole).isNull()) {
        clearJobDetails();
        clearHiringManagerDetails();
        return;
    }
    
    QJsonObject jobObj = current->data(Qt::UserRole).toJsonObject();
    updateJobDetails(jobObj);

    qDebug() << Qt::endl << jobObj << Qt::endl;
    
    // Get the employer ID - try different field names its either username or user ID
    QString employerId = jobObj["employerId"].toString();
    if (employerId.isEmpty()) {
        employerId = jobObj["uid"].toString();
    }
    if (employerId.isEmpty()) {
        employerId = jobObj["employerUid"].toString();
    }
    
    qDebug() << "Loading hiring manager info for employerId:" << employerId;
    loadHiringManagerInfo(employerId);
}

void FreelancerPortal::onApprovedJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current || current->data(Qt::UserRole).isNull()) {
        clearJobDetails();
        clearHiringManagerDetails();
        return;
    }
    
    QJsonObject jobObj = current->data(Qt::UserRole).toJsonObject();
    updateJobDetails(jobObj);
    
    // Get the employer ID - try different field names
    QString employerId = jobObj["employerId"].toString();
    if (employerId.isEmpty()) {
        employerId = jobObj["uid"].toString();
    }
    if (employerId.isEmpty()) {
        employerId = jobObj["employerUid"].toString();
    }
    
    qDebug() << "Loading hiring manager info for employerId:" << employerId;
    loadHiringManagerInfo(employerId);
}

void FreelancerPortal::loadHiringManagerInfo(const QString &employerId)
{
    if (employerId.isEmpty()) {
        qDebug() << "Empty employerId provided";
        clearHiringManagerDetails();
        return;
    }
    
    qDebug() << "Fetching hiring manager profile for:" << employerId;
    
    // Show loading state
    hmNameLabel->setText("Loading hiring manager details...");
    hmEmailLabel->setText("");
    hmCompanyLabel->setText("");
    hmDescriptionLabel->setText("");
    hmCompanyDescLabel->setText("");
    
    BackendClient::getInstance()->getHiringManagerProfile(employerId,
        [this, employerId](bool success, const QJsonArray &profileArray) {
            qDebug() << "Hiring manager profile response - success:" << success << "array size:" << profileArray.size();
            
            if (!success || profileArray.isEmpty()) {
                qDebug() << "Failed to get hiring manager profile for:" << employerId;
                clearHiringManagerDetails();
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            qDebug() << "Received hiring manager profile:" << profile;
            updateHiringManagerDetails(profile);
        });
}

void FreelancerPortal::updateJobDetails(const QJsonObject &jobObj)
{
    jobInfoTitleLabel->setText(jobObj["jobTitle"].toString());
    jobInfoEmployerLabel->setText("Employer: " + jobObj["employerName"].toString());
    jobInfoPaymentLabel->setText("Payment: " + jobObj["payment"].toString());
    jobInfoDateLabel->setText("Posted: " + jobObj["dateCreated"].toString());
    jobInfoDescriptionLabel->setText(jobObj["jobDescription"].toString());
    
    // Handle skills array
    QJsonArray skillsArray = jobObj["requiredSkills"].toArray();
    QStringList skillsList;
    for (const auto &skill : skillsArray) {
        skillsList << skill.toString();
    }
    jobInfoSkillsLabel->setText(skillsList.isEmpty() ? "No specific skills required" : skillsList.join(" • "));
}

void FreelancerPortal::updateHiringManagerDetails(const QJsonObject &profile)
{
    qDebug() << "Updating hiring manager details with profile:" << profile;
    
    QString name = profile["name"].toString();
    QString email = profile["email"].toString();
    QString description = profile["description"].toString();
    
    hmNameLabel->setText(name.isEmpty() ? "Name not available" : name);
    hmEmailLabel->setText(email.isEmpty() ? "Email: Not available" : "Email: " + email);
    hmDescriptionLabel->setText(description.isEmpty() ? "No description available" : description);
}

void FreelancerPortal::clearHiringManagerDetails()
{
    hmNameLabel->setText("Select a job to view hiring manager details");
    hmEmailLabel->setText("");
    hmCompanyLabel->setText("");
    hmDescriptionLabel->setText("");
}
void FreelancerPortal::clearJobDetails()
{
    jobInfoTitleLabel->setText("Select a job to view details");
    jobInfoEmployerLabel->setText("");
    jobInfoPaymentLabel->setText("");
    jobInfoDateLabel->setText("");
    jobInfoDescriptionLabel->setText("");
    jobInfoSkillsLabel->setText("");
}

void FreelancerPortal::editProfile()
{
    UserManager *userManager = UserManager::getInstance();
    Freelancer *freelancer = userManager->getCurrentFreelancer();
    
    if (!freelancer) {
        QMessageBox::warning(this, "Error", "No freelancer user found.");
        return;
    }
    
    FreelancerProfileEdit dialog(freelancer, this);
    connect(&dialog, &FreelancerProfileEdit::profileUpdated, 
            this, &FreelancerPortal::onProfileUpdated);
    
    if (dialog.exec() == QDialog::Accepted) {
        loadUserData(); // Refresh profile display
    }
}

void FreelancerPortal::onProfileUpdated()
{
    loadUserData();
    QMessageBox::information(this, "Success", "Profile updated successfully!");
}

void FreelancerPortal::onLogoutClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Logout", 
        "Are you sure you want to logout?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // Clear user data
        UserManager::getInstance()->clearCurrentUser();
        
        // Sign out from backend
        BackendClient::getInstance()->signOut([this](bool success) {
            Q_UNUSED(success)
            // Return to home page regardless of backend result
            emit returnToHomeRequested();
        });
    }
}

void FreelancerPortal::refreshAllData()
{
    loadUserData();
    loadAppliedJobs();
    loadApprovedJobs();
}