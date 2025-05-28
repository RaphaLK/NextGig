#include "Job.h"

void Job::removeRequiredSkill(std::string skill)
{
    requiredSkills.erase(std::remove(requiredSkills.begin(), requiredSkills.end(), skill), requiredSkills.end());
}

