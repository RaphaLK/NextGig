#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

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
  string payment;

public:
  Job(string jobId, string jobTitle,
      string jobDescription, string employerName,
      string dateCreated, string expiryDate,
      vector<string> requiredSkills,
      string payment)
      : jobId(jobId), jobTitle(jobTitle),
        jobDescription(jobDescription),
        employerName(employerName), dateCreated(dateCreated),
        expiryDate(expiryDate), requiredSkills(requiredSkills),
        payment(payment) {}

  ~Job() = default;

  // getters
  string getJobId() const { return jobId; }
  string getJobTitle() const { return jobTitle; }
  string getJobDescription() const { return jobDescription; }
  string getEmployer() const { return employerName; }
  string getDateCreated() const { return dateCreated; }
  string getExpiryDate() const { return expiryDate; }
  vector<string> getRequiredSkills() const { return requiredSkills; }
  string getPayment() const { return payment; }

  // setters
  void updateExpiryDate(string newDate) { expiryDate = newDate; }
  void insertRequiredSkill(string skill) { requiredSkills.push_back(skill); };
  void removeRequiredSkill(string skill);
  void updatePayment(float newPayment) { payment = newPayment; }
};