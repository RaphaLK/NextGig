#include <string>
#include <vector>

using namespace std;

typedef struct WorkExperience
{
    string jobTitle;
    string description;
} WorkExperience;

class User
{
    string name;
    string description;
    vector<string> tags;
    vector<WorkExperience> jobHistory;

public:
    User(string _name,
         string _description,
         vector<string> _tags,
         vector<WorkExperience> _jobHistory) : name(_name),
                                               description(_description),
                                               tags(_tags),
                                               jobHistory(_jobHistory) {}
    ~User();
    string getName() { return name; };
    string getDescription() { return description; };
    vector<string> getTags() { return tags; };
    vector<WorkExperience> getJobHistory() { return jobHistory; };

    void setDescription(string description);
    void addTags(string tag);
    void removeTags(string tag);
    void editJobHistory(string job);
    void editJobHistory(string job, string description);
};