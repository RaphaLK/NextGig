#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include "../src/models/Job.h"
#include "../src/models/Proposal.h"
#include "AddJobDialog.h"
#include <vector>
#include <QPointer>
#include <QDateTime>

Q_DECLARE_METATYPE(Job)

class JobFeed : public QWidget
{
    Q_OBJECT
    
public:
    explicit JobFeed(QWidget *parent = nullptr, bool showApplyButtons = true);
    ~JobFeed();
    void setupUI();
    void loadJobs();
    void refreshJobs();
    bool isDestroying = false;
    static QDateTime lastRequestTime;
    static const int MIN_REQUEST_INTERVAL_MS = 1000;
    void setShowApplyButtons(bool show);

private slots:
    void onJobSelected(QListWidgetItem* current, QListWidgetItem* previous);
    void applyFilters();
    void onApplyClicked();
    void onAddJobClicked();
    void onDeleteJobClicked();
    
private:
    void displayJobs();
    void updateJobDetails(const Job& job);
    void clearJobDetails();
    bool passesFilters(const Job& job) const;
    void processJobsResult(bool success, const std::vector<Job>& jobs);

    // UI elements
    QListWidget* jobsList;
    QLabel* jobTitleLabel;
    QLabel* jobDescriptionLabel;
    QLabel* employerLabel;
    QLabel* paymentLabel;
    QLabel* skillsLabel;
    QLabel* dateCreatedLabel;
    QPushButton* applyButton = nullptr;
    QPushButton* addJobButton = nullptr;
    QPushButton* deleteJobButton = nullptr;
    QLineEdit* searchInput;
    QComboBox* skillFilterCombo;
    QGroupBox* detailsGroup;
    QGroupBox* actionsGroup;
    
    // Data
    std::vector<Job> allJobs;
    std::vector<Job> filteredJobs;
    Job currentSelectedJob;
    bool showApplyButtons;
    bool isLoading;
    bool hasSelectedJob;

    // Safety
    QPointer<JobFeed> selfPtr; 
};