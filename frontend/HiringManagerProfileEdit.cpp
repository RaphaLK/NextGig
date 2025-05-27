#include "HiringManagerProfileEdit.h"
#include "client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>

HiringManagerProfileEdit::HiringManagerProfileEdit(HiringManager* hiringManager, QWidget* parent)
    : QDialog(parent), hiringManager(hiringManager)
{
    setWindowTitle("Edit Company Profile");
    setMinimumSize(600, 700);
    setupUi();
    populateFields();
}

void HiringManagerProfileEdit::setupUi()
{
    // Create a scroll area to contain everything
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    
    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
    
    // Basic info group
    QGroupBox* basicInfoGroup = new QGroupBox("Basic Information");
    QFormLayout* basicInfoLayout = new QFormLayout();
    
    QLabel* nameLabel = new QLabel(QString::fromStdString(hiringManager->getName()));
    QLabel* emailLabel = new QLabel(QString::fromStdString(hiringManager->getEmail()));
    
    descriptionEdit = new QTextEdit();
    companyNameEdit = new QLineEdit();
    companyDescriptionEdit = new QTextEdit();
    
    basicInfoLayout->addRow("Name:", nameLabel);
    basicInfoLayout->addRow("Email:", emailLabel);
    basicInfoLayout->addRow("Personal Bio:", descriptionEdit);
    basicInfoLayout->addRow("Company Name:", companyNameEdit);
    basicInfoLayout->addRow("Company Description:", companyDescriptionEdit);
    
    basicInfoGroup->setLayout(basicInfoLayout);
    
    // Accomplishments group
    QGroupBox* accomplishmentsGroup = new QGroupBox("Company Accomplishments");
    QVBoxLayout* accomplishmentsLayout = new QVBoxLayout();
    
    QHBoxLayout* addAccomplishmentLayout = new QHBoxLayout();
    accomplishmentInput = new QLineEdit();
    accomplishmentInput->setPlaceholderText("Enter company achievement or milestone");
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
    QGroupBox* jobHistoryGroup = new QGroupBox("Company History");
    QVBoxLayout* jobHistoryLayout = new QVBoxLayout();
    
    // Form for adding job
    QGroupBox* addJobGroup = new QGroupBox("Add Milestone");
    QFormLayout* addJobLayout = new QFormLayout();
    
    jobTitleInput = new QLineEdit();
    jobStartInput = new QLineEdit();
    jobStartInput->setPlaceholderText("MM/YYYY");
    jobEndInput = new QLineEdit();
    jobEndInput->setPlaceholderText("MM/YYYY or Present");
    jobDescInput = new QTextEdit();
    jobDescInput->setMaximumHeight(80);
    
    QPushButton* addJobBtn = new QPushButton("Add to Company History");
    
    addJobLayout->addRow("Title:", jobTitleInput);
    addJobLayout->addRow("Start Date:", jobStartInput);
    addJobLayout->addRow("End Date:", jobEndInput);
    addJobLayout->addRow("Description:", jobDescInput);
    addJobLayout->addRow("", addJobBtn);
    
    addJobGroup->setLayout(addJobLayout);
    
    // Job list
    jobHistoryList = new QListWidget();
    QPushButton* removeJobBtn = new QPushButton("Remove Selected Entry");
    
    connect(addJobBtn, &QPushButton::clicked, [this]() {
        if (jobTitleInput->text().isEmpty() || jobStartInput->text().isEmpty()) {
            QMessageBox::warning(this, "Missing Information", 
                               "Please enter at least title and start date");
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
    connect(saveBtn, &QPushButton::clicked, this, &HiringManagerProfileEdit::saveProfile);
    
    // Main dialog layout
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(scrollArea);
    dialogLayout->addLayout(buttonLayout);
}

void HiringManagerProfileEdit::populateFields()
{
    // Populate basic info
    descriptionEdit->setText(QString::fromStdString(hiringManager->getDescription()));
    companyNameEdit->setText(QString::fromStdString(hiringManager->getCompanyName()));
    companyDescriptionEdit->setText(QString::fromStdString(hiringManager->getCompanyDescription()));
    
    // Populate accomplishments
    for (const auto& accomplishment : hiringManager->getAccomplishments()) {
        accomplishmentsList->addItem(QString::fromStdString(accomplishment));
    }
    
    // Populate job history
    for (const auto& job : hiringManager->getJobHistory()) {
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

void HiringManagerProfileEdit::saveProfile()
{
    // Get client instance
    BackendClient* client = BackendClient::getInstance();
    
    // Update hiringManager object
    hiringManager->setDescription(descriptionEdit->toPlainText().toStdString());
    hiringManager->setCompanyName(companyNameEdit->text().toStdString());
    hiringManager->setCompanyDescription(companyDescriptionEdit->toPlainText().toStdString());
    
    // Update accomplishments
    // Clear existing accomplishments
    for (const auto& acc : hiringManager->getAccomplishments()) {
        hiringManager->removeAccomplishment(acc);
    }
    
    // Add new accomplishments
    for (int i = 0; i < accomplishmentsList->count(); ++i) {
        hiringManager->addAccomplishment(accomplishmentsList->item(i)->text().toStdString());
    }
    
    // Update job history
    // Clear existing job history
    for (const auto& job : hiringManager->getJobHistory()) {
        hiringManager->removeJobHistory(job.jobTitle);
    }
    
    // Add new jobs
    for (int i = 0; i < jobHistoryList->count(); ++i) {
        QListWidgetItem* item = jobHistoryList->item(i);
        QStringList jobData = item->data(Qt::UserRole).toStringList();
        
        if (jobData.size() >= 4) {
            hiringManager->addJobHistory(
                jobData[0].toStdString(),
                jobData[1].toStdString(),
                jobData[2].toStdString(),
                jobData[3].toStdString()
            );
        }
    }
    
    // Save to database through BackendClient
    client->updateHiringManagerProfile(hiringManager, [this](bool success) {
        if (success) {
            emit profileUpdated();
            accept();
        } else {
            QMessageBox::warning(this, "Update Failed", 
                               "Failed to update profile. Please try again.");
        }
    });
}