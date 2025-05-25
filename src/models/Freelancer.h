#pragma once
#include "User.h"
#include <string>
#include <vector>

class Freelancer : public User
{
private:
    float hourlyRate;
    std::vector<std::string> skills;

public:
    Freelancer(string _uid, string _email, string _name,
               string _description, vector<string> _tags,
               experience _education, vector<string> _accomplishments,
               vector<experience> _jobHistory,
               float _hourlyRate) : User(_uid, _email, _name, _description, _tags, _education,
                                         _accomplishments, _jobHistory, User::FREELANCER),
                                    hourlyRate(_hourlyRate) {}

    float getHourlyRate() const { return hourlyRate; }
    std::vector<std::string> getSkills() const { return skills; }

    void setHourlyRate(float rate) { hourlyRate = rate; }
    void addSkill(const std::string &item) { skills.push_back(item); }
};