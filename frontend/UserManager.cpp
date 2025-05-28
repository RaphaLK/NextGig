#include "UserManager.h"
#include <QMessageBox>
#include <QDebug>

std::unique_ptr<UserManager> UserManager::instance = nullptr;

UserManager *UserManager::getInstance()
{
  if (instance == nullptr)
  {
    instance = std::unique_ptr<UserManager>(new UserManager());
  }
  return instance.get();
}

void UserManager::setCurrentUser(User *user)
{
  if (currentUser != nullptr && currentUser != user)
  {
    currentUser = nullptr;
  }

  // Set the new user
  currentUser = user;

  // Reset specific pointers first
  currentFreelancer = nullptr;
  currentHiringManager = nullptr;

  // Also set the specific user type pointers
  if (currentUser)
  {
    if (currentUser->getUserType() == User::FREELANCER)
    {
      // Use dynamic_cast instead of static_cast for safety
      currentFreelancer = dynamic_cast<Freelancer *>(user);
      if (!currentFreelancer) {
        qDebug() << "WARNING: User type is FREELANCER but dynamic_cast failed!";
        qDebug() << "This indicates a type mismatch between the declared type and actual object type.";
      } else {
        qDebug() << "UserManager: User set as Freelancer:" << QString::fromStdString(currentUser->getName());
      }
    }
    else if (currentUser->getUserType() == User::HIRING_MANAGER)
    {
      // Use dynamic_cast instead of static_cast for safety
      currentHiringManager = dynamic_cast<HiringManager *>(user);
      if (!currentHiringManager) {
        qDebug() << "WARNING: User type is HIRING_MANAGER but dynamic_cast failed!";
        qDebug() << "This indicates a type mismatch between the declared type and actual object type.";
      } else {
        qDebug() << "UserManager: User set as HiringManager:" << QString::fromStdString(currentUser->getName());
      }
    }
    else
    {
      qDebug() << "UserManager: User set with unknown type:" << currentUser->getUserType();
    }
  }
  else
  {
    qDebug() << "UserManager: User set to null";
  }
}

void UserManager::clearCurrentUser()
{
  // Add proper null checks before deletion
  if (currentUser) {
    // Check if the current user is one of our specialized types
    if (currentUser == currentFreelancer || currentUser == currentHiringManager) {
      delete currentUser;
    } else {
      // Handle case where we have separate objects
      if (currentFreelancer) {
        delete currentFreelancer;
      }
      if (currentHiringManager) {
        delete currentHiringManager;
      }
    }
  }

  currentUser = nullptr;
  currentFreelancer = nullptr;
  currentHiringManager = nullptr;
  qDebug() << "UserManager: All users cleared";
}

Freelancer *UserManager::getCurrentFreelancer()
{
  if (!currentFreelancer)
  {
    qDebug() << "UserManager: getCurrentFreelancer called but no freelancer is logged in";
  }
  return currentFreelancer;
}

HiringManager *UserManager::getCurrentHiringManager()
{
  if (!currentHiringManager)
  {
    qDebug() << "UserManager: getCurrentHiringManager called but no hiring manager is logged in";
  }
  return currentHiringManager;
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