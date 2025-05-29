#pragma once
#include <stdlib.h>
#include <string>

using namespace std;

class Proposal {
  string proposalText;
  string uid;
  string budgetRequested;

  public:
    Proposal(string _proposalText, string _uid, string _budgetRequested) : proposalText(_proposalText), uid(_uid), budgetRequested(_budgetRequested) {}

    ~Proposal() = default;

    string getProposalText() { return proposalText; }
    string getUid() { return uid; }
    string getBudgetRequested() { return budgetRequested; }
};