#include "TextInputDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include "../src/models/Proposal.h"
#include "UserManager.h"

// TextInputDialog::TextInputDialog(QWidget *parent)
//     : QDialog(parent), inputField(new QLineEdit), submitButton(new QPushButton("Submit"))
// {
//     // Logic to: get UID and get Proposal Object
//     std::string uid = UserManager::getInstance()->getCurrentUser()->getUid();
//     QVBoxLayout *layout = new QVBoxLayout;
//     layout->addWidget(new QLabel("Enter your proposal:"));
//     layout->addWidget(inputField);
//     layout->addWidget(submitButton);
    
//     setLayout(layout);
//     setWindowTitle("Enter proposal");
    
//     layout->addWidget(new QLabel("Requested Budget:"));
//     layout->addWidget(inputField);
//     layout->addWidget(submitButton);
    
//     Proposal* proposal = new Proposal();
//     setLayout(layout);
//     setWindowTitle("Enter proposal");
//     connect(submitButton, &QPushButton::clicked, this, &TextInputDialog::onSubmitClicked);
// }

// void TextInputDialog::onSubmitClicked()
// {
//     inputText = inputField->text();
//     accept(); // closes the dialog with QDialog::Accepted
// }

// QString TextInputDialog::getInputText() const
// {
//     return inputText;
// }
