#include "JobFeed.h"
#include "UserManager.h"
#include "client.h"
#include <QSplitter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>

JobFeed::JobFeed(QWidget *parent, bool showApplyButtons) 
    : QWidget(parent), showApplyButtons(showApplyButtons), isLoading(false), hasSelectedJob(false), isDestroying(false)
{
    qDebug() << "JobFeed constructor called - instance:" << this << "parent:" << parent;
    qRegisterMetaType<Job>("Job");
    setupUI();
    loadJobs();
}

JobFeed::~JobFeed()
{
    isDestroying = true;
    qDebug() << "JobFeed destructor called - instance:" << this;
}

void JobFeed::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel("Job Feed");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Filter Section
    QGroupBox *filterGroup = new QGroupBox("Filters");
    filterGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);
    
    // Search input
    QLabel *searchLabel = new QLabel("Search:");
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Search jobs by title or description...");
    connect(searchInput, &QLineEdit::textChanged, this, &JobFeed::applyFilters);
    
    // Skill filter
    QLabel *skillLabel = new QLabel("Filter by Skill:");
    skillFilterCombo = new QComboBox();
    skillFilterCombo->addItem("All Skills");
    skillFilterCombo->setMinimumWidth(150);
    connect(skillFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &JobFeed::applyFilters);
    
    // Refresh button
    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setStyleSheet("background-color: #17a2b8; color: white; padding: 8px; border-radius: 4px;");
    connect(refreshButton, &QPushButton::clicked, this, &JobFeed::refreshJobs);
    
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(searchInput, 1);
    filterLayout->addWidget(skillLabel);
    filterLayout->addWidget(skillFilterCombo);
    filterLayout->addWidget(refreshButton);
    filterLayout->addStretch();
    
    mainLayout->addWidget(filterGroup);
    
    // Main content area with splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // Left side: Jobs list
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    jobsList = new QListWidget();
    jobsList->setStyleSheet(
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #e0e0e0; }"
        "QListWidget::item:selected { background-color:rgb(116, 121, 124); }"
        "QListWidget::item:hover { background-color:rgb(116, 121, 124); }");
    connect(jobsList, &QListWidget::currentItemChanged, this, &JobFeed::onJobSelected);
    
    leftLayout->addWidget(jobsList);
    
    // Add job button for hiring managers
    UserManager *userManager = UserManager::getInstance();
    if (!showApplyButtons && userManager->isUserLoggedIn() && !userManager->isFreelancer()) {
        addJobButton = new QPushButton("Post New Job");
        addJobButton->setStyleSheet("background-color: #28a745; color: white; padding: 10px; border-radius: 4px; font-weight: bold;");
        connect(addJobButton, &QPushButton::clicked, this, &JobFeed::onAddJobClicked);
        leftLayout->addWidget(addJobButton);
        
        deleteJobButton = new QPushButton("Delete Selected Job");
        deleteJobButton->setStyleSheet("background-color: #dc3545; color: white; padding: 8px; border-radius: 4px;");
        deleteJobButton->setEnabled(false);
        connect(deleteJobButton, &QPushButton::clicked, this, &JobFeed::onDeleteJobClicked);
        leftLayout->addWidget(deleteJobButton);
    }
    
    // Right side: Job details
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    detailsGroup = new QGroupBox("Job Details");
    detailsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
    
    // Create scroll area for job details
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *detailsContent = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(detailsContent);
    
    // Job detail labels
    jobTitleLabel = new QLabel("Select a job to view details");
    jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #2c3e50; margin-bottom: 10px;");
    jobTitleLabel->setWordWrap(true);
    
    employerLabel = new QLabel();
    employerLabel->setStyleSheet("color: #495057; font-size: 14px; margin-bottom: 5px;");
    
    paymentLabel = new QLabel();
    paymentLabel->setStyleSheet("color: #28a745; font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    
    dateCreatedLabel = new QLabel();
    dateCreatedLabel->setStyleSheet("color: #6c757d; font-size: 12px; margin-bottom: 10px;");
    
    QLabel *descriptionHeaderLabel = new QLabel("Description:");
    descriptionHeaderLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    
    jobDescriptionLabel = new QLabel();
    jobDescriptionLabel->setWordWrap(true);
    jobDescriptionLabel->setStyleSheet(
        "background-color: #f8f9fa; border-radius: 6px; padding: 10px; margin: 5px 0; border: 1px solid #dee2e6;"
    );
    
    QLabel *skillsHeaderLabel = new QLabel("Required Skills:");
    skillsHeaderLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    
    skillsLabel = new QLabel();
    skillsLabel->setWordWrap(true);
    skillsLabel->setStyleSheet(
        "background-color: #e9ecef; border-radius: 6px; padding: 8px; margin: 5px 0; border: 1px solid #dee2e6;"
    );
    
    contentLayout->addWidget(jobTitleLabel);
    contentLayout->addWidget(employerLabel);
    contentLayout->addWidget(paymentLabel);
    contentLayout->addWidget(dateCreatedLabel);
    contentLayout->addWidget(descriptionHeaderLabel);
    contentLayout->addWidget(jobDescriptionLabel);
    contentLayout->addWidget(skillsHeaderLabel);
    contentLayout->addWidget(skillsLabel);
    contentLayout->addStretch();
    
    scrollArea->setWidget(detailsContent);
    detailsLayout->addWidget(scrollArea);
    
    rightLayout->addWidget(detailsGroup);
    
    // Actions group for apply button
    if (showApplyButtons) {
        actionsGroup = new QGroupBox("Actions");
        actionsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
        QVBoxLayout *actionsLayout = new QVBoxLayout(actionsGroup);
        
        applyButton = new QPushButton("Apply for This Job");
        applyButton->setStyleSheet("background-color: #007bff; color: white; padding: 12px; border-radius: 4px; font-weight: bold;");
        applyButton->setEnabled(false);
        connect(applyButton, &QPushButton::clicked, this, &JobFeed::onApplyClicked);
        
        actionsLayout->addWidget(applyButton);
        rightLayout->addWidget(actionsGroup);
    }
    
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({400, 600});
    
    mainLayout->addWidget(splitter);
    
    // Populate skill filter with user's skills if freelancer
    if (showApplyButtons && userManager->isUserLoggedIn() && userManager->isFreelancer()) {
        Freelancer *freelancer = userManager->getCurrentFreelancer();
        if (freelancer) {
            for (const auto &skill : freelancer->getSkills()) {
                skillFilterCombo->addItem(QString::fromStdString(skill));
            }
        }
    }
}

void JobFeed::loadJobs()
{
    if (isLoading || isDestroying) return;
    
    // Throttle requests - prevent rapid-fire requests
    QDateTime now = QDateTime::currentDateTime();
    if (lastRequestTime.isValid() && 
        lastRequestTime.msecsTo(now) < MIN_REQUEST_INTERVAL_MS) {
        qDebug() << "Request throttled - too soon since last request";
        return;
    }
    lastRequestTime = now;
    
    isLoading = true;
    jobsList->clear();
    jobsList->addItem("Loading jobs...");
    
    BackendClient *client = BackendClient::getInstance();
    QPointer<JobFeed> safeThis(this);
    
    qDebug() << "JobFeed making getJobs request - instance:" << this;
    
    client->getJobs([safeThis](bool success, std::vector<Job> jobs) {
        if (!safeThis) {
            qDebug() << "JobFeed callback called but object is destroyed or destroying";
            return;
        }
        
        if (safeThis->isDestroying) {
            qDebug() << "JobFeed callback called but object is destroying";
            return;
        }
        
        QMetaObject::invokeMethod(safeThis, [safeThis, success, jobs]() {
            if (!safeThis || safeThis->isDestroying) {
                return;
            }
            
            qDebug() << "Processing jobs result for instance:" << safeThis.data();
            safeThis->processJobsResult(success, jobs);
        }, Qt::QueuedConnection);
    });
}
// Add this new method to handle the result processing
void JobFeed::processJobsResult(bool success, const std::vector<Job>& jobs)
{
    if (isDestroying || !jobsList) {
        return;
    }
    
    isLoading = false;
    jobsList->clear();
    
    if (!success) {
        jobsList->addItem("Failed to load jobs. Please try again.");
        return;
    }
    
    allJobs = jobs;
    
    // Filter jobs based on user type
    UserManager *userManager = UserManager::getInstance();
    filteredJobs.clear();
    
    if (userManager->isUserLoggedIn()) {
        if (userManager->isFreelancer()) {
            // Freelancers see all jobs
            filteredJobs = allJobs;
        } else {
            // Hiring managers see only their jobs
            HiringManager *hiringManager = userManager->getCurrentHiringManager();
            if (hiringManager) {
                std::string managerName = hiringManager->getName();
                for (const auto &job : allJobs) {
                    if (job.getEmployer() == managerName) {
                        filteredJobs.push_back(job);
                    }
                }
            }
        }
    } else {
        // If not logged in, show all jobs
        filteredJobs = allJobs;
    }
    
    displayJobs();
}

void JobFeed::displayJobs()
{
    jobsList->clear();
    
    if (filteredJobs.empty()) {
        UserManager *userManager = UserManager::getInstance();
        if (!showApplyButtons && userManager->isUserLoggedIn() && !userManager->isFreelancer()) {
            jobsList->addItem("No jobs posted yet. Click 'Post New Job' to create one.");
        } else {
            jobsList->addItem("No jobs available at the moment.");
        }
        clearJobDetails();
        return;
    }
    
    for (const auto &job : filteredJobs) {
        QListWidgetItem *item = new QListWidgetItem();
        
        QString displayText = QString("%1\nEmployer: %2\nPayment: %3")
                               .arg(QString::fromStdString(job.getJobTitle()))
                               .arg(QString::fromStdString(job.getEmployer()))
                               .arg(QString::fromStdString(job.getPayment()));
        
        item->setText(displayText);
        
        // Store job data
        QVariant jobData;
        jobData.setValue(job);
        item->setData(Qt::UserRole, jobData);
        
        jobsList->addItem(item);
    }
}

void JobFeed::onJobSelected(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous)
    
    if (!current || current->data(Qt::UserRole).isNull()) {
        clearJobDetails();
        hasSelectedJob = false;
        if (applyButton) applyButton->setEnabled(false);
        if (deleteJobButton) deleteJobButton->setEnabled(false);
        return;
    }
    
    Job job = current->data(Qt::UserRole).value<Job>();
    currentSelectedJob = job;
    hasSelectedJob = true;
    
    updateJobDetails(job);
    
    if (applyButton) {
        applyButton->setEnabled(true);
    }
    
    if (deleteJobButton) {
        deleteJobButton->setEnabled(true);
    }
}

void JobFeed::updateJobDetails(const Job& job)
{
    jobTitleLabel->setText(QString::fromStdString(job.getJobTitle()));
    employerLabel->setText("Employer: " + QString::fromStdString(job.getEmployer()));
    paymentLabel->setText("Payment: " + QString::fromStdString(job.getPayment()));
    dateCreatedLabel->setText("Posted: " + QString::fromStdString(job.getDateCreated()));
    jobDescriptionLabel->setText(QString::fromStdString(job.getJobDescription()));
    
    // Handle skills
    QStringList skillsList;
    for (const auto &skill : job.getRequiredSkills()) {
        skillsList << QString::fromStdString(skill);
    }
    skillsLabel->setText(skillsList.isEmpty() ? "No specific skills required" : skillsList.join(" • "));
}

void JobFeed::clearJobDetails()
{
    jobTitleLabel->setText("Select a job to view details");
    employerLabel->setText("");
    paymentLabel->setText("");
    dateCreatedLabel->setText("");
    jobDescriptionLabel->setText("");
    skillsLabel->setText("");
}

void JobFeed::applyFilters()
{
    if (allJobs.empty()) return;
    
    filteredJobs.clear();
    
    // First apply user-type filter
    UserManager *userManager = UserManager::getInstance();
    std::vector<Job> userFilteredJobs;
    
    if (userManager->isUserLoggedIn()) {
        if (userManager->isFreelancer()) {
            userFilteredJobs = allJobs;
        } else {
            HiringManager *hiringManager = userManager->getCurrentHiringManager();
            if (hiringManager) {
                std::string managerName = hiringManager->getName();
                for (const auto &job : allJobs) {
                    if (job.getEmployer() == managerName) {
                        userFilteredJobs.push_back(job);
                    }
                }
            }
        }
    } else {
        userFilteredJobs = allJobs;
    }
    
    // Apply search and skill filters
    for (const auto &job : userFilteredJobs) {
        if (passesFilters(job)) {
            filteredJobs.push_back(job);
        }
    }
    
    displayJobs();
}

bool JobFeed::passesFilters(const Job& job) const
{
    // Search filter
    QString searchText = searchInput->text().toLower();
    if (!searchText.isEmpty()) {
        QString jobTitle = QString::fromStdString(job.getJobTitle()).toLower();
        QString jobDesc = QString::fromStdString(job.getJobDescription()).toLower();
        
        if (!jobTitle.contains(searchText) && !jobDesc.contains(searchText)) {
            return false;
        }
    }
    
    // Skill filter
    QString selectedSkill = skillFilterCombo->currentText();
    if (selectedSkill != "All Skills") {
        bool hasSkill = false;
        for (const auto &skill : job.getRequiredSkills()) {
            if (QString::fromStdString(skill) == selectedSkill) {
                hasSkill = true;
                break;
            }
        }
        if (!hasSkill) return false;
    }
    
    return true;
}

void JobFeed::onApplyClicked()
{
    if (!hasSelectedJob) {
        QMessageBox::warning(this, "No Job Selected", "Please select a job to apply for.");
        return;
    }
    
    UserManager *userManager = UserManager::getInstance();
    Freelancer *freelancer = userManager->getCurrentFreelancer();
    
    if (!freelancer) {
        QMessageBox::warning(this, "Not Logged In", "You must be logged in as a freelancer to apply for jobs.");
        return;
    }
    
    // Get proposal details from user
    bool ok;
    QString proposalText = QInputDialog::getMultiLineText(
        this, 
        "Job Application", 
        "Enter your proposal description:",
        "",
        &ok
    );
    
    if (!ok || proposalText.isEmpty()) {
        return;
    }
    
    QString budgetRequest = QInputDialog::getText(
        this,
        "Budget Request",
        "Enter your requested budget (e.g., $1000):",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || budgetRequest.isEmpty()) {
        return;
    }
    
    // Create proposal
    Proposal proposal(
        freelancer->getUid(),
        proposalText.toStdString(),
        budgetRequest.toStdString()
    );
    
    // Apply for job
    BackendClient *client = BackendClient::getInstance();
    client->applyForJob(currentSelectedJob, proposal, [this](bool success) {
        if (success) {
            QMessageBox::information(this, "Success", "Your application has been submitted successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to submit application. Please try again.");
        }
    });
}

void JobFeed::onAddJobClicked()
{
    AddJobDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshJobs();
    }
}

void JobFeed::onDeleteJobClicked()
{
    if (!hasSelectedJob) {
        QMessageBox::warning(this, "No Job Selected", "Please select a job to delete.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Job",
        "Are you sure you want to delete this job? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        BackendClient *client = BackendClient::getInstance();
        client->removeJob(currentSelectedJob.getJobId(), [this](bool success) {
            if (success) {
                QMessageBox::information(this, "Success", "Job deleted successfully!");
                refreshJobs();
            } else {
                QMessageBox::warning(this, "Error", "Failed to delete job. Please try again.");
            }
        });
    }
}

void JobFeed::refreshJobs()
{
    loadJobs();
}

void JobFeed::setShowApplyButtons(bool show)
{
    showApplyButtons = show;
    if (applyButton) {
        applyButton->setVisible(show);
    }
    if (actionsGroup) {
        actionsGroup->setVisible(show);
    }
}