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

HiringManagerPortal::HiringManagerPortal(QWidget *parent) : QWidget(parent), currentUser(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(renderHiringManagerPortal());
    setLayout(mainLayout);
}

void HiringManagerPortal::setCurrentUser(User* user) {
    currentUser = user;
    updateProfileInfo();
}

void HiringManagerPortal::updateProfileInfo() {
    if (!currentUser) return;
    
    // Update profile info in the profile tab
    nameLabel->setText(QString::fromStdString(currentUser->getName()));
    emailLabel->setText(QString::fromStdString(currentUser->getEmail()));
    descriptionTextEdit->setPlainText(QString::fromStdString(currentUser->getDescription()));
}

QWidget* HiringManagerPortal::renderHiringManagerPortal() {
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

QWidget* HiringManagerPortal::createDashboardTab() {
    QWidget *dashboardWidget = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardWidget);
    
    // Jobs status summary
    QGroupBox *summaryGroup = new QGroupBox("Jobs Summary");
    QHBoxLayout *summaryLayout = new QHBoxLayout();
    
    // Active Jobs
    QVBoxLayout *activeJobsLayout = new QVBoxLayout();
    QLabel *activeJobsLabel = new QLabel("Active Jobs");
    activeJobsLabel->setStyleSheet("font-weight: bold;");
    QLabel *activeJobsCount = new QLabel("3");
    activeJobsCount->setStyleSheet("font-size: 24px; color: #007bff;");
    activeJobsLayout->addWidget(activeJobsLabel, 0, Qt::AlignCenter);
    activeJobsLayout->addWidget(activeJobsCount, 0, Qt::AlignCenter);
    
    // Pending Proposals
    QVBoxLayout *pendingProposalsLayout = new QVBoxLayout();
    QLabel *pendingProposalsLabel = new QLabel("Pending Proposals");
    pendingProposalsLabel->setStyleSheet("font-weight: bold;");
    QLabel *pendingProposalsCount = new QLabel("7");
    pendingProposalsCount->setStyleSheet("font-size: 24px; color: #ffc107;");
    pendingProposalsLayout->addWidget(pendingProposalsLabel, 0, Qt::AlignCenter);
    pendingProposalsLayout->addWidget(pendingProposalsCount, 0, Qt::AlignCenter);
    
    // Completed Jobs
    QVBoxLayout *completedJobsLayout = new QVBoxLayout();
    QLabel *completedJobsLabel = new QLabel("Completed Jobs");
    completedJobsLabel->setStyleSheet("font-weight: bold;");
    QLabel *completedJobsCount = new QLabel("12");
    completedJobsCount->setStyleSheet("font-size: 24px; color: #28a745;");
    completedJobsLayout->addWidget(completedJobsLabel, 0, Qt::AlignCenter);
    completedJobsLayout->addWidget(completedJobsCount, 0, Qt::AlignCenter);
    
    summaryLayout->addLayout(activeJobsLayout);
    summaryLayout->addLayout(pendingProposalsLayout);
    summaryLayout->addLayout(completedJobsLayout);
    summaryGroup->setLayout(summaryLayout);
    
    // Your posted jobs list
    QGroupBox *jobsGroup = new QGroupBox("Your Posted Jobs");
    QVBoxLayout *jobsLayout = new QVBoxLayout();
    
    postedJobsList = new QListWidget();
    postedJobsList->addItem("Frontend Developer - React.js");
    postedJobsList->addItem("Backend Developer - Node.js");
    postedJobsList->addItem("UI/UX Designer");
    
    // Job action buttons
    QHBoxLayout *jobActionsLayout = new QHBoxLayout();
    QPushButton *viewDetailsBtn = new QPushButton("View Details");
    QPushButton *editJobBtn = new QPushButton("Edit Job");
    QPushButton *closeJobBtn = new QPushButton("Close Job");
    
    viewDetailsBtn->setStyleSheet("background-color: #007bff; color: white;");
    editJobBtn->setStyleSheet("background-color: #ffc107; color: black;");
    closeJobBtn->setStyleSheet("background-color: #dc3545; color: white;");
    
    jobActionsLayout->addWidget(viewDetailsBtn);
    jobActionsLayout->addWidget(editJobBtn);
    jobActionsLayout->addWidget(closeJobBtn);
    
    // Connect job action buttons
    connect(viewDetailsBtn, &QPushButton::clicked, [this]() {
        if (postedJobsList->currentItem()) {
            QMessageBox::information(this, "Job Details", 
                "Details for: " + postedJobsList->currentItem()->text() + 
                "\n\nSkills: React.js, JavaScript, CSS" +
                "\nBudget: $2000-$3000" +
                "\nDuration: 2 months" +
                "\nApplicants: 5");
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a job first.");
        }
    });
    
    jobsLayout->addWidget(postedJobsList);
    jobsLayout->addLayout(jobActionsLayout);
    jobsGroup->setLayout(jobsLayout);
    
    // Recently received proposals
    QGroupBox *proposalsGroup = new QGroupBox("Recent Proposals");
    QVBoxLayout *proposalsLayout = new QVBoxLayout();
    
    QListWidget *proposalsList = new QListWidget();
    proposalsList->addItem("John Doe - Frontend Developer (React.js)");
    proposalsList->addItem("Jane Smith - UI/UX Designer");
    proposalsList->addItem("Mike Johnson - Backend Developer (Node.js)");
    
    QPushButton *viewProposalBtn = new QPushButton("View Proposal Details");
    viewProposalBtn->setStyleSheet("background-color: #17a2b8; color: white;");
    
    proposalsLayout->addWidget(proposalsList);
    proposalsLayout->addWidget(viewProposalBtn);
    proposalsGroup->setLayout(proposalsLayout);
    
    // Arrange everything in the dashboard
    dashboardLayout->addWidget(summaryGroup);
    dashboardLayout->addWidget(jobsGroup);
    dashboardLayout->addWidget(proposalsGroup);
    
    return dashboardWidget;
}

QWidget* HiringManagerPortal::createProfileTab() {
    QWidget *profileWidget = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);
    
    // Company/User Information
    QGroupBox *infoGroup = new QGroupBox("Company Information");
    QFormLayout *infoLayout = new QFormLayout();
    
    nameLabel = new QLabel("Your Name/Company");
    emailLabel = new QLabel("email@example.com");
    descriptionTextEdit = new QTextEdit();
    descriptionTextEdit->setReadOnly(true);
    descriptionTextEdit->setMaximumHeight(100);
    
    infoLayout->addRow("Name:", nameLabel);
    infoLayout->addRow("Email:", emailLabel);
    infoLayout->addRow("Description:", descriptionTextEdit);
    infoGroup->setLayout(infoLayout);
    
    // Edit profile button
    QPushButton *editProfileBtn = new QPushButton("Edit Profile");
    editProfileBtn->setStyleSheet("background-color: #6c757d; color: white; padding: 8px;");
    
    connect(editProfileBtn, &QPushButton::clicked, [this]() {
        // In a real app this would open a dialog to edit profile
        QMessageBox::information(this, "Edit Profile", "This would open a dialog to edit your profile details.");
    });
    
    // Company/User preferences
    // QGroupBox *preferencesGroup = new QGroupBox("Hiring Preferences");
    // QVBoxLayout *preferencesLayout = new QVBoxLayout();
    
    // QCheckBox *remoteWorkCheck = new QCheckBox("Open to Remote Workers");
    // QCheckBox *intlWorkCheck = new QCheckBox("Open to International Candidates");
    // QCheckBox *entryLevelCheck = new QCheckBox("Consider Entry-Level Candidates");
    
    // remoteWorkCheck->setChecked(true);
    
    // preferencesLayout->addWidget(remoteWorkCheck);
    // preferencesLayout->addWidget(intlWorkCheck);
    // preferencesLayout->addWidget(entryLevelCheck);
    // preferencesGroup->setLayout(preferencesLayout);
    
    // Company/User stats
    QGroupBox *statsGroup = new QGroupBox("Your Statistics");
    QFormLayout *statsLayout = new QFormLayout();
    
    statsLayout->addRow("Jobs Posted:", new QLabel("15"));
    statsLayout->addRow("Jobs Completed:", new QLabel("12"));
    statsLayout->addRow("Average Rating:", new QLabel("4.8/5.0"));
    statsLayout->addRow("Average Response Time:", new QLabel("6 hours"));
    
    statsGroup->setLayout(statsLayout);
    
    // Save preferences button
    QPushButton *savePrefsBtn = new QPushButton("Save Preferences");
    savePrefsBtn->setStyleSheet("background-color: #28a745; color: white; padding: 8px;");
    
    // Arrange everything in the profile tab
    profileLayout->addWidget(infoGroup);
    profileLayout->addWidget(editProfileBtn, 0, Qt::AlignRight);
    // profileLayout->addWidget(preferencesGroup);
    profileLayout->addWidget(savePrefsBtn, 0, Qt::AlignRight);
    profileLayout->addWidget(statsGroup);
    profileLayout->addStretch();
    
    return profileWidget;
}

QWidget* HiringManagerPortal::createPostJobTab() {
    QWidget *postJobWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(postJobWidget);
    
    // Job Details Form
    QGroupBox *jobDetailsGroup = new QGroupBox("Job Details");
    QFormLayout *jobDetailsLayout = new QFormLayout();
    
    QLineEdit *titleEdit = new QLineEdit();
    QTextEdit *descriptionEdit = new QTextEdit();
    
    jobDetailsLayout->addRow("Job Title:", titleEdit);
    jobDetailsLayout->addRow("Description:", descriptionEdit);
    jobDetailsGroup->setLayout(jobDetailsLayout);
    
    // Skills Requirements
    QGroupBox *skillsGroup = new QGroupBox("Required Skills");
    QVBoxLayout *skillsLayout = new QVBoxLayout();
    
    QListWidget *skillsList = new QListWidget();
    QHBoxLayout *addSkillLayout = new QHBoxLayout();
    QLineEdit *skillInput = new QLineEdit();
    QPushButton *addSkillBtn = new QPushButton("Add Skill");
    QPushButton *removeSkillBtn = new QPushButton("Remove Selected");
    
    skillInput->setPlaceholderText("Enter skill (e.g., JavaScript)");
    
    addSkillLayout->addWidget(skillInput);
    addSkillLayout->addWidget(addSkillBtn);
    
    skillsLayout->addLayout(addSkillLayout);
    skillsLayout->addWidget(skillsList);
    skillsLayout->addWidget(removeSkillBtn, 0, Qt::AlignRight);
    
    // Populate with common skills
    skillsList->addItem("HTML");
    skillsList->addItem("CSS");
    skillsList->addItem("JavaScript");
    
    // Connect skill buttons
    connect(addSkillBtn, &QPushButton::clicked, [=]() {
        if (!skillInput->text().isEmpty()) {
            skillsList->addItem(skillInput->text());
            skillInput->clear();
        }
    });
    
    connect(removeSkillBtn, &QPushButton::clicked, [=]() {
        if (skillsList->currentRow() >= 0) {
            delete skillsList->takeItem(skillsList->currentRow());
        }
    });
    
    skillsGroup->setLayout(skillsLayout);
    
    // Budget and Timeline
    QGroupBox *budgetGroup = new QGroupBox("Budget & Timeline");
    QFormLayout *budgetLayout = new QFormLayout();
    
    QHBoxLayout *budgetRangeLayout = new QHBoxLayout();
    QDoubleSpinBox *minBudget = new QDoubleSpinBox();
    QDoubleSpinBox *maxBudget = new QDoubleSpinBox();
    QLabel *toLabel = new QLabel("to");
    
    minBudget->setPrefix("$");
    minBudget->setMaximum(100000);
    minBudget->setValue(500);
    
    maxBudget->setPrefix("$");
    maxBudget->setMaximum(100000);
    maxBudget->setValue(2000);
    
    budgetRangeLayout->addWidget(minBudget);
    budgetRangeLayout->addWidget(toLabel);
    budgetRangeLayout->addWidget(maxBudget);
    
    QComboBox *durationCombo = new QComboBox();
    durationCombo->addItem("Less than 1 week");
    durationCombo->addItem("1-2 weeks");
    durationCombo->addItem("2-4 weeks");
    durationCombo->addItem("1-3 months");
    durationCombo->addItem("3-6 months");
    durationCombo->addItem("More than 6 months");
    
    QComboBox *experienceCombo = new QComboBox();
    experienceCombo->addItem("Entry Level");
    experienceCombo->addItem("Intermediate");
    experienceCombo->addItem("Expert");
    
    budgetLayout->addRow("Budget Range:", budgetRangeLayout);
    budgetLayout->addRow("Expected Duration:", durationCombo);
    budgetLayout->addRow("Experience Level:", experienceCombo);
    
    budgetGroup->setLayout(budgetLayout);
    
    // Additional Requirements
    QGroupBox *additionalGroup = new QGroupBox("Additional Requirements");
    QVBoxLayout *additionalLayout = new QVBoxLayout();
    
    QCheckBox *remoteCheck = new QCheckBox("Remote work allowed");
    QCheckBox *fullTimeCheck = new QCheckBox("Full-time availability required");
    QCheckBox *nDACheck = new QCheckBox("NDA required");
    
    remoteCheck->setChecked(true);
    
    additionalLayout->addWidget(remoteCheck);
    additionalLayout->addWidget(fullTimeCheck);
    additionalLayout->addWidget(nDACheck);
    additionalGroup->setLayout(additionalLayout);
    
    // Submit button
    QPushButton *postJobButton = new QPushButton("Post Job");
    postJobButton->setStyleSheet("background-color: #28a745; color: white; padding: 10px; font-weight: bold;");
    
    // Connect post job button
    connect(postJobButton, &QPushButton::clicked, [=]() {
        if (titleEdit->text().isEmpty() || descriptionEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(this, "Missing Information", "Please fill in job title and description.");
            return;
        }
        
        if (skillsList->count() == 0) {
            QMessageBox::warning(this, "Missing Skills", "Please add at least one required skill.");
            return;
        }
        
        // In a real app, this would save to the backend
        QMessageBox::information(this, "Job Posted", 
                                "Job '" + titleEdit->text() + "' has been posted successfully!");
        
        // Clear form for next posting
        titleEdit->clear();
        descriptionEdit->clear();
        skillsList->clear();
        minBudget->setValue(500);
        maxBudget->setValue(2000);
        durationCombo->setCurrentIndex(0);
        experienceCombo->setCurrentIndex(0);
    });
    
    // Arrange everything in the post job tab
    mainLayout->addWidget(jobDetailsGroup);
    mainLayout->addWidget(skillsGroup);
    mainLayout->addWidget(budgetGroup);
    mainLayout->addWidget(additionalGroup);
    mainLayout->addWidget(postJobButton, 0, Qt::AlignCenter);
    
    return postJobWidget;
}

QWidget* HiringManagerPortal::createMessagesTab() {
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
    connect(sendBtn, &QPushButton::clicked, [=]() {
        if (!newMessageEdit->toPlainText().isEmpty()) {
            messagesArea->append(QString("<div style='background-color: #dcf8c6; padding: 8px; margin: 10px 0px 10px 40px; border-radius: 8px; text-align: right;'><b>You:</b> %1</div>").arg(newMessageEdit->toPlainText()));
            newMessageEdit->clear();
        }
    });
    
    messageThreadLayout->addWidget(threadHeaderLabel);
    messageThreadLayout->addWidget(messagesArea);
    messageThreadLayout->addLayout(newMessageLayout);
    messageThreadGroup->setLayout(messageThreadLayout);
    
    // Arrange the split view
    splittedLayout->addWidget(conversationsGroup, 1);
    splittedLayout->addWidget(messageThreadGroup, 2);
    
    messagesLayout->addLayout(splittedLayout);
    
    // Connect conversation selection to update messages
    connect(conversationsList, &QListWidget::currentTextChanged, [=](const QString &text) {
        threadHeaderLabel->setText("Conversation with " + text.split(" (").first());
        // In a real app, this would load actual messages from the backend
    });
    
    return messagesWidget;
}

QWidget* HiringManagerPortal::createRatingsTab() {
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
    auto addFeedbackItem = [&feedbackLayout](const QString &name, const QString &job, const QString &rating, const QString &comment) {
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
    connect(rateFreelancerBtn, &QPushButton::clicked, [=]() {
        if (pendingRatingsList->currentItem()) {
            // In a real app, this would open a rating dialog
            QMessageBox::information(this, "Rate Freelancer", 
                                    "This would open a rating form for: " + 
                                    pendingRatingsList->currentItem()->text());
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a freelancer to rate.");
        }
    });
    
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