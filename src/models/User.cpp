#include "User.h"

User::~User() {
    // Cleanup code if needed -- user manager handles this but we may need to implement this for future reference
}

void User::setDescription(string description) {
    this->description = description;
}

void User::addTags(string tag) {
    tags.push_back(tag);
}

void User::removeTags(string tag) {
    for (auto it = tags.begin(); it != tags.end(); ++it) {
        if (*it == tag) {
            tags.erase(it);
            break;
        }
    }
}

void User::addAccomplishment(string award) {
    accomplishments.push_back(award);
}

void User::removeAccomplishment(string award) {
    for (auto it = accomplishments.begin(); it != accomplishments.end(); ++it) {
        if (*it == award) {
            accomplishments.erase(it);
            break;
        }
    }
}

void User::removeJobHistory(string job) {
    for (auto it = jobHistory.begin(); it != jobHistory.end(); ++it) {
        if (it->jobTitle == job) {
            jobHistory.erase(it);
            break;
        }
    }
}

void User::editJobHistory(string job, string startDate, string endDate, string description) {
    for (auto& history : jobHistory) {
        if (history.jobTitle == job) {
            history.startDate = startDate;
            history.endDate = endDate;
            history.description = description;
            break;
        }
    }
}

void User::addJobHistory(string job, string startDate, string endDate, string description) {
    experience newJob;
    newJob.jobTitle = job;
    newJob.startDate = startDate;
    newJob.endDate = endDate;
    newJob.description = description;
    jobHistory.push_back(newJob);
}