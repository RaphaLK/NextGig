#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include "AddJobDialog.h"
#include "../src/models/Job.h"
#include <vector>

class JobFeed : public QWidget
{
    Q_OBJECT
    
public:
    // Constructor takes an optional flag to determine if apply buttons should be shown
    // (true for freelancers, false for hiring managers)
    explicit JobFeed(QWidget *parent = nullptr, bool showApplyButtons = true);
    
    // Call this to refresh the job list
    void refreshJobs();
    
private slots:
    // Handles job selection in the list
    void onJobSelected(QListWidgetItem* current, QListWidgetItem* previous);
    
    // Applies current filter settings
    void applyFilters();
    
    // Handle apply button clicks
    void onApplyClicked();
    
private:
    // UI elements
    QListWidget* jobsList;
    QLabel* jobTitleLabel;
    QLabel* jobDescriptionLabel;
    QLabel* employerLabel;
    QLabel* paymentLabel;
    QLabel* skillsLabel;
    QPushButton* applyButton;
    QPushButton* addJobButton;
    QLineEdit* searchInput;
    QComboBox* skillFilterCombo;
    QGroupBox* detailsGroup;
    
    // Data
    std::vector<Job> allJobs;
    std::vector<Job> filteredJobs;
    std::string currentJobId;
    bool showApplyButtons;
    
    // Helper methods
    void setupUI();
    void loadJobs();
    void addJob();
    void applyToJob();
    void onAddJobClicked();
    void displayFilteredJobs();
};