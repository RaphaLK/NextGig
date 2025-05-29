#pragma once
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include "../src/models/User.h"
#include "JobFeed.h"
#include <QVector>

class FreelancerPortal : public QWidget
{
  Q_OBJECT
public:
  explicit FreelancerPortal(QWidget *parent = nullptr);
  // ~FreelancerPortal() { jobFeedWidget = nullptr; }
  QWidget *renderFreelancerPortal();
  void setCurrentUser(User *user);

  QVector<bool> tabInitialized;

private:
  QWidget *createJobsTab();
  QWidget *createProfileTab();
  QWidget *createMessagesTab();
  void updateProfileInfo();
  void loadCurrentJobsData();
  JobFeed *jobFeedWidget;
  QTabWidget *tabWidget; 
  QListWidget *jobsList;
  QLabel *jobTitleLabel;
  QLabel *jobDescriptionLabel;
  QLabel *nameLabel;
  QTextEdit *descriptionTextEdit;
  QListWidget *skillsListWidget;
  QListWidget *jobHistoryList;
  QListWidget *accomplishmentsList;
  QListWidget *appliedJobsList;
  QListWidget *approvedJobsList;
  QTextEdit *jobDescText;
  QLabel *paymentLabel;
  QTextEdit *proposalText;
  QLabel *requestedAmountLabel;
  QLabel *statusLabel;
  QLabel *employerNameLabel;
  QLabel *emailLabel;
  QTextEdit *companyInfoText;
  // Current user data
  User *currentUser;

signals:
  void returnToHomeRequested(); // LOGOUT
  void appliedJobsDataReady(const QJsonArray &jobs);
  void approvedJobsDataReady(const QJsonArray &jobs);
};