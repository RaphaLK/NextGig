#pragma once
#include "User.h"
#include <string>
#include <vector>

typedef struct education
{
    string school;
    string degreeLvl;
} education;

class Freelancer : public User
{
private:
    float hourlyRate;
    education edu;

    std::vector<std::string> skills;

public:
    Freelancer(string _uid, string _email, string _name,
               string _description, vector<string> _tags,
               vector<string> _accomplishments,
               vector<experience> _jobHistory, education _education,
               float _hourlyRate) : User(_uid, _email, _name, _description, _tags, 
                                         _accomplishments, _jobHistory, User::FREELANCER),
                                    edu(_education),hourlyRate(_hourlyRate) {}
    Freelancer() : User(), hourlyRate(0.0) {
        edu.school = "";
        edu.degreeLvl = "";
    }

    float getHourlyRate() const { return hourlyRate; }
    std::vector<std::string> getSkills() const { return skills; }
    education getEducation() const { return edu; }

    void setHourlyRate(float rate) { hourlyRate = rate; }
    void addSkill(const std::string &item) { skills.push_back(item); }
};