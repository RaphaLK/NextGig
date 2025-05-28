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

    Freelancer *currentUser = UserManager::getInstance()->getCurrentFreelancer();;
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
    tabWidget->addTab(messagesTab, "Messages");

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
    
    QListWidget *appliedJobsList = new QListWidget();
    appliedJobsList->addItem("Frontend Developer at TechCorp");
    appliedJobsList->addItem("UI/UX Designer at DesignStudio");
    appliedJobsList->addItem("React Developer at WebSolutions");
    appliedJobsList->setStyleSheet("QListWidget::item { padding: 8px; }");
    
    appliedJobsLayout->addWidget(appliedJobsList);
    appliedJobsGroup->setLayout(appliedJobsLayout);
    
    // 2. Top Right: Approved Jobs
    QGroupBox *approvedJobsGroup = new QGroupBox("Approved Applications");
    approvedJobsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *approvedJobsLayout = new QVBoxLayout();
    
    QListWidget *approvedJobsList = new QListWidget();
    approvedJobsList->addItem("✓ React Native Developer at MobileApp Inc.");
    approvedJobsList->addItem("✓ Database Consultant at DataTech");
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
    
    // Set up connections between selected job and detail views
    connect(appliedJobsList, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *)
    {
        if (current) {
            QString jobTitle = current->text();
            jobTitleLabel->setText("Job Title: " + jobTitle);
            
            // Simulate different job details based on selection
            if (jobTitle.contains("Frontend")) {
                jobDescText->setPlainText("We're looking for an experienced Frontend Developer to join our team. "
                                         "The ideal candidate should have strong skills in React, Vue, or Angular.");
                paymentLabel->setText("Payment Offer: $40-50/hour");
                employerNameLabel->setText("Company: TechCorp");
                emailLabel->setText("Email: jobs@techcorp.com");
                companyInfoText->setPlainText("TechCorp is a leading software development company specializing in web applications. "
                                             "Founded in 2010, we have a team of over 100 developers worldwide.");
                proposalText->setPlainText("I believe my 5 years of experience in React and Vue make me an ideal candidate for this role. "
                                          "I've previously worked on similar projects and can deliver high-quality code on time.");
                requestedAmountLabel->setText("Your Requested Payment: $45/hour");
                statusLabel->setText("Status: Under Review");
                statusLabel->setStyleSheet("font-weight: bold; color: #ffc107; margin-top: 10px;");
            } else if (jobTitle.contains("UI/UX")) {
                jobDescText->setPlainText("DesignStudio is seeking a creative UI/UX Designer with experience in Figma and Adobe XD. "
                                         "You'll be responsible for creating intuitive user interfaces for web and mobile applications.");
                paymentLabel->setText("Payment Offer: Fixed $5,000");
                employerNameLabel->setText("Company: DesignStudio");
                emailLabel->setText("Email: careers@designstudio.com");
                companyInfoText->setPlainText("DesignStudio is a creative agency focused on branding and digital product design. "
                                             "We work with startups and established companies to create beautiful, functional designs.");
                proposalText->setPlainText("I have a strong portfolio of UI/UX designs and would love to bring my aesthetic sense to your project. "
                                          "I specialize in creating designs that are both beautiful and highly usable.");
                requestedAmountLabel->setText("Your Requested Payment: $5,200");
                statusLabel->setText("Status: Pending");
                statusLabel->setStyleSheet("font-weight: bold; color: #ffc107; margin-top: 10px;");
            } else if (jobTitle.contains("React Developer")) {
                jobDescText->setPlainText("WebSolutions is looking for a React Developer to help build a new e-commerce platform. "
                                         "Experience with Redux, TypeScript, and responsive design is required.");
                paymentLabel->setText("Payment Offer: $35-45/hour");
                employerNameLabel->setText("Company: WebSolutions");
                emailLabel->setText("Email: hiring@websolutions.com");
                companyInfoText->setPlainText("WebSolutions builds custom web applications for clients across various industries. "
                                             "Our team is fully remote and collaborates using modern development practices.");
                proposalText->setPlainText("I've been working with React for 3 years and have built several e-commerce applications. "
                                          "I'm particularly strong with Redux state management and can start immediately.");
                requestedAmountLabel->setText("Your Requested Payment: $42/hour");
                statusLabel->setText("Status: Interview Scheduled");
                statusLabel->setStyleSheet("font-weight: bold; color: #17a2b8; margin-top: 10px;");
            }
        }
    });
    
    connect(approvedJobsList, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *)
    {
        if (current) {
            QString jobTitle = current->text().mid(2); // Remove the checkmark
            jobTitleLabel->setText("Job Title: " + jobTitle);
            
            // Show approved job details
            if (jobTitle.contains("React Native")) {
                jobDescText->setPlainText("MobileApp Inc. needs a React Native developer to help build a cross-platform mobile app. "
                                         "The project is expected to last 3 months with possibility of extension.");
                paymentLabel->setText("Payment Offer: $50/hour");
                employerNameLabel->setText("Company: MobileApp Inc.");
                emailLabel->setText("Email: dev@mobileapp.com");
                companyInfoText->setPlainText("MobileApp Inc. specializes in creating mobile applications for businesses. "
                                             "We're a small team of 15 developers focused on delivering high-quality apps.");
                proposalText->setPlainText("I have extensive experience with React Native and have published several apps to both App Store and Play Store. "
                                          "I can commit to your timeline and deliver a polished product.");
                requestedAmountLabel->setText("Your Requested Payment: $48/hour");
                statusLabel->setText("Status: Approved ✓");
                statusLabel->setStyleSheet("font-weight: bold; color: #28a745; margin-top: 10px;");
            } else if (jobTitle.contains("Database")) {
                jobDescText->setPlainText("DataTech is looking for a database consultant to optimize their PostgreSQL setup. "
                                         "This is a short-term contract expected to last 2-4 weeks.");
                paymentLabel->setText("Payment Offer: $4,000 fixed");
                employerNameLabel->setText("Company: DataTech");
                emailLabel->setText("Email: consulting@datatech.com");
                companyInfoText->setPlainText("DataTech provides data management solutions for enterprise clients. "
                                             "The company has been in business for over 15 years and has a global presence.");
                proposalText->setPlainText("As a database administrator with 8+ years of experience, I can identify and resolve your performance issues quickly. "
                                          "I've worked on similar optimization projects with great results.");
                requestedAmountLabel->setText("Your Requested Payment: $4,200");
                statusLabel->setText("Status: Approved ✓");
                statusLabel->setStyleSheet("font-weight: bold; color: #28a745; margin-top: 10px;");
            }
        }
    });
    
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
    
    return messagesWidget;
}