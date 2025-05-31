#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include "JobFeed.h"
#include "Job.h"

class QListWidgetItem;

class FreelancerPortal : public QWidget
{
    Q_OBJECT

public:
    explicit FreelancerPortal(QWidget *parent = nullptr);
    ~FreelancerPortal();
signals:
    void returnToHomeRequested();

public slots:
    void refreshAllData();

private slots:
    void onAppliedJobSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void onApprovedJobSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void editProfile();
    void onProfileUpdated();
    void onLogoutClicked();
    void onCompletedJobSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void submitRating();

private:
    void setupUI();
    void setupAvailableJobsTab();
    void setupCurrentJobsTab();
    void setupProfileTab();
    void setupJobInfoPanel();
    void setupHiringManagerInfoPanel();
    void setupNavigationBar();

    void loadUserData();
    void loadAppliedJobs();
    void loadApprovedJobs();
    void loadHiringManagerInfo(const QString &employerUid);

    void updateJobDetails(const QJsonObject &jobObj);
    void updateHiringManagerDetails(const QJsonObject &profile);
    void clearJobDetails();
    void clearHiringManagerDetails();

    void setupCompletedJobsTab();
    void setupCompletedJobInfoPanel();
    void setupRatingPanel();
    void loadCompletedJobs();
    // void updateCompletedJobDetails(const QJsonObject &jobObj, const QJsonObject &hiringManagerObj);
    void updateCompletedJobDetails(const QJsonObject &jobObj);

    void loadHiringManagerForRating(const QString &hiringManagerId, const QJsonObject &jobObj);
    void updateRatingPanelDetails(const QJsonObject &profile);
    void clearCompletedJobDetails();
    void clearRatingPanel();

    void setupRatingsTab();
    void loadFreelancerRatings();
    void updateFreelancerRatingsDisplay(const QJsonArray &ratings);
    void addFreelancerFeedbackItem(const QJsonObject &rating);
    QString generateFreelancerStarString(double rating);
    void checkFreelancerExistingRating(const QString &hiringManagerId);
    
    // UI Components
    QTabWidget *tabWidget;

    // Navigation
    QWidget *navigationBar;
    QLabel *welcomeLabel;
    QPushButton *logoutButton;

    // Current Jobs Tab
    QListWidget *appliedJobsList;
    QListWidget *approvedJobsList;
    QGroupBox *jobInfoGroup;
    QGroupBox *hiringManagerInfoGroup;
    JobFeed *availableJobsFeed = nullptr;
    bool availableJobsTabSetup = false;

    // Job Info Panel
    QLabel *jobInfoTitleLabel;
    QLabel *jobInfoEmployerLabel;
    QLabel *jobInfoPaymentLabel;
    QLabel *jobInfoDateLabel;
    QLabel *jobInfoDescriptionLabel;
    QLabel *jobInfoSkillsLabel;

    // Hiring Manager Info Panel
    QLabel *hmNameLabel;
    QLabel *hmEmailLabel;
    QLabel *hmCompanyLabel;
    QLabel *hmDescriptionLabel;
    QLabel *hmCompanyDescLabel;

    // Profile Tab
    QLabel *profileNameLabel;
    QLabel *profileEmailLabel;
    QLabel *profileHourlyRateLabel;
    QLabel *profileDescriptionLabel;
    QLabel *profileSkillsLabel;
    QLabel *profileEducationLabel;
    QLabel *profileDegreeLabel;
    QLabel *profileAccomplishmentsLabel;
    QListWidget *profileJobHistoryList;

    // State management
    Job currentSelectedJob;
    bool hasSelectedJob;
    QJsonObject currentSelectedAppliedJob;
    bool hasSelectedAppliedJob;
    QJsonObject currentSelectedApprovedJob;
    bool hasSelectedApprovedJob;

    // Completed Jobs tab components
    QListWidget *completedJobsList;
    QGroupBox *completedJobInfoGroup;
    QGroupBox *ratingGroup;

    // Completed job info labels
    QLabel *completedJobTitleLabel;
    QLabel *completedJobEmployerLabel;
    QLabel *completedJobPaymentLabel;
    QLabel *completedRatingLabel;
    QLabel *completedJobDateLabel;
    QLabel *completedJobDescriptionLabel;
    QLabel *completedJobStatusLabel;

    // Rating panel components
    QLabel *ratingHmNameLabel;
    QLabel *ratingHmEmailLabel;
    QButtonGroup *ratingButtons;
    QTextEdit *ratingCommentEdit;
    QPushButton *submitRatingBtn;

    QLabel *freelancerOverallStarLabel;
    QLabel *freelancerOverallRatingLabel;
    QLabel *freelancerTotalReviewsLabel;
    QWidget *freelancerFeedbackContentWidget;
    QVBoxLayout *freelancerFeedbackContentLayout;
    // Rating state
    int currentRating = 0;
    QJsonObject currentCompletedJob;

    bool uiSetup = false;
};
