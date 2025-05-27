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
class HiringManagerPortal : public QWidget {
  Q_OBJECT
public:
  explicit HiringManagerPortal(QWidget *parent = nullptr);
  QWidget* renderHiringManagerPortal();
  void setCurrentUser(User* user);

private:
  // Tab creation methods
  QWidget* createDashboardTab();
  QWidget* createProfileTab();
  QWidget* createPostJobTab();
  QWidget* createMessagesTab();
  QWidget* createRatingsTab();
  
  // Helper methods
  void updateProfileInfo();
  void fetchProfileFromFirebase();

  // UI elements that need to be accessed from multiple methods
  QListWidget* postedJobsList;
  QLabel* nameLabel;
  QLabel* emailLabel;
  QTextEdit* descriptionTextEdit;
  QLabel* companyNameLabel;
  QTextEdit* companyDescriptionTextEdit;
  QListWidget* accomplishmentsList;  

  // User data
  User* currentUser;

signals:
  void returnToHomeRequested(); // LOGOUT
};