#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include "../src/models/User.h"
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include "../src/models/User.h"
#include "UserManager.h"
#include "JobFeed.h"
#include <QJsonObject>
class HiringManagerPortal : public QWidget
{
  Q_OBJECT
public:
  explicit HiringManagerPortal(QWidget *parent = nullptr);
  QWidget *renderHiringManagerPortal();
  void setCurrentUser(User *user);

private:
  // Tab creation methods
  QWidget *createDashboardTab();
  QWidget *createProfileTab();
  QWidget *createPostJobTab();
  QWidget *createMessagesTab();
  QWidget *createRatingsTab();

  // Helper methods
  void updateProfileInfo();
  void fetchProfileFromFirebase();
  void loadProposalsForHiringManager();
  void updateProposalStatus(const QString &jobId, const QString &freelancerId, const QString &status);
  void completeJob(const QString &jobId,
                   const QString &hiringManagerId,
                   const QString &freelancerId,
                   const QString &jobTitle,
                   const QString &jobDescription,
                   double budgetRequested);
  // UI elements that need to be accessed from multiple methods
  QListWidget *incomingPrpsls;
  QLabel *nameLabel;
  QLabel *emailLabel;
  QTextEdit *descriptionTextEdit;
  QLabel *companyNameLabel;
  QTextEdit *companyDescriptionTextEdit;
  QListWidget *accomplishmentsList;

  QListWidget *hmCompletedJobsList;
  QGroupBox *hmCompletedJobInfoGroup;
  QGroupBox *hmRatingGroup;

  // Completed job info labels
  QLabel *hmCompletedJobTitleLabel;
  QLabel *hmCompletedJobFreelancerLabel;
  QLabel *hmCompletedJobPaymentLabel;
  QLabel *hmCompletedJobDateLabel;
  QLabel *hmCompletedJobDescriptionLabel;
  QLabel *hmCompletedJobStatusLabel;

  // Rating panel components
  QLabel *hmRatingFreelancerNameLabel;
  QLabel *hmRatingFreelancerEmailLabel;
  QButtonGroup *hmRatingButtons;
  QTextEdit *hmRatingCommentEdit;
  QPushButton *hmSubmitRatingBtn;

  // Rating state
  int hmCurrentRating = 0;
  QJsonObject hmCurrentCompletedJob;

  // Ratings tab components
  QLabel *overallStarLabel;
  QLabel *overallRatingLabel;
  QLabel *totalReviewsLabel;
  QLabel *communicationRatingLabel;
  QLabel *clarityRatingLabel;
  QLabel *paymentRatingLabel;
  QLabel *professionalismRatingLabel;
  QWidget *feedbackContentWidget;
  QVBoxLayout *feedbackContentLayout;

  QWidget *createCompletedJobsTab();
  void setupHMCompletedJobInfoPanel();
  void setupHMRatingPanel();
  void loadHMCompletedJobs();
  void updateHMCompletedJobDetails(const QJsonObject &jobObj);
  void loadFreelancerForRating(const QString &freelancerId, const QJsonObject &jobObj);
  void updateHMRatingPanelDetails(const QJsonObject &profile);
  void checkHMExistingRating(const QString &freelancerId);
  void clearHMCompletedJobDetails();
  void clearHMRatingPanel();
  void loadHiringManagerRatings();
  void updateRatingsDisplay(const QJsonArray &ratings);
  void addFeedbackItem(const QJsonObject &rating);
  QString generateStarString(double rating);
  // User data
  User *currentUser;

private slots:
  void onHMCompletedJobSelected(QListWidgetItem *current, QListWidgetItem *previous);
  void submitHMRating();
signals:
  void returnToHomeRequested(); // LOGOUT
};