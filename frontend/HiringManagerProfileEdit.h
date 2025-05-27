#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include "../src/models/HiringManager.h"

class HiringManagerProfileEdit : public QDialog {
    Q_OBJECT
public:
    explicit HiringManagerProfileEdit(HiringManager* hiringManager, QWidget* parent = nullptr);

private:
    HiringManager* hiringManager;
    QTextEdit* descriptionEdit;
    QLineEdit* companyNameEdit;
    QTextEdit* companyDescriptionEdit;
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