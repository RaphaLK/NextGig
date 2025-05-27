// Made this to deal with session management
#pragma once

#include <memory>
#include "../src/models/User.h"
#include "../src/models/Freelancer.h"
#include "../src/models/HiringManager.h"

// Singleton class to manage the application's current user
class UserManager {
public:
    static UserManager* getInstance();

    User* getCurrentUser() { return currentUser; }
    void setCurrentUser(User* user);
    void clearCurrentUser();
    
    // Type-safe getter for freelancer
    Freelancer* getCurrentFreelancer();
    HiringManager* getCurrentHiringManager();

    // Check if current user exists and is of a specific type
    bool isUserLoggedIn() const;
    bool isFreelancer() const;
    bool isHiringManager;

private:
    UserManager() : currentUser(nullptr) {}
    ~UserManager() { clearCurrentUser(); }
    
    // Disable copy/move operations
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    UserManager(UserManager&&) = delete;
    UserManager& operator=(UserManager&&) = delete;
    
    static UserManager* instance;
    User* currentUser;
};