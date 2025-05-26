#include "FreelancerProfileEdit.h"
#include "client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>

FreelancerProfileEdit::FreelancerProfileEdit(Freelancer* freelancer, QWidget* parent)
    : QDialog(parent), freelancer(freelancer)
{
    setWindowTitle("Edit Profile");
    setMinimumSize(600, 700);
    setupUi();
    populateFields();
}

void FreelancerProfileEdit::setupUi()
{
    // Create a scroll area to contain everything
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
    
    // Basic info group
    QGroupBox* basicInfoGroup = new QGroupBox("Basic Information");
    QFormLayout* basicInfoLayout = new QFormLayout();
    
    QLabel* nameLabel = new QLabel(QString::fromStdString(freelancer->getName()));
    QLabel* emailLabel = new QLabel(QString::fromStdString(freelancer->getEmail()));
    
    descriptionEdit = new QTextEdit();
    hourlyRateEdit = new QDoubleSpinBox();
    hourlyRateEdit->setPrefix("$");
    hourlyRateEdit->setMaximum(500.0);
    hourlyRateEdit->setValue(dynamic_cast<Freelancer*>(freelancer)->getHourlyRate());
    
    basicInfoLayout->addRow("Name:", nameLabel);
    basicInfoLayout->addRow("Email:", emailLabel);
    basicInfoLayout->addRow("Hourly Rate:", hourlyRateEdit);
    basicInfoLayout->addRow("About Me:", descriptionEdit);
    
    basicInfoGroup->setLayout(basicInfoLayout);
    
    // Skills group
    QGroupBox* skillsGroup = new QGroupBox("Skills & Expertise");
    QVBoxLayout* skillsLayout = new QVBoxLayout();
    
    QHBoxLayout* addSkillLayout = new QHBoxLayout();
    skillInput = new QLineEdit();
    skillInput->setPlaceholderText("Enter skill (e.g., JavaScript)");
    QPushButton* addSkillBtn = new QPushButton("Add");
    
    addSkillLayout->addWidget(skillInput);
    addSkillLayout->addWidget(addSkillBtn);
    
    skillsList = new QListWidget();
    QPushButton* removeSkillBtn = new QPushButton("Remove Selected");
    
    skillsLayout->addLayout(addSkillLayout);
    skillsLayout->addWidget(skillsList);
    skillsLayout->addWidget(removeSkillBtn, 0, Qt::AlignRight);
    
    connect(addSkillBtn, &QPushButton::clicked, [this]() {
        if (!skillInput->text().isEmpty()) {
            skillsList->addItem(skillInput->text());
            skillInput->clear();
        }
    });
    
    connect(removeSkillBtn, &QPushButton::clicked, [this]() {
        if (skillsList->currentRow() >= 0) {
            delete skillsList->takeItem(skillsList->currentRow());
        }
    });
    
    skillsGroup->setLayout(skillsLayout);
    
    // Education group
    QGroupBox* educationGroup = new QGroupBox("Education");
    QFormLayout* educationLayout = new QFormLayout();
    
    educationTitleEdit = new QLineEdit();
    educationLevelEdit = new QLineEdit();
    educationTitleEdit->setPlaceholderText("University");
    educationLevelEdit->setPlaceholderText("MS Management Information System");
    educationLayout->addRow("Education:", educationTitleEdit);
    educationLayout->addRow("Degree Obtained:", educationLevelEdit);
    
    educationGroup->setLayout(educationLayout);
    
    // Accomplishments group
    QGroupBox* accomplishmentsGroup = new QGroupBox("Accomplishments");
    QVBoxLayout* accomplishmentsLayout = new QVBoxLayout();
    
    QHBoxLayout* addAccomplishmentLayout = new QHBoxLayout();
    accomplishmentInput = new QLineEdit();
    accomplishmentInput->setPlaceholderText("Enter accomplishment");
    QPushButton* addAccomplishmentBtn = new QPushButton("Add");
    
    addAccomplishmentLayout->addWidget(accomplishmentInput);
    addAccomplishmentLayout->addWidget(addAccomplishmentBtn);
    
    accomplishmentsList = new QListWidget();
    QPushButton* removeAccomplishmentBtn = new QPushButton("Remove Selected");
    
    accomplishmentsLayout->addLayout(addAccomplishmentLayout);
    accomplishmentsLayout->addWidget(accomplishmentsList);
    accomplishmentsLayout->addWidget(removeAccomplishmentBtn, 0, Qt::AlignRight);
    
    connect(addAccomplishmentBtn, &QPushButton::clicked, [this]() {
        if (!accomplishmentInput->text().isEmpty()) {
            accomplishmentsList->addItem(accomplishmentInput->text());
            accomplishmentInput->clear();
        }
    });
    
    connect(removeAccomplishmentBtn, &QPushButton::clicked, [this]() {
        if (accomplishmentsList->currentRow() >= 0) {
            delete accomplishmentsList->takeItem(accomplishmentsList->currentRow());
        }
    });
    
    accomplishmentsGroup->setLayout(accomplishmentsLayout);
    
    // Job History group
    QGroupBox* jobHistoryGroup = new QGroupBox("Job History");
    QVBoxLayout* jobHistoryLayout = new QVBoxLayout();
    
    // Form for adding job
    QGroupBox* addJobGroup = new QGroupBox("Add Job");
    QFormLayout* addJobLayout = new QFormLayout();
    
    jobTitleInput = new QLineEdit();
    jobStartInput = new QLineEdit();
    jobStartInput->setPlaceholderText("MM/YYYY");
    jobEndInput = new QLineEdit();
    jobEndInput->setPlaceholderText("MM/YYYY or Present");
    jobDescInput = new QTextEdit();
    jobDescInput->setMaximumHeight(80);
    
    QPushButton* addJobBtn = new QPushButton("Add Job to History");
    
    addJobLayout->addRow("Job Title:", jobTitleInput);
    addJobLayout->addRow("Start Date:", jobStartInput);
    addJobLayout->addRow("End Date:", jobEndInput);
    addJobLayout->addRow("Description:", jobDescInput);
    addJobLayout->addRow("", addJobBtn);
    
    addJobGroup->setLayout(addJobLayout);
    
    // Job list
    jobHistoryList = new QListWidget();
    QPushButton* removeJobBtn = new QPushButton("Remove Selected Job");
    
    connect(addJobBtn, &QPushButton::clicked, [this]() {
        if (jobTitleInput->text().isEmpty() || jobStartInput->text().isEmpty()) {
            QMessageBox::warning(this, "Missing Information", 
                               "Please enter at least job title and start date");
            return;
        }
        
        QString jobEntry = jobTitleInput->text() + " (" + 
                         jobStartInput->text() + " - " + 
                         jobEndInput->text() + ")";
        
        QListWidgetItem* item = new QListWidgetItem(jobEntry);
        // Store the full data as user data in the item
        QStringList jobData;
        jobData << jobTitleInput->text() 
               << jobStartInput->text() 
               << jobEndInput->text() 
               << jobDescInput->toPlainText();
        item->setData(Qt::UserRole, jobData);
        
        jobHistoryList->addItem(item);
        
        // Clear form
        jobTitleInput->clear();
        jobStartInput->clear();
        jobEndInput->clear();
        jobDescInput->clear();
    });
    
    connect(removeJobBtn, &QPushButton::clicked, [this]() {
        if (jobHistoryList->currentRow() >= 0) {
            delete jobHistoryList->takeItem(jobHistoryList->currentRow());
        }
    });
    
    jobHistoryLayout->addWidget(addJobGroup);
    jobHistoryLayout->addWidget(jobHistoryList);
    jobHistoryLayout->addWidget(removeJobBtn, 0, Qt::AlignRight);
    
    jobHistoryGroup->setLayout(jobHistoryLayout);
    
    // Add all groups to main layout
    mainLayout->addWidget(basicInfoGroup);
    mainLayout->addWidget(skillsGroup);
    mainLayout->addWidget(educationGroup);
    mainLayout->addWidget(accomplishmentsGroup);
    mainLayout->addWidget(jobHistoryGroup);
    
    scrollArea->setWidget(contentWidget);
    
    // Dialog buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* cancelBtn = new QPushButton("Cancel");
    QPushButton* saveBtn = new QPushButton("Save Profile");
    saveBtn->setStyleSheet("background-color: #28a745; color: white;");
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);
    
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &FreelancerProfileEdit::saveProfile);
    
    // Main dialog layout
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(scrollArea);
    dialogLayout->addLayout(buttonLayout);
}

void FreelancerProfileEdit::populateFields()
{
    // Populate basic info
    descriptionEdit->setText(QString::fromStdString(freelancer->getDescription()));
    
    // Populate skills
    for (const auto& skill : dynamic_cast<Freelancer*>(freelancer)->getSkills()) {
        skillsList->addItem(QString::fromStdString(skill));
    }
    
    // Populate tags
    for (const auto& tag : freelancer->getTags()) {
        skillsList->addItem(QString::fromStdString(tag));
    }
    
    // Populate education
    User::education edu = freelancer->getEducation();
    educationTitleEdit->setText(QString::fromStdString(edu.school));
    educationLevelEdit->setText(QString::fromStdString(edu.degreeLvl));
    
    // Populate accomplishments
    for (const auto& accomplishment : freelancer->getAccomplishments()) {
        accomplishmentsList->addItem(QString::fromStdString(accomplishment));
    }
    
    // Populate job history
    for (const auto& job : freelancer->getJobHistory()) {
        QString jobEntry = QString::fromStdString(job.jobTitle) + " (" +
                         QString::fromStdString(job.startDate) + " - " +
                         QString::fromStdString(job.endDate) + ")";
        
        QListWidgetItem* item = new QListWidgetItem(jobEntry);
        QStringList jobData;
        jobData << QString::fromStdString(job.jobTitle)
               << QString::fromStdString(job.startDate)
               << QString::fromStdString(job.endDate)
               << QString::fromStdString(job.description);
        item->setData(Qt::UserRole, jobData);
        
        jobHistoryList->addItem(item);
    }
}

void FreelancerProfileEdit::saveProfile()
{
    // Get client instance
    BackendClient* client = BackendClient::getInstance();
    
    // Update freelancer object
    freelancer->setDescription(descriptionEdit->toPlainText().toStdString());
    dynamic_cast<Freelancer*>(freelancer)->setHourlyRate(hourlyRateEdit->value());
    
    // Update skills/tags
    std::vector<std::string> skillsAndTags;
    for (int i = 0; i < skillsList->count(); ++i) {
        skillsAndTags.push_back(skillsList->item(i)->text().toStdString());
    }
    
    // First 5 items are considered tags, the rest are skills
    std::vector<std::string> tags;
    std::vector<std::string> skills;
    
    int count = 0;
    for (const auto& item : skillsAndTags) {
        if (count < 5) {
            tags.push_back(item);
        } else {
            skills.push_back(item);
        }
        count++;
    }
    
    // Clear and update tags
    // (This is a simplified approach - in a real app, you might want to handle this differently)
    for (const auto& tag : freelancer->getTags()) {
        freelancer->removeTags(tag);
    }
    
    for (const auto& tag : tags) {
        freelancer->addTags(tag);
    }
    
    // Update skills
    dynamic_cast<Freelancer*>(freelancer)->getSkills().clear();
    for (const auto& skill : skills) {
        dynamic_cast<Freelancer*>(freelancer)->addSkill(skill);
    }
    
    // Update education
    education edu;
    edu.school = educationTitleEdit->text().toStdString();
    edu.degreeLvl = educationLevelEdit->text().toStdString();

    // Update accomplishments
    // Clear existing accomplishments
    for (const auto& acc : freelancer->getAccomplishments()) {
        freelancer->removeAccomplishment(acc);
    }
    
    // Add new accomplishments
    for (int i = 0; i < accomplishmentsList->count(); ++i) {
        freelancer->addAccomplishment(accomplishmentsList->item(i)->text().toStdString());
    }
    
    // Update job history
    // Clear existing job history
    for (const auto& job : freelancer->getJobHistory()) {
        freelancer->removeJobHistory(job.jobTitle);
    }
    
    // Add new jobs
    for (int i = 0; i < jobHistoryList->count(); ++i) {
        QListWidgetItem* item = jobHistoryList->item(i);
        QStringList jobData = item->data(Qt::UserRole).toStringList();
        
        if (jobData.size() >= 4) {
            freelancer->addJobHistory(
                jobData[0].toStdString(),
                jobData[1].toStdString(),
                jobData[2].toStdString(),
                jobData[3].toStdString()
            );
        }
    }
    
    // Save to Firebase through BackendClient
    client->updateFreelancerProfile(freelancer, [this](bool success) {
        if (success) {
            emit profileUpdated();
            accept();
        } else {
            QMessageBox::warning(this, "Update Failed", 
                               "Failed to update profile. Please try again.");
        }
    });
}