#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class Job
{
  private:
    string jobId;
    string jobTitle;
    string jobDescription;
    string employerName;
    string dateCreated;
    string expiryDate;
    vector<string> requiredSkills;
    vector<string> jobTags;
    float payment;

  public:
    Job(string jobId, string jobTitle, 
      string jobDescription, string employerName, 
      string dateCreated, string expiryDate, 
      vector<string> requiredSkills, vector<string> jobTags, 
      float payment)
        : jobId(jobId), jobTitle(jobTitle), 
        jobDescription(jobDescription), 
        employerName(employerName), dateCreated(dateCreated), 
        expiryDate(expiryDate), requiredSkills(requiredSkills), 
        jobTags(jobTags), payment(payment) {}
    
    ~Job();

    // getters
    string getJobId() { return jobId; }
    string getJobTitle() { return jobTitle; }
    string getJobDescription() { return jobDescription; }
    string getEmployer() { return employerName; }
    string getDateCreated() { return dateCreated; }
    string getExpiryDate() { return expiryDate; }
    vector<string> getRequiredSkills() { return requiredSkills; }
    vector<string> getJobTags() { return jobTags; }
    float getPayment() { return payment; }

    // setters
    void updateExpiryDate(string newDate) { expiryDate = newDate; }
    void insertJobTag(string tag);
    void removeJobTag(string tag);
    void insertRequiredSkill(string skill);
    void removeRequiredSkill(string skill);
    void updatePayment(float newPayment) { payment = newPayment; }
};