#pragma once
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include "../src/models/User.h"

class FreelancerPortal : public QWidget {
  Q_OBJECT
public:
  explicit FreelancerPortal(QWidget *parent = nullptr);
  QWidget* renderFreelancerPortal();
  void setCurrentUser(User* user);
  
private:
  QWidget* createJobsTab();
  QWidget* createProfileTab();
  QWidget* createMessagesTab();
  void updateProfileInfo();
  
  // UI elements that need to be accessed from multiple methods
  QListWidget* jobsList;
  QLabel* jobTitleLabel;
  QLabel* jobDescriptionLabel;
  QLabel* nameLabel;
  QTextEdit* descriptionTextEdit;
  QListWidget* skillsListWidget;
  QListWidget* jobHistoryList;
  QListWidget* accomplishmentsList;
  QListWidget* appliedJobsList;
  
  // Current user data
  User* currentUser;

signals:
  void returnToHomeRequested(); // LOGOUT
};