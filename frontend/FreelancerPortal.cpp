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
#include <QButtonGroup>

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
    QTimer::singleShot(1500, this, &FreelancerPortal::loadCompletedJobs);
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
    setupCompletedJobsTab();
    setupRatingsTab(); 
    setupProfileTab();
    
    mainLayout->addWidget(tabWidget);
}

void FreelancerPortal::updateRatingPanelDetails(const QJsonObject &profile)
{
    QString name = profile["name"].toString();
    QString email = profile["email"].toString();
    
    ratingHmNameLabel->setText(name.isEmpty() ? "Name not available" : "Rate: " + name);
    ratingHmEmailLabel->setText(email.isEmpty() ? "" : "Email: " + email);
    
    // Reset rating form
    ratingButtons->setExclusive(false);
    for (auto button : ratingButtons->buttons()) {
        button->setChecked(false);
        button->setEnabled(true);
    }
    ratingButtons->setExclusive(true);
    
    ratingCommentEdit->clear();
    ratingCommentEdit->setEnabled(true);
    currentRating = 0;
    submitRatingBtn->setEnabled(false);
    submitRatingBtn->setText("Submit Rating");
    
    // Check for existing rating
    checkFreelancerExistingRating(QString::fromStdString(currentCompletedJob["hiringManagerId"].toString().toStdString()));
}

void FreelancerPortal::checkFreelancerExistingRating(const QString &hiringManagerId)
{
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        return;
    }
    
    QString freelancerId = QString::fromStdString(freelancer->getUid());
    
    // Get the hiring manager's profile to check existing ratings
    BackendClient::getInstance()->getHiringManagerProfile(hiringManagerId,
        [this, freelancerId](bool success, const QJsonArray &profileArray) {
            if (!success || profileArray.isEmpty()) {
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            QJsonArray ratings = profile["ratings"].toArray();
            
            // Check if this freelancer has already rated this hiring manager
            bool hasRated = false;
            int existingRating = 0;
            QString existingComment;
            
            for (const auto &ratingValue : ratings) {
                QJsonObject rating = ratingValue.toObject();
                if (rating["fromUserId"].toString() == freelancerId) {
                    hasRated = true;
                    existingRating = rating["rating"].toInt();
                    existingComment = rating["comment"].toString();
                    break;
                }
            }
            
            if (hasRated) {
                // Pre-fill the form with existing rating
                QAbstractButton *button = ratingButtons->button(existingRating);
                if (button) {
                    button->setChecked(true);
                    currentRating = existingRating;
                }
                ratingCommentEdit->setText(existingComment);
                submitRatingBtn->setText("Update Rating");
                submitRatingBtn->setEnabled(true);
                
                // Add a note that this is an update
                QLabel *updateNote = new QLabel("Note: You have already rated this hiring manager. Submitting will update your previous rating.");
                updateNote->setStyleSheet("color: #6c757d; font-style: italic; margin: 5px 0;");
                updateNote->setWordWrap(true);
                
                // Add the note to the rating group layout
                QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ratingGroup->layout());
                if (layout) {
                    layout->insertWidget(layout->count() - 1, updateNote); // Insert before the stretch
                }
            } else {
                submitRatingBtn->setText("Submit Rating");
            }
        });
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
    
    // Education - Handle empty fields properly
    User::education edu = freelancer->getEducation();
    QString schoolText = QString::fromStdString(edu.school);
    QString degreeText = QString::fromStdString(edu.degreeLvl);
    
    profileEducationLabel->setText(schoolText.isEmpty() ? "No institution added yet" : schoolText);
    profileDegreeLabel->setText(degreeText.isEmpty() ? "No degree added yet" : degreeText);
    
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
    
    if (!current) {
        hasSelectedAppliedJob = false;
        return;
    }
    
    // Get job data from item
    QVariant jobData = current->data(Qt::UserRole);
    if (!jobData.isValid()) {
        hasSelectedAppliedJob = false;
        return;
    }
    
    currentSelectedAppliedJob = jobData.toJsonObject();
    hasSelectedAppliedJob = true;
    
    // Update job info panel with applied job details - FIX: Use correct variable names
    if (jobInfoTitleLabel && jobInfoDescriptionLabel && jobInfoPaymentLabel && jobInfoSkillsLabel) {
        jobInfoTitleLabel->setText(currentSelectedAppliedJob["jobTitle"].toString());
        jobInfoDescriptionLabel->setText(currentSelectedAppliedJob["jobDescription"].toString());
        
        // Show employer information
        jobInfoEmployerLabel->setText("Employer: " + currentSelectedAppliedJob["employerName"].toString());
        
        // FIX: Show payment from the job, not budget request
        QString payment = currentSelectedAppliedJob["payment"].toString();
        if (payment.isEmpty() || payment == "Payment not specified") {
            // Fallback to budget request if payment is not available
            QString budgetRequest = currentSelectedAppliedJob["budgetRequest"].toString();
            jobInfoPaymentLabel->setText(QString("Your Bid: %1").arg(budgetRequest));
        } else {
            jobInfoPaymentLabel->setText(QString("Job Payment: %1").arg(payment));
        }
        
        // Show required skills
        QJsonArray skillsArray = currentSelectedAppliedJob["requiredSkills"].toArray();
        QStringList skillsList;
        for (const auto &skill : skillsArray) {
            skillsList << skill.toString();
        }
        jobInfoSkillsLabel->setText(skillsList.isEmpty() ? "No specific skills required" : skillsList.join(" • "));
        
        // Show date if available
        if (jobInfoDateLabel) {
            jobInfoDateLabel->setText("Posted: " + currentSelectedAppliedJob["dateCreated"].toString());
        }
        
        // Show status
        QString status = currentSelectedAppliedJob["status"].toString();
        QString statusText = QString("Status: %1").arg(status.toUpper());
        if (status == "accepted") {
            statusText += " ✓";
        } else if (status == "rejected") {
            statusText += " ✗";
        } else {
            statusText += " ⏳";
        }
        qDebug() << "Applied job status:" << statusText;
    }
    
    // Update hiring manager info panel
    QString employerId = currentSelectedAppliedJob["employerId"].toString();
    if (!employerId.isEmpty() && employerId != "Unknown") {
        loadHiringManagerInfo(employerId);
    } else {
        // Clear hiring manager info if employer is unknown
        clearHiringManagerDetails();
    }
}

void FreelancerPortal::onApprovedJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current) {
        hasSelectedApprovedJob = false;
        return;
    }
    
    // Get job data from item
    QVariant jobData = current->data(Qt::UserRole);
    if (!jobData.isValid()) {
        hasSelectedApprovedJob = false;
        return;
    }
    
    currentSelectedApprovedJob = jobData.toJsonObject();
    hasSelectedApprovedJob = true;
    
    // Update job info panel with approved job details - FIX: Use correct variable names
    if (jobInfoTitleLabel && jobInfoDescriptionLabel && jobInfoPaymentLabel && jobInfoSkillsLabel) {
        jobInfoTitleLabel->setText(currentSelectedApprovedJob["jobTitle"].toString());
        jobInfoDescriptionLabel->setText(currentSelectedApprovedJob["jobDescription"].toString());
        
        // Show employer information
        jobInfoEmployerLabel->setText("Employer: " + currentSelectedApprovedJob["employerName"].toString());
        
        // FIX: Show payment from the job, not just budget request
        QString payment = currentSelectedApprovedJob["payment"].toString();
        QString budgetRequest = currentSelectedApprovedJob["budgetRequest"].toString();
        
        if (!payment.isEmpty() && payment != "Payment not specified") {
            jobInfoPaymentLabel->setText(QString("Job Payment: %1 (Your Bid: %2)").arg(payment, budgetRequest));
        } else {
            jobInfoPaymentLabel->setText(QString("Your Accepted Bid: %1").arg(budgetRequest));
        }
        
        // Show required skills
        QJsonArray skillsArray = currentSelectedApprovedJob["requiredSkills"].toArray();
        QStringList skillsList;
        for (const auto &skill : skillsArray) {
            skillsList << skill.toString();
        }
        jobInfoSkillsLabel->setText(skillsList.isEmpty() ? "No specific skills required" : skillsList.join(" • "));
        
        // Show date if available
        if (jobInfoDateLabel) {
            jobInfoDateLabel->setText("Posted: " + currentSelectedApprovedJob["dateCreated"].toString());
        }
    }
    
    // Update hiring manager info panel
    QString employerId = currentSelectedApprovedJob["employerId"].toString();
    if (!employerId.isEmpty() && employerId != "Unknown") {
        loadHiringManagerInfo(employerId);
    } else {
        // Clear hiring manager info if employer is unknown
        clearHiringManagerDetails();
    }
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

void FreelancerPortal::setupCompletedJobsTab()
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
    
    completedJobsList = new QListWidget();
    completedJobsList->setStyleSheet(
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #e0e0e0; }"
        "QListWidget::item:selected { background-color: #4a5759; }"
        "QListWidget::item:hover { background-color: #b0c4b1; }"
    );
    completedLayout->addWidget(completedJobsList);
    completedJobsGroup->setLayout(completedLayout);
    
    leftLayout->addWidget(completedJobsGroup);
    
    // Right side: Job details and rating panel
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    // Job Information section for completed jobs
    completedJobInfoGroup = new QGroupBox("Completed Job Information");
    completedJobInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupCompletedJobInfoPanel();
    
    // Hiring Manager Rating section
    ratingGroup = new QGroupBox("Rate Hiring Manager");
    ratingGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    setupRatingPanel();
    
    rightLayout->addWidget(completedJobInfoGroup);
    rightLayout->addWidget(ratingGroup);
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 500});
    
    mainLayout->addWidget(mainSplitter);
    
    // Connect signals
    connect(completedJobsList, &QListWidget::currentItemChanged, 
            this, &FreelancerPortal::onCompletedJobSelected);
    
    // Add refresh button
    QPushButton *refreshButton = new QPushButton("Refresh Completed Jobs");
    refreshButton->setStyleSheet("background-color: #28a745; color: white; padding: 8px; border-radius: 4px;");
    connect(refreshButton, &QPushButton::clicked, [this]() {
        loadCompletedJobs();
    });
    mainLayout->addWidget(refreshButton);
    
    tabWidget->addTab(completedJobsWidget, "Completed Jobs");
}

void FreelancerPortal::setupRatingsTab()
{
    QWidget *ratingsWidget = new QWidget();
    QVBoxLayout *ratingsLayout = new QVBoxLayout(ratingsWidget);

    // Title
    QLabel *titleLabel = new QLabel("My Ratings & Reviews");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    ratingsLayout->addWidget(titleLabel);

    // Your overall rating
    QGroupBox *overallGroup = new QGroupBox("Your Overall Rating");
    overallGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; margin: 10px 0; padding: 15px; }");
    QVBoxLayout *overallLayout = new QVBoxLayout();

    QHBoxLayout *starsLayout = new QHBoxLayout();
    freelancerOverallStarLabel = new QLabel("☆☆☆☆☆");
    freelancerOverallStarLabel->setStyleSheet("font-size: 32px; color: gold;");
    freelancerOverallRatingLabel = new QLabel("N/A");
    freelancerOverallRatingLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    starsLayout->addWidget(freelancerOverallStarLabel);
    starsLayout->addWidget(freelancerOverallRatingLabel);
    starsLayout->addStretch();

    QHBoxLayout *ratingDetailsLayout = new QHBoxLayout();
    QFormLayout *ratingBreakdownForm = new QFormLayout();

    freelancerTotalReviewsLabel = new QLabel("No reviews yet");

    ratingDetailsLayout->addLayout(ratingBreakdownForm);
    ratingDetailsLayout->addStretch();
    ratingDetailsLayout->addWidget(freelancerTotalReviewsLabel, 0, Qt::AlignBottom);

    overallLayout->addLayout(starsLayout);
    overallLayout->addLayout(ratingDetailsLayout);
    overallGroup->setLayout(overallLayout);

    // Recent feedback from hiring managers
    QGroupBox *feedbackGroup = new QGroupBox("Recent Feedback from Hiring Managers");
    feedbackGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; margin: 10px 0; padding: 15px; }");
    QVBoxLayout *feedbackLayout = new QVBoxLayout();
    
    // Scroll area for feedback
    QScrollArea *feedbackScrollArea = new QScrollArea();
    feedbackScrollArea->setWidgetResizable(true);
    feedbackScrollArea->setFrameShape(QFrame::NoFrame);
    
    freelancerFeedbackContentWidget = new QWidget();
    freelancerFeedbackContentLayout = new QVBoxLayout(freelancerFeedbackContentWidget);
    
    // Initially show loading message
    QLabel *loadingLabel = new QLabel("Loading feedback...");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("color: #6c757d; font-style: italic; padding: 20px;");
    freelancerFeedbackContentLayout->addWidget(loadingLabel);
    
    feedbackScrollArea->setWidget(freelancerFeedbackContentWidget);
    feedbackLayout->addWidget(feedbackScrollArea);
    feedbackGroup->setLayout(feedbackLayout);

    // Arrange everything in the ratings tab
    ratingsLayout->addWidget(overallGroup);
    ratingsLayout->addWidget(feedbackGroup);

    // Add refresh button
    QPushButton *refreshRatingsButton = new QPushButton("Refresh Ratings");
    refreshRatingsButton->setStyleSheet("background-color: #17a2b8; color: white; padding: 8px; border-radius: 4px;");
    connect(refreshRatingsButton, &QPushButton::clicked, this, &FreelancerPortal::loadFreelancerRatings);
    ratingsLayout->addWidget(refreshRatingsButton);

    // Load ratings when tab is created
    QTimer::singleShot(500, this, &FreelancerPortal::loadFreelancerRatings);

    tabWidget->addTab(ratingsWidget, "My Ratings");
}

void FreelancerPortal::loadFreelancerRatings()
{
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        qDebug() << "No freelancer found for loading ratings";
        return;
    }
    
    QString freelancerId = QString::fromStdString(freelancer->getUid());
    qDebug() << "Loading ratings for freelancer:" << freelancerId;
    
    // Get the freelancer's profile to load ratings using UID directly
    QJsonObject request;
    request["type"] = "getProfile";
    request["uid"] = freelancerId; // Use UID directly
    
    BackendClient::getInstance()->sendRequest(request, [this](const QJsonObject &response) {
        if (response["status"].toString() == "success") {
            // Extract ratings from the response
            QJsonArray ratings;
            if (response.contains("ratings") && response["ratings"].isArray()) {
                ratings = response["ratings"].toArray();
            }
            qDebug() << "Received" << ratings.size() << "ratings for freelancer";
            updateFreelancerRatingsDisplay(ratings);
        } else {
            qDebug() << "Failed to load freelancer profile:" << response["error"].toString();
            updateFreelancerRatingsDisplay(QJsonArray()); // Show empty state
        }
    });
}

void FreelancerPortal::updateFreelancerRatingsDisplay(const QJsonArray &ratings)
{
    qDebug() << "Updating freelancer ratings display with" << ratings.size() << "ratings";
    
    // Clear existing feedback content
    QLayoutItem *child;
    while ((child = freelancerFeedbackContentLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    if (ratings.isEmpty()) {
        QLabel *noRatingsLabel = new QLabel("No ratings received yet.");
        noRatingsLabel->setAlignment(Qt::AlignCenter);
        noRatingsLabel->setStyleSheet("color: #6c757d; font-style: italic; padding: 20px;");
        freelancerFeedbackContentLayout->addWidget(noRatingsLabel);

        freelancerTotalReviewsLabel->setText("No reviews yet");
        
        return;
    }
    
    // Calculate overall rating properly
    double totalRating = 0.0;
    int ratingCount = ratings.size();
    
    for (const auto &ratingValue : ratings) {
        QJsonObject rating = ratingValue.toObject();
        totalRating += rating["rating"].toInt();
    }
    
    double averageRating = totalRating / ratingCount;
    
    qDebug() << "Calculated average rating:" << averageRating << "from" << ratingCount << "ratings";
    
    // Update overall rating display
    QString stars = generateFreelancerStarString(averageRating);
    freelancerOverallStarLabel->setText(stars);
    freelancerOverallRatingLabel->setText(QString::number(averageRating, 'f', 1));
    freelancerTotalReviewsLabel->setText(QString("Based on %1 review%2").arg(ratingCount).arg(ratingCount == 1 ? "" : "s"));
    
    
    // Add individual feedback items
    for (const auto &ratingValue : ratings) {
        QJsonObject rating = ratingValue.toObject();
        addFreelancerFeedbackItem(rating);
    }
    
    freelancerFeedbackContentLayout->addStretch();
}

void FreelancerPortal::addFreelancerFeedbackItem(const QJsonObject &rating)
{
    QGroupBox *feedbackItem = new QGroupBox();
    feedbackItem->setStyleSheet("QGroupBox { border: 1px solid #dee2e6; border-radius: 6px; margin: 5px 0; padding-top: 10px; }");
    QVBoxLayout *itemLayout = new QVBoxLayout();

    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    // Get hiring manager name using fromUserId
    QString fromUserId = rating["fromUserId"].toString();
    QLabel *nameLabel = new QLabel("Loading..."); // Default name
    nameLabel->setStyleSheet("font-weight: bold;");
    
    // Fetch the actual hiring manager name using getProfile
    if (!fromUserId.isEmpty()) {
        QJsonObject request;
        request["type"] = "getProfile";
        request["uid"] = fromUserId; // Use UID for direct lookup
        
        BackendClient::getInstance()->sendRequest(request, [nameLabel](const QJsonObject &response) {
            if (response["status"].toString() == "success") {
                QString hiringManagerName = response["name"].toString();
                if (hiringManagerName.isEmpty()) {
                    hiringManagerName = response["username"].toString();
                }
                if (hiringManagerName.isEmpty()) {
                    hiringManagerName = "Unknown Hiring Manager";
                }
                nameLabel->setText(hiringManagerName);
            } else {
                nameLabel->setText("Unknown Hiring Manager");
            }
        });
    }
    
    int ratingValue = rating["rating"].toInt();
    QString starString = generateFreelancerStarString(ratingValue);
    QLabel *starLabel = new QLabel(starString);
    starLabel->setStyleSheet("color: gold; font-size: 16px;");
    
    // Add timestamp if available
    QLabel *timeLabel = new QLabel();
    if (rating.contains("timestamp")) {
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
    freelancerFeedbackContentLayout->addWidget(feedbackItem);
}

void FreelancerPortal::setupCompletedJobInfoPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(completedJobInfoGroup);
    
    // Create scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    
    // Job details labels for completed jobs
    completedJobTitleLabel = new QLabel("Select a completed job to view details");
    completedJobTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    completedJobTitleLabel->setWordWrap(true);
    
    completedJobEmployerLabel = new QLabel();
    completedJobEmployerLabel->setStyleSheet("color: #495057; font-size: 14px;");
    
    completedJobPaymentLabel = new QLabel();
    completedJobPaymentLabel->setStyleSheet("color: #28a745; font-size: 14px; font-weight: bold;");

    completedRatingLabel = new QLabel();
    completedRatingLabel->setStyleSheet("color:rgb(255, 215, 0); font-size: 14px; font-weight: bold;");

    completedJobDateLabel = new QLabel();
    completedJobDateLabel->setStyleSheet("color: #6c757d; font-size: 12px;");
    
    completedJobDescriptionLabel = new QLabel();
    completedJobDescriptionLabel->setWordWrap(true);
    completedJobDescriptionLabel->setStyleSheet(
        "background-color: #f8f9fa; border-radius: 6px; padding: 10px; margin: 5px 0;"
    );
    
    completedJobStatusLabel = new QLabel();
    completedJobStatusLabel->setStyleSheet("color: #28a745; font-weight: bold; font-size: 14px;");
    
    contentLayout->addWidget(completedJobTitleLabel);
    contentLayout->addWidget(completedJobEmployerLabel);
    contentLayout->addWidget(completedJobPaymentLabel);
    contentLayout->addWidget(completedRatingLabel);
    contentLayout->addWidget(completedJobDateLabel);
    contentLayout->addWidget(new QLabel("Description:"));
    contentLayout->addWidget(completedJobDescriptionLabel);
    contentLayout->addWidget(completedJobStatusLabel);
    contentLayout->addStretch();
    
    scrollArea->setWidget(content);
    layout->addWidget(scrollArea);
}

void FreelancerPortal::setupRatingPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(ratingGroup);
    
    // Hiring manager info
    ratingHmNameLabel = new QLabel("Select a completed job to rate the hiring manager");
    ratingHmNameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    layout->addWidget(ratingHmNameLabel);
    
    ratingHmEmailLabel = new QLabel();
    ratingHmEmailLabel->setStyleSheet("color: #495057; font-size: 14px;");
    layout->addWidget(ratingHmEmailLabel);
    
    // Rating section
    QLabel *ratingLabel = new QLabel("Rate your experience:");
    ratingLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(ratingLabel);
    
    // Star rating buttons
    QHBoxLayout *starsLayout = new QHBoxLayout();
    ratingButtons = new QButtonGroup(this);
    
    for (int i = 1; i <= 5; i++) {
        QPushButton *starBtn = new QPushButton(QString("★ %1").arg(i));
        starBtn->setCheckable(true);
        starBtn->setStyleSheet(
            "QPushButton { background-color: #f8f9fa; border: 2px solid #dee2e6; "
            "padding: 8px; border-radius: 4px; font-size: 14px; }"
            "QPushButton:checked { background-color: #ffc107; border-color: #ffc107; color: white; }"
            "QPushButton:hover { background-color: #e2e6ea; }"
        );
        ratingButtons->addButton(starBtn, i);
        starsLayout->addWidget(starBtn);
    }
    layout->addLayout(starsLayout);
    
    // Comment section
    QLabel *commentLabel = new QLabel("Additional comments (optional):");
    commentLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(commentLabel);
    
    ratingCommentEdit = new QTextEdit();
    ratingCommentEdit->setMaximumHeight(100);
    ratingCommentEdit->setPlaceholderText("Share your experience working with this hiring manager...");
    ratingCommentEdit->setStyleSheet("border: 1px solid #dee2e6; border-radius: 4px; padding: 8px;");
    layout->addWidget(ratingCommentEdit);
    
    // Submit rating button
    submitRatingBtn = new QPushButton("Submit Rating");
    submitRatingBtn->setStyleSheet(
        "background-color: #007bff; color: white; padding: 10px 20px; "
        "font-weight: bold; border-radius: 6px; font-size: 14px;"
    );
    submitRatingBtn->setEnabled(false);
    connect(submitRatingBtn, &QPushButton::clicked, this, &FreelancerPortal::submitRating);
    layout->addWidget(submitRatingBtn);
    
    // Connect rating buttons
    connect(ratingButtons, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int rating) {
                currentRating = rating;
                submitRatingBtn->setEnabled(rating > 0);
            });
    
    layout->addStretch();
}
void FreelancerPortal::loadCompletedJobs()
{
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        return;
    }
    
    QString freelancerId = QString::fromStdString(freelancer->getUid());
    
    completedJobsList->clear();
    completedJobsList->addItem("Loading completed jobs...");
    
    BackendClient::getInstance()->getCompletedJobs(freelancerId, false, // false = as freelancer
        [this](bool success, const QJsonArray &jobs) {
            completedJobsList->clear();
            
            if (!success) {
                completedJobsList->addItem("Failed to load completed jobs");
                return;
            }
            
            if (jobs.isEmpty()) {
                completedJobsList->addItem("No completed jobs found");
                return;
            }
            
            for (const auto &jobValue : jobs) {
                QJsonObject jobObj = jobValue.toObject();
                
                // Try both possible field names for job title
                QString jobTitle = jobObj["jobName"].toString();
                if (jobTitle.isEmpty()) {
                    jobTitle = jobObj["jobTitle"].toString();
                }
                
                QString hiringManagerId = jobObj["hiringManagerId"].toString();
                
                // Get budget - handle both string and number formats
                QString payment = jobObj["budgetRequested"].toString();
                if (payment.isEmpty()) {
                    double budgetValue = jobObj["budgetRequested"].toDouble();
                    payment = QString::number(budgetValue);
                }
                
                QString displayText = QString("%1\nHiring Manager: %2\nPayment: $%3")
                                       .arg(jobTitle)
                                       .arg(hiringManagerId.isEmpty() ? "Unknown" : "Loading...")
                                       .arg(payment);
                
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, jobValue);
                completedJobsList->addItem(item);
                
                // Fetch hiring manager name for display
                if (!hiringManagerId.isEmpty()) {
                    BackendClient::getInstance()->getHiringManagerProfile(hiringManagerId,
                        [this, item](bool success, const QJsonArray &profileArray) {
                            if (success && !profileArray.isEmpty()) {
                                QJsonObject profile = profileArray[0].toObject();
                                QString hiringManagerName = profile["name"].toString();
                                if (hiringManagerName.isEmpty()) {
                                    hiringManagerName = profile["username"].toString();
                                }
                                if (hiringManagerName.isEmpty()) {
                                    hiringManagerName = "Unknown Manager";
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
                                
                                QString updatedText = QString("%1\nHiring Manager: %2\nPayment: $%3")
                                                       .arg(jobTitle)
                                                       .arg(hiringManagerName)
                                                       .arg(payment);
                                
                                item->setText(updatedText);
                            }
                        });
                }
            }
        });
}

void FreelancerPortal::onCompletedJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    
    if (!current || current->data(Qt::UserRole).isNull()) {
        clearCompletedJobDetails();
        clearRatingPanel();
        return;
    }
    
    QJsonObject jobObj = current->data(Qt::UserRole).toJsonObject();
    updateCompletedJobDetails(jobObj);
    
    // Get the hiring manager ID
    QString hiringManagerId = jobObj["hiringManagerId"].toString();
    loadHiringManagerForRating(hiringManagerId, jobObj);
}

// void FreelancerPortal::updateCompletedJobDetails(const QJsonObject &jobObj, const QJsonObject &hiringManagerObj)
void FreelancerPortal::updateCompletedJobDetails(const QJsonObject &jobObj)
// {
//     qDebug() << "FreelancerPortal::updateCompletedJobDetails" << jobObj << Qt::endl;   
//     completedJobTitleLabel->setText(jobObj["jobName"].toString());
//     completedJobEmployerLabel->setText("Hiring Manager: " + jobObj["hiringManagerName"].toString());
//     completedJobPaymentLabel->setText("Payment: $" + jobObj["budgetRequested"].toString());
//     // completedRatingLabel->setText("Rating: ");

//     completedJobDescriptionLabel->setText(jobObj["jobDescription"].toString());
//     completedJobStatusLabel->setText("Status: ✓ Completed");
    
//     // You might want to add completion date if available
//     if (jobObj.contains("completedAt")) {
//         completedJobDateLabel->setText("Completed: " + jobObj["completedAt"].toString());
//     } else {
//         completedJobDateLabel->setText("Completion date not available");
//     }
// }
{
    qDebug() << "FreelancerPortal::updateCompletedJobDetails" << jobObj << Qt::endl;   
    
    // Get job title - try both possible field names
    QString jobTitle = jobObj["jobName"].toString();
    if (jobTitle.isEmpty()) {
        jobTitle = jobObj["jobTitle"].toString();
    }
    
    completedJobTitleLabel->setText(jobTitle);
    
    // For now, show the hiring manager ID until we fetch the name
    QString hiringManagerId = jobObj["hiringManagerId"].toString();
    completedJobEmployerLabel->setText("Hiring Manager: Loading...");
    
    // Fix: Use "budgetRequested" (with 'ed') instead of "budgetRequest"
    QString budget = jobObj["budgetRequested"].toString();
    if (budget.isEmpty()) {
        // Fallback to number format
        double budgetValue = jobObj["budgetRequested"].toDouble();
        budget = QString::number(budgetValue);
    }
    completedJobPaymentLabel->setText("Payment: $" + budget);
    
    completedJobDescriptionLabel->setText(jobObj["jobDescription"].toString());
    completedJobStatusLabel->setText("Status: ✓ Completed");
    
    // You might want to add completion date if available
    if (jobObj.contains("completedAt")) {
        completedJobDateLabel->setText("Completed: " + jobObj["completedAt"].toString());
    } else {
        completedJobDateLabel->setText("Completion date not available");
    }
    
    // Now fetch the hiring manager name
    if (!hiringManagerId.isEmpty()) {
        BackendClient::getInstance()->getHiringManagerProfile(hiringManagerId,
            [this](bool success, const QJsonArray &profileArray) {
                if (success && !profileArray.isEmpty()) {
                    QJsonObject profile = profileArray[0].toObject();
                    QString hiringManagerName = profile["name"].toString();
                    if (hiringManagerName.isEmpty()) {
                        hiringManagerName = profile["username"].toString();
                    }
                    if (hiringManagerName.isEmpty()) {
                        hiringManagerName = "Unknown Manager";
                    }
                    
                    // Update the label with the actual name
                    completedJobEmployerLabel->setText("Hiring Manager: " + hiringManagerName);
                } else {
                    completedJobEmployerLabel->setText("Hiring Manager: Unknown");
                }
            });
    }
}

void FreelancerPortal::loadHiringManagerForRating(const QString &hiringManagerId, const QJsonObject &jobObj)
{
    if (hiringManagerId.isEmpty()) {
        clearRatingPanel();
        return;
    }
    
    // Store current job data for rating submission
    currentCompletedJob = jobObj;
    
    // Show loading state
    ratingHmNameLabel->setText("Loading hiring manager details...");
    ratingHmEmailLabel->setText("");
    
    BackendClient::getInstance()->getHiringManagerProfile(hiringManagerId,
        [this](bool success, const QJsonArray &profileArray) {
            if (!success || profileArray.isEmpty()) {
                ratingHmNameLabel->setText("Hiring manager details not available");
                ratingHmEmailLabel->setText("");
                return;
            }
            
            QJsonObject profile = profileArray[0].toObject();
            updateRatingPanelDetails(profile);
        });
}

void FreelancerPortal::submitRating()
{
    if (currentRating == 0 || currentCompletedJob.isEmpty()) {
        QMessageBox::warning(this, "Invalid Rating", "Please select a rating before submitting.");
        return;
    }
    
    UserManager *userManager = UserManager::getInstance();
    User *freelancer = userManager->getCurrentUser();
    
    if (!freelancer) {
        QMessageBox::warning(this, "Error", "User not found.");
        return;
    }
    
    QString fromUserId = QString::fromStdString(freelancer->getUid());
    QString hiringManagerId = currentCompletedJob["hiringManagerId"].toString();
    QString comment = ratingCommentEdit->toPlainText().trimmed();
    
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this, "Submit Rating",
        QString("Are you sure you want to submit a %1-star rating for this hiring manager?")
            .arg(currentRating),
        QMessageBox::Yes | QMessageBox::No);
    
    if (confirm == QMessageBox::Yes) {
        submitRatingBtn->setEnabled(false);
        submitRatingBtn->setText("Submitting...");
        
        BackendClient::getInstance()->rateUser(fromUserId, hiringManagerId, currentRating, comment,
            [this](bool success) {
                submitRatingBtn->setText("Submit Rating");
                
                if (success) {
                    QMessageBox::information(this, "Success", "Rating submitted successfully!");
                    
                    // Disable the rating form to prevent duplicate ratings
                    for (auto button : ratingButtons->buttons()) {
                        button->setEnabled(false);
                    }
                    ratingCommentEdit->setEnabled(false);
                    submitRatingBtn->setText("Rating Submitted");
                } else {
                    QMessageBox::warning(this, "Error", "Failed to submit rating. Please try again.");
                    submitRatingBtn->setEnabled(true);
                }
            });
    }
}

void FreelancerPortal::clearCompletedJobDetails()
{
    completedJobTitleLabel->setText("Select a completed job to view details");
    completedJobEmployerLabel->setText("");
    completedJobPaymentLabel->setText("");
    completedJobDateLabel->setText("");
    completedJobDescriptionLabel->setText("");
    completedJobStatusLabel->setText("");
}

void FreelancerPortal::clearRatingPanel()
{
    ratingHmNameLabel->setText("Select a completed job to rate the hiring manager");
    ratingHmEmailLabel->setText("");
    
    // Reset rating form
    ratingButtons->setExclusive(false);
    for (auto button : ratingButtons->buttons()) {
        button->setChecked(false);
        button->setEnabled(true);
    }
    ratingButtons->setExclusive(true);
    
    ratingCommentEdit->clear();
    ratingCommentEdit->setEnabled(true);
    currentRating = 0;
    submitRatingBtn->setEnabled(false);
    submitRatingBtn->setText("Submit Rating");
    
    currentCompletedJob = QJsonObject();
}

QString FreelancerPortal::generateFreelancerStarString(double rating)
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
