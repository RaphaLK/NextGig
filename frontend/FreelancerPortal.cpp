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
    QVBoxLayout *messagesLayout = new QVBoxLayout(messagesWidget);

    // Messages inbox
    QGroupBox *inboxGroup = new QGroupBox("Inbox");
    QVBoxLayout *inboxLayout = new QVBoxLayout();

    QListWidget *messagesList = new QListWidget();
    messagesList->addItem("Job proposal response from TechCorp");
    messagesList->addItem("Follow-up from DesignStudio about your application");
    messagesList->addItem("Rating received for completed project");

    inboxLayout->addWidget(messagesList);
    inboxGroup->setLayout(inboxLayout);

    // Message details
    QGroupBox *messageDetailsGroup = new QGroupBox("Message");
    QVBoxLayout *messageDetailsLayout = new QVBoxLayout();

    QLabel *messageFromLabel = new QLabel("From: ");
    QLabel *messageSubjectLabel = new QLabel("Subject: ");
    QTextEdit *messageBodyText = new QTextEdit();
    messageBodyText->setReadOnly(true);

    messageDetailsLayout->addWidget(messageFromLabel);
    messageDetailsLayout->addWidget(messageSubjectLabel);
    messageDetailsLayout->addWidget(messageBodyText);
    messageDetailsGroup->setLayout(messageDetailsLayout);

    // Reply button
    QPushButton *replyBtn = new QPushButton("Reply");
    replyBtn->setStyleSheet("padding: 8px; background-color: #007BFF; color: white;");

    // Connect message selection
    connect(messagesList, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *)
            {
        if (current) {
            messageFromLabel->setText("From: " + current->text().split(" ").back());
            messageSubjectLabel->setText("Subject: " + current->text());
            messageBodyText->setPlainText("This is the body of the selected message. In a real application, "
                                         "this would contain the actual message content from the sender.");
        } });

    // Layout
    messagesLayout->addWidget(inboxGroup, 2);
    messagesLayout->addWidget(messageDetailsGroup, 3);
    messagesLayout->addWidget(replyBtn, 0, Qt::AlignRight);

    return messagesWidget;
}