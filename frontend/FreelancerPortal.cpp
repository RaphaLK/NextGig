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

FreelancerPortal::FreelancerPortal(QWidget *parent) : QWidget(parent), currentUser(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(renderFreelancerPortal());
    setLayout(mainLayout);
}

void FreelancerPortal::setCurrentUser(User* user) {
    currentUser = user;
    updateProfileInfo();
}

void FreelancerPortal::updateProfileInfo() {
    if (!currentUser) return;
    
    nameLabel->setText(QString::fromStdString(currentUser->getName()));
    descriptionTextEdit->setPlainText(QString::fromStdString(currentUser->getDescription()));
    
    // Update tags/skills
    skillsListWidget->clear();
    for (const auto& tag : currentUser->getTags()) {
        skillsListWidget->addItem(QString::fromStdString(tag));
    }
    
    // Update job history
    jobHistoryList->clear();
    for (const auto& job : currentUser->getJobHistory()) {
        QString jobEntry = QString::fromStdString(job.jobTitle) + " (" + 
                          QString::fromStdString(job.startDate) + " - " + 
                          QString::fromStdString(job.endDate) + ")";
        jobHistoryList->addItem(jobEntry);
    }
    
    // Update accomplishments
    accomplishmentsList->clear();
    for (const auto& accomplishment : currentUser->getAccomplishments()) {
        accomplishmentsList->addItem(QString::fromStdString(accomplishment));
    }
}

QWidget* FreelancerPortal::renderFreelancerPortal() {
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
    connect(logoutBtn, &QPushButton::clicked, [this](){
        BackendClient* client = BackendClient::getInstance();
        // Logout from server
        client->signOut([this](bool success) {
            if (success) {
                emit returnToHomeRequested();
            } else {
                QMessageBox::warning(this, "Logout Error", 
                                     "Failed to log out. Please try again.");
            }
        });
    });
    
    headerLayout->addWidget(welcomeLabel, 1);
    headerLayout->addWidget(logoutBtn, 0, Qt::AlignRight);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(tabWidget);
    
    QWidget *container = new QWidget();
    container->setLayout(mainLayout);
    return container;
}

QWidget* FreelancerPortal::createJobsTab() {
    QWidget *jobsWidget = new QWidget();
    QVBoxLayout *jobsLayout = new QVBoxLayout(jobsWidget);
    
    // Search and filter area
    QGroupBox *filterGroup = new QGroupBox("Filter Jobs");
    QHBoxLayout *filterLayout = new QHBoxLayout();
    
    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search by keyword...");
    
    QComboBox *skillFilter = new QComboBox();
    skillFilter->addItem("All Skills");
    skillFilter->addItem("Web Development");
    skillFilter->addItem("Mobile Development");
    skillFilter->addItem("UI/UX Design");
    skillFilter->addItem("Data Science");
    
    QPushButton *applyFilterBtn = new QPushButton("Apply Filter");
    applyFilterBtn->setStyleSheet("padding: 5px; background-color: #4CAF50; color: white;");
    
    filterLayout->addWidget(new QLabel("Search:"));
    filterLayout->addWidget(searchBox, 3);
    filterLayout->addWidget(new QLabel("Skill:"));
    filterLayout->addWidget(skillFilter, 2);
    filterLayout->addWidget(applyFilterBtn);
    
    filterGroup->setLayout(filterLayout);
    
    // Jobs list area
    QGroupBox *jobListGroup = new QGroupBox("Available Jobs");
    QVBoxLayout *jobListLayout = new QVBoxLayout();
    
    jobsList = new QListWidget();
    
    // Sample job data (would be populated from backend in real app)
    QListWidgetItem *job1 = new QListWidgetItem("Frontend Developer - React.js");
    QListWidgetItem *job2 = new QListWidgetItem("Backend Engineer - Node.js & MongoDB");
    QListWidgetItem *job3 = new QListWidgetItem("UI/UX Designer for Mobile App");
    QListWidgetItem *job4 = new QListWidgetItem("WordPress Developer for E-commerce Site");
    QListWidgetItem *job5 = new QListWidgetItem("Full Stack Developer - MERN Stack");
    
    jobsList->addItem(job1);
    jobsList->addItem(job2);
    jobsList->addItem(job3);
    jobsList->addItem(job4);
    jobsList->addItem(job5);
    
    jobListLayout->addWidget(jobsList);
    jobListGroup->setLayout(jobListLayout);
    
    // Job details area
    QGroupBox *jobDetailsGroup = new QGroupBox("Job Details");
    QVBoxLayout *detailsLayout = new QVBoxLayout();
    
    jobTitleLabel = new QLabel("Select a job to view details");
    jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    jobDescriptionLabel = new QLabel("");
    jobDescriptionLabel->setWordWrap(true);
    
    QPushButton *sendProposalBtn = new QPushButton("Send Proposal");
    sendProposalBtn->setStyleSheet("padding: 8px; background-color: #007BFF; color: white;");
    
    detailsLayout->addWidget(jobTitleLabel);
    detailsLayout->addWidget(jobDescriptionLabel);
    detailsLayout->addWidget(sendProposalBtn);
    jobDetailsGroup->setLayout(detailsLayout);
    
    // Connect job selection to update details
    connect(jobsList, &QListWidget::currentItemChanged, [this](QListWidgetItem *current, QListWidgetItem *) {
        if (current) {
            jobTitleLabel->setText(current->text());
            jobDescriptionLabel->setText("This is a detailed description for the selected job. "
                                        "It would include the job requirements, responsibilities, "
                                        "payment details, and timeline expectations.");
        }
    });
    
    // Connect send proposal button
    connect(sendProposalBtn, &QPushButton::clicked, [this]() {
        QListWidgetItem *selectedJob = jobsList->currentItem();
        if (selectedJob) {
            // In a real app, this would open a dialog to compose a message
            QMessageBox::information(this, "Send Proposal", 
                                    "This would open a message composer to send a proposal for: " + 
                                    selectedJob->text());
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a job first.");
        }
    });
    
    // Layout everything
    jobsLayout->addWidget(filterGroup);
    jobsLayout->addWidget(jobListGroup, 3);
    jobsLayout->addWidget(jobDetailsGroup, 2);
    
    return jobsWidget;
}

QWidget* FreelancerPortal::createProfileTab() {
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
    connect(editProfileBtn, &QPushButton::clicked, [this]() {
    if (!currentUser) return;

    // Cast to Freelancer to access Freelancer-specific methods
    Freelancer* freelancer = dynamic_cast<Freelancer*>(currentUser);
    if (!freelancer) return;
    
    FreelancerProfileEdit* dialog = new FreelancerProfileEdit(freelancer, this);
    
    connect(dialog, &FreelancerProfileEdit::profileUpdated, [this]() {
        updateProfileInfo(); // Refresh the UI with updated info
    });
    
    dialog->exec();
    delete dialog;
});
    connect(editProfileBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Edit Profile", 
                                "This would open a profile editor dialog.");
    });
    
    // Arrange everything in the profile tab
    profileLayout->addWidget(userInfoGroup);
    profileLayout->addWidget(skillsGroup);
    profileLayout->addWidget(historyGroup);
    profileLayout->addWidget(accomplishmentsGroup);
    profileLayout->addWidget(editProfileBtn, 0, Qt::AlignRight);
    
    return profileWidget;
}

QWidget* FreelancerPortal::createMessagesTab() {
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
    connect(messagesList, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *) {
        if (current) {
            messageFromLabel->setText("From: " + current->text().split(" ").back());
            messageSubjectLabel->setText("Subject: " + current->text());
            messageBodyText->setPlainText("This is the body of the selected message. In a real application, "
                                         "this would contain the actual message content from the sender.");
        }
    });
    
    // Layout
    messagesLayout->addWidget(inboxGroup, 2);
    messagesLayout->addWidget(messageDetailsGroup, 3);
    messagesLayout->addWidget(replyBtn, 0, Qt::AlignRight);
    
    return messagesWidget;
}