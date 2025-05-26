#pragma once
#include "User.h"
#include <string>
#include <vector>

class HiringManager : public User
{
private:
    std::string companyName;
    std::string companyDescription;

public:
    HiringManager(string _uid, string _email, string _name,
                  string _description, vector<string> _tags,
                  vector<string> _accomplishments,
                  vector<experience> _jobHistory,
                  string _companyName, string _companyDescription) : User(_uid, _email, _name, _description, _tags,
                                                                          _accomplishments, _jobHistory, User::HIRING_MANAGER),
                                                                     companyName(_companyName), companyDescription(_companyDescription) {}

    std::string getCompanyName() const { return companyName; }
    std::string getCompanyDescription() const { return companyDescription; }

    void setCompanyName(const std::string &name) { companyName = name; }
    void setCompanyDescription(const std::string &desc) { companyDescription = desc; }
};