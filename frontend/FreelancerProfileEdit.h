#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QDoubleSpinBox>
#include "../src/models/Freelancer.h"

class FreelancerProfileEdit : public QDialog {
    Q_OBJECT
public:
    explicit FreelancerProfileEdit(Freelancer* freelancer, QWidget* parent = nullptr);
    // User::education getEducation() { return freelancer->getEducation(); };

private:
    Freelancer* freelancer;
    QTextEdit* descriptionEdit;
    QDoubleSpinBox* hourlyRateEdit;
    QLineEdit* skillInput;
    QListWidget* skillsList;
    QLineEdit* educationTitleEdit;
    QLineEdit* educationLevelEdit;
    QLineEdit* accomplishmentInput;
    QListWidget* accomplishmentsList;
    QLineEdit* jobTitleInput;
    QLineEdit* jobStartInput;
    QLineEdit* jobEndInput;
    QTextEdit* jobDescInput;
    QListWidget* jobHistoryList;
    
    void setupUi();
    void populateFields();
    void saveProfile();

signals:
    void profileUpdated();
};