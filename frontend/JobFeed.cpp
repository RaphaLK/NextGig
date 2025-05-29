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
#include "TextInputDialog.h"
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

  // Create a scroll area for details to handle longer content
  QScrollArea *detailsScrollArea = new QScrollArea();
  detailsScrollArea->setWidgetResizable(true);
  detailsScrollArea->setFrameShape(QFrame::NoFrame);

  QWidget *detailsContent = new QWidget();
  QVBoxLayout *detailsContentLayout = new QVBoxLayout(detailsContent);

  // Job title with improved styling
  jobTitleLabel = new QLabel("Select a job to view details");
  jobTitleLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #2c3e50; padding: 5px 0;");
  jobTitleLabel->setWordWrap(true);

  // Create card-like container for job metadata
  QFrame *metadataFrame = new QFrame();
  metadataFrame->setStyleSheet("background-color: #f8f9fa; border-radius: 8px; padding: 10px;");
  QVBoxLayout *metadataLayout = new QVBoxLayout(metadataFrame);

  // Use icons or better formatting for employer and payment
  employerLabel = new QLabel();
  employerLabel->setStyleSheet("font-size: 14px; color: #34495e;");

  paymentLabel = new QLabel();
  paymentLabel->setStyleSheet("font-size: 14px; color: #34495e; font-weight: bold;");

  QLabel *descHeader = new QLabel("Description");
  descHeader->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50; margin-top: 15px;");

  jobDescriptionLabel = new QLabel();
  jobDescriptionLabel->setWordWrap(true);
  jobDescriptionLabel->setTextFormat(Qt::RichText);
  jobDescriptionLabel->setStyleSheet("line-height: 150%; color: #34495e; background-color: #f8f9fa; padding: 12px; border-radius: 8px;");

  // Skills section with tags-like appearance
  QLabel *skillsHeader = new QLabel("Required Skills");
  skillsHeader->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50; margin-top: 15px;");

  skillsLabel = new QLabel();
  skillsLabel->setWordWrap(true);
  skillsLabel->setStyleSheet("padding: 8px;");

  // Add all components to the layout
  metadataLayout->addWidget(employerLabel);
  metadataLayout->addWidget(paymentLabel);

  detailsContentLayout->addWidget(jobTitleLabel);
  detailsContentLayout->addWidget(metadataFrame);
  detailsContentLayout->addWidget(descHeader);
  detailsContentLayout->addWidget(jobDescriptionLabel);
  detailsContentLayout->addWidget(skillsHeader);
  detailsContentLayout->addWidget(skillsLabel);
  detailsContentLayout->addStretch();

  detailsScrollArea->setWidget(detailsContent);
  detailsLayout->addWidget(detailsScrollArea);

  // Only show apply button for freelancers
  if (showApplyButtons)
  {
    applyButton = new QPushButton("Apply for Job");
    applyButton->setStyleSheet("background-color: #007bff; color: white; padding: 12px; font-weight: bold; border-radius: 4px; font-size: 14px;");
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

        if(!showApplyButtons)
        {
            std::string currentUid = UserManager::getInstance()->getCurrentHiringManager()->getName();

            std::vector<Job> filteredJobs;
            for (const auto &job : allJobs)
            {
                if (job.getEmployer() == currentUid) // Ensure Job class has getEmployerUid()
                {
                    filteredJobs.push_back(job);
                }
            }
            allJobs = filteredJobs;
        }

        displayFilteredJobs(); });
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
  if (!current)
  {
    detailsGroup->setVisible(false);
    currentJobId = "";
    if (showApplyButtons && applyButton)
    {
      applyButton->setEnabled(false);
    }
    return;
  }

  // Get job ID from the item's data
  QString jobId = current->data(Qt::UserRole).toString();
  currentJobId = jobId.toStdString();

  // Find the job in the filtered jobs list
  const Job *selectedJob = nullptr;
  for (const auto &job : filteredJobs)
  {
    if (job.getJobId() == currentJobId)
    {
      selectedJob = &job;
      break;
    }
  }

  if (selectedJob)
  {
    // Update the detail view with job info
    jobTitleLabel->setText(QString::fromStdString(selectedJob->getJobTitle()));
    jobDescriptionLabel->setText(QString::fromStdString(selectedJob->getJobDescription()));
    employerLabel->setText(QString("👤 Employer: %1").arg(QString::fromStdString(selectedJob->getEmployer())));
    paymentLabel->setText(QString("💰 Payment: $%1").arg(QString::fromStdString(selectedJob->getPayment())));

    // Format skills list as tags
    QString skillsText;
    const auto &skills = selectedJob->getRequiredSkills();
    for (size_t i = 0; i < skills.size(); ++i)
    {
      QString skillTag = QString("<span style='background-color: #e9ecef; color: #495057; padding: 5px 10px; margin: 3px; border-radius: 15px; display: inline-block;'>%1</span>")
                             .arg(QString::fromStdString(skills[i]));
      skillsText += skillTag + " ";
    }
    skillsLabel->setText(skillsText);

    // Show the details group and enable apply button
    detailsGroup->setVisible(true);
    if (showApplyButtons && applyButton)
    {
      applyButton->setEnabled(true);
    }
  }
}

void JobFeed::applyFilters()
{
  if (allJobs.empty())
  {
    qDebug() << "JobFeed: No jobs to filter, reloading jobs first";
    loadJobs();
    return;
  }

  displayFilteredJobs();
}

void JobFeed::onApplyClicked()
{
  if (currentJobId.empty())
  {
    QMessageBox::warning(this, "Error", "Please select a job first");
    return;
  }

  UserManager *userManager = UserManager::getInstance();
  if (!userManager->isUserLoggedIn() || !userManager->getCurrentFreelancer())
  {
    QMessageBox::warning(this, "Login Required", "You must be logged in as a freelancer to apply for jobs");
    return;
  }

  QMessageBox::StandardButton confirm = QMessageBox::question(
      this, "Confirm Application",
      "Are you sure you want to apply for this job?",
      QMessageBox::Yes | QMessageBox::No);

  if (confirm == QMessageBox::Yes)
  {
    // Show the custom text input dialog
    TextInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
      QString message = dialog.getInputText();
      QString budget = dialog.getBudgetText();

      // Here you would send `message` along with `currentJobId` and user ID to Firebase or your backend
      qDebug() << "Applying to job with ID:" << QString::fromStdString(currentJobId)
               << "with message:" << message;

      // For now, show success message
      BackendClient *client = BackendClient::getInstance();

      Job* selectedJob = nullptr;
      bool foundJob = false;
      for (const auto &job : filteredJobs)
      {
        if (job.getJobId() == currentJobId)
        {
          selectedJob = new Job(job); // Create a copy of the job
          foundJob = true;
          break;
        }
      }

      if (!foundJob) {
        QMessageBox::warning(this, "Error", "Could not find the selected job");
        return;
      }
      
      string uid = UserManager::getInstance()->getCurrentUser()->getUid();
      // Create the proposal object (use a stack variable instead of a pointer)
      Proposal proposal(message.toStdString(), uid, budget.toStdString());
      
      // For improved user experience, show a loading indicator or disable the button
      if (applyButton) {
        applyButton->setEnabled(false);
        applyButton->setText("Submitting...");
      }

      client->applyForJob(*selectedJob, proposal, [this, selectedJob](bool success) {
        // Re-enable the button regardless of outcome
        if (applyButton) {
          applyButton->setEnabled(true);
          applyButton->setText("Apply for Job");
        }
        
        if (success) {
          QMessageBox::information(this, "Application Submitted",
                                 "Your application has been submitted successfully!");
          
          // Disable the apply button after successful submission
          if (applyButton) {
            applyButton->setEnabled(false);
          }
        } else {
          QMessageBox::warning(this, "Application Failed",
                             "Failed to submit your application. Please try again.");
        }
        
        // Clean up the allocated job object
        delete selectedJob;
      });
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
