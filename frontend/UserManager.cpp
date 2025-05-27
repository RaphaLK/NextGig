#include "UserManager.h"
#include <QMessageBox>
#include <QDebug>

UserManager *UserManager::instance = nullptr;

UserManager *UserManager::getInstance()
{
  if (instance == nullptr)
  {
    instance = new UserManager();
  }
  return instance;
}

void UserManager::setCurrentUser(User *user)
{
  // If there's already a user, clear it first
  if (currentUser != nullptr && currentUser != user)
  {
    currentUser = nullptr;
  }

  // Set the new user
  currentUser = user;

  // Debug output to verify the user was set
  if (currentUser)
  {
    qDebug() << "UserManager: User set to" << QString::fromStdString(currentUser->getName());
    qDebug() << "UserManager: User is Freelancer:" << (dynamic_cast<Freelancer *>(currentUser) != nullptr);
  }
  else
  {
    qDebug() << "UserManager: User set to null";
  }
}

void UserManager::clearCurrentUser()
{
  currentUser = nullptr;
}

Freelancer *UserManager::getCurrentFreelancer()
{
  // If no user is logged in, return nullptr
  if (!currentUser)
  {
    qDebug() << "UserManager: getCurrentFreelancer called but no user is logged in";
    return nullptr;
  }

  // Try to cast to Freelancer
  Freelancer *freelancer = dynamic_cast<Freelancer *>(currentUser);

  // Debug output
  if (!freelancer)
  {
    qDebug() << "UserManager: getCurrentFreelancer called but user is not a Freelancer";
  }

  return freelancer;
}

HiringManager *UserManager::getCurrentHiringManager()
{
  if (!currentUser)
  {
    qDebug() << "UserManager: getCurrentHiringManager called but no user is logged in";
    return nullptr;
  }

  HiringManager *hiringManager = dynamic_cast<HiringManager *>(currentUser);

  if (!hiringManager)
  {
    qDebug() << "UserManager: getHiringManager called but user is not a Freelancer";
  }

  return hiringManager;
}
bool UserManager::isUserLoggedIn() const
{
  bool result = currentUser != nullptr;
  qDebug() << "UserManager: isUserLoggedIn returned" << result;
  return result;
}

bool UserManager::isFreelancer() const
{
  if (!currentUser)
  {
    return false;
  }

  bool result = dynamic_cast<Freelancer *>(currentUser) != nullptr;
  qDebug() << "UserManager: isFreelancer returned" << result;
  return result;
}