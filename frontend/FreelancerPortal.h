#pragma  once

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
    JobFeed* availableJobsFeed = nullptr;
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

    bool uiSetup = false;
};
