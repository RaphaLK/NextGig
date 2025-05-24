#pragma once
#include <string>
#include <vector>

using namespace std;

typedef struct experience
{
    string jobTitle;
    string startDate;
    string endDate;
    string description;
} experience;

class User
{
    string uid;
    string name;
    string email;
    string description;
    vector<string> tags;
    experience education;
    vector<experience> jobHistory;
    vector<string> accomplishments;
    bool isAuthenticated;

public:
    User(string _uid, string _email, string _name,
         string _description, vector<string> _tags,
         experience _education, vector<string> _accomplishments,
         vector<experience> _jobHistory) : uid(_uid), email(_email), name(_name),
                                           description(_description), tags(_tags),
                                           education(_education), accomplishments(_accomplishments),
                                           jobHistory(_jobHistory), isAuthenticated(false) {}
                                           
    virtual ~User();
    // profile get
    string getName() { return name; };
    string getDescription() { return description; };
    vector<string> getTags() { return tags; };
    vector<string> getAccomplishments() { return accomplishments; };
    vector<experience> getJobHistory() { return jobHistory; };

    // auth
    string getUid() const { return uid; }
    string getEmail() const { return email; }
    bool getAuthStatus() const { return isAuthenticated; }
    void setAuthStatus(bool status) { isAuthenticated = status; }

    // profile set
    void setDescription(string description);
    void addTags(string tag);
    void removeTags(string tag);
    void addAccomplishment(string award);
    void removeAccomplishment(string award);
    void removeJobHistory(string job);
    void editJobHistory(string job, string startDate, string endDate, string description);
    void addJobHistory(string job, string startDate, string endDate, string description);
};