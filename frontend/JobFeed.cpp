#include "JobFeed.h"
#include "client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QUuid>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include <QScrollArea>
#include <QFormLayout>
#include <QSpacerItem>
#include "UserManager.h"
#include "Job.h"
#include <fstream> // Add at the top of your file

JobFeed::JobFeed(QWidget *parent, bool showApplyButtons)
    : QWidget(parent), showApplyButtons(showApplyButtons)
{
  setupUI();
  loadJobs();
}

void JobFeed::setupUI()
{
  qDebug() << "Setup UI -- Job Feed";
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Create filter section
  QGroupBox *filterGroup = new QGroupBox("Filter Jobs");
  QHBoxLayout *filterLayout = new QHBoxLayout();

  searchInput = new QLineEdit();
  searchInput->setPlaceholderText("Search jobs by keyword...");

  skillFilterCombo = new QComboBox();
  skillFilterCombo->addItem("All Skills");
  skillFilterCombo->addItem("Web Development");
  skillFilterCombo->addItem("Mobile Development");
  skillFilterCombo->addItem("UI/UX Design");
  skillFilterCombo->addItem("Data Science");
  skillFilterCombo->addItem("Machine Learning");

  QPushButton *applyFilterBtn = new QPushButton("Apply Filters");
  applyFilterBtn->setStyleSheet("background-color: #4CAF50; color: white; padding: 5px;");

  filterLayout->addWidget(new QLabel("Search:"));
  filterLayout->addWidget(searchInput, 3);
  filterLayout->addWidget(new QLabel("Skill:"));
  filterLayout->addWidget(skillFilterCombo, 1);
  filterLayout->addWidget(applyFilterBtn);

  filterGroup->setLayout(filterLayout);

  // Create jobs list section
  QGroupBox *jobsGroup = new QGroupBox("Available Jobs");
  QVBoxLayout *jobsLayout = new QVBoxLayout();

  jobsList = new QListWidget();
  jobsLayout->addWidget(jobsList);
  jobsGroup->setLayout(jobsLayout);
  // Create job details section
  detailsGroup = new QGroupBox("Job Details");
  QVBoxLayout *detailsLayout = new QVBoxLayout();

  jobTitleLabel = new QLabel("Select a job to view details");
  jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");

  jobDescriptionLabel = new QLabel();
  jobDescriptionLabel->setWordWrap(true);
  jobDescriptionLabel->setTextFormat(Qt::RichText);

  employerLabel = new QLabel();
  paymentLabel = new QLabel();
  skillsLabel = new QLabel();

  detailsLayout->addWidget(jobTitleLabel);
  detailsLayout->addWidget(employerLabel);
  detailsLayout->addWidget(paymentLabel);
  detailsLayout->addWidget(new QLabel("Description:"));
  detailsLayout->addWidget(jobDescriptionLabel);
  detailsLayout->addWidget(new QLabel("Required Skills:"));
  detailsLayout->addWidget(skillsLabel);

  // Only show apply button for freelancers
  if (showApplyButtons)
  {
    applyButton = new QPushButton("Apply for Job");
    applyButton->setStyleSheet("background-color: #007bff; color: white; padding: 8px; font-weight: bold;");
    applyButton->setEnabled(false); // Disabled until a job is selected
    detailsLayout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, this, &JobFeed::onApplyClicked);
  }
  else
  {
    addJobButton = new QPushButton("Add Job");
    addJobButton->setStyleSheet("background-color: #007bff; color: white; padding: 8px; font-weight: bold;");
    addJobButton->setEnabled(true);
    filterLayout->addWidget(addJobButton);

    connect(addJobButton, &QPushButton::clicked, this, &JobFeed::onAddJobClicked);
  }

  detailsGroup->setLayout(detailsLayout);

  // Arrange main layout
  QHBoxLayout *contentLayout = new QHBoxLayout();
  contentLayout->addWidget(jobsGroup, 1);
  contentLayout->addWidget(detailsGroup, 2);

  mainLayout->addWidget(filterGroup);
  mainLayout->addLayout(contentLayout, 1);

  // Connect signals and slots
  connect(jobsList, &QListWidget::currentItemChanged, this, &JobFeed::onJobSelected);
  connect(applyFilterBtn, &QPushButton::clicked, this, &JobFeed::applyFilters);

  // Initial state setup
  detailsGroup->setVisible(false);
}

void JobFeed::loadJobs()
{
  qDebug() << "Loading jobs function called";
  // Show loading indicator
  jobsList->clear();
  QListWidgetItem *loadingItem = new QListWidgetItem("Loading jobs...");
  loadingItem->setFlags(loadingItem->flags() & ~Qt::ItemIsEnabled);
  jobsList->addItem(loadingItem);

  // Get jobs from backend
  BackendClient *client = BackendClient::getInstance();
  client->getJobs([this](bool success, std::vector<Job> jobs)
                  {
        // Clear loading indicator
        jobsList->clear();
        
        if (!success) {
            QListWidgetItem* errorItem = new QListWidgetItem("Failed to load jobs");
            errorItem->setFlags(errorItem->flags() & ~Qt::ItemIsEnabled);
            jobsList->addItem(errorItem);
            return;
        }        
        if (jobs.empty()) {
            QListWidgetItem* emptyItem = new QListWidgetItem("No jobs available");
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled);
            jobsList->addItem(emptyItem);
            return;
        }
        
        allJobs = jobs;
        displayFilteredJobs();
 });
}

void JobFeed::displayFilteredJobs()
{
  jobsList->clear();
  filteredJobs.clear();

  QString searchText = searchInput->text().toLower();
  QString selectedSkill = skillFilterCombo->currentText();
  qDebug() << "JobFeed: Displaying filtered jobs from" << allJobs.size() << "total jobs";
  qDebug() << "JobFeed: Search text:" << searchText << "Selected skill:" << selectedSkill;

  int matchCount = 0;

  for (const auto &job : allJobs)
  {
    bool matchesSearch = true;
    bool matchesSkill = true;

    // Apply text search filter
    if (!searchText.isEmpty())
    {
      QString title = QString::fromStdString(job.getJobTitle()).toLower();
      QString desc = QString::fromStdString(job.getJobDescription()).toLower();
      QString employer = QString::fromStdString(job.getEmployer()).toLower();

      matchesSearch = title.contains(searchText) ||
                      desc.contains(searchText) ||
                      employer.contains(searchText);
    }

    // Apply skill filter
    if (selectedSkill != "All Skills")
    {
      matchesSkill = false;
      for (const auto &skill : job.getRequiredSkills())
      {
        if (QString::fromStdString(skill) == selectedSkill)
        {
          matchesSkill = true;
          break;
        }
      }
    }

    // Add to filtered list if it matches all filters
    if (matchesSearch && matchesSkill)
    {
      filteredJobs.push_back(job);

      // Create list item
      QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(job.getJobTitle()));
      item->setData(Qt::UserRole, QString::fromStdString(job.getJobId())); // Store job ID for later
      jobsList->addItem(item);
      matchCount++;
    }
  }
  qDebug() << "JobFeed: Displayed" << matchCount << "jobs after filtering";
}

void JobFeed::onJobSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        detailsGroup->setVisible(false);
        currentJobId = "";
        if (showApplyButtons && applyButton) {
            applyButton->setEnabled(false);
        }
        return;
    }

    // Get job ID from the item's data
    QString jobId = current->data(Qt::UserRole).toString();
    currentJobId = jobId.toStdString();
    
    // Find the job in the filtered jobs list
    const Job* selectedJob = nullptr;
    for (const auto& job : filteredJobs) {
        if (job.getJobId() == currentJobId) {
            selectedJob = &job;
            break;
        }
    }
    
    if (!selectedJob) {
        qDebug() << "JobFeed: Job not found for ID:" << jobId;
        detailsGroup->setVisible(false);
        return;
    }
    
    // Update the detail view with job info
    jobTitleLabel->setText(QString::fromStdString(selectedJob->getJobTitle()));
    jobDescriptionLabel->setText(QString::fromStdString(selectedJob->getJobDescription()));
    employerLabel->setText("Employer: " + QString::fromStdString(selectedJob->getEmployer()));
    paymentLabel->setText("Payment: $" + QString::fromStdString(selectedJob->getPayment()));
    
    // Format skills list
    QString skillsText;
    const auto& skills = selectedJob->getRequiredSkills();
    for (size_t i = 0; i < skills.size(); ++i) {
        if (i > 0) skillsText += ", ";
        skillsText += QString::fromStdString(skills[i]);
    }
    skillsLabel->setText(skillsText);
    
    // Show the details group and enable apply button
    detailsGroup->setVisible(true);
    if (showApplyButtons && applyButton) {
        applyButton->setEnabled(true);
    }
}

void JobFeed::applyFilters()
{
  if (allJobs.empty()) {
    qDebug() << "JobFeed: No jobs to filter, reloading jobs first";
    loadJobs();
    return;
  }
  
  displayFilteredJobs();
}

void JobFeed::onApplyClicked()
{
    if (currentJobId.empty()) {
        QMessageBox::warning(this, "Error", "Please select a job first");
        return;
    }

    // Check if user is logged in
    UserManager *userManager = UserManager::getInstance();
    // Add null checks before attempting to access the user
    if (!userManager->isUserLoggedIn() || !userManager->getCurrentFreelancer()) {
        QMessageBox::warning(this, "Login Required", "You must be logged in as a freelancer to apply for jobs");
        return;
    }

    // Confirm application
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this, "Confirm Application", 
        "Are you sure you want to apply for this job?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (confirm == QMessageBox::Yes) {
        // In a real app, you'd call your backend to submit the application
        // For now, we'll just show a success message
        QMessageBox::information(this, "Application Submitted", 
                                "Your application has been submitted successfully!");
        
        // Disable the apply button to prevent multiple applications
        if (applyButton) {
            applyButton->setEnabled(false);
        }
    }
}

void JobFeed::refreshJobs()
{
  loadJobs();
}

void JobFeed::onAddJobClicked()
{

  // Check if user is logged in
  UserManager *userManager = UserManager::getInstance();
  if (!userManager->isUserLoggedIn() || !userManager->getCurrentHiringManager())
  {
    QMessageBox::warning(this, "Login Required", "You must be logged in as a freelancer to apply for jobs");
    return;
  }

  // Here you would implement the job application functionality
  // For now, just show a message
  addJob();
}
void JobFeed::addJob()
{
  AddJobDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted)
  {
    return;
  }

  // Build your Job object; generate a client-side ID however you prefer (UUID, timestamp, etc)
  std::string newJobId = QUuid::createUuid().toString().toStdString();

  std::vector<std::string> skillsVector;
  QList<QString> skillsList = dlg.requiredSkills();
  for (const QString &skill : skillsList)
  {
    skillsVector.push_back(skill.toStdString());
  }

  Job job(
      newJobId,
      dlg.jobTitle().toStdString(),
      dlg.jobDescription().toStdString(),
      dlg.employerName().toStdString(),
      QDateTime::currentDateTime().toString(Qt::ISODate).toStdString(),
      dlg.expiryDate().toString(Qt::ISODate).toStdString(),
      skillsVector,
      dlg.payment().toStdString());

  // Send to backend
  BackendClient::getInstance()->addJob(job,
                                       [this](bool success)
                                       {
                                         if (success)
                                         {
                                           // refresh the feed
                                           refreshJobs();
                                         }
                                         else
                                         {
                                           QMessageBox::critical(this, "Error", "Failed to post job.");
                                         }
                                       });
}

void JobFeed::applyToJob()
{
}