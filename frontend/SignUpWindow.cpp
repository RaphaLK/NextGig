#include "SignUpWindow.h"
#include <iostream>
#include "../backend/server.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>

SignUpWindow::SignUpWindow(QWidget *parent) : QWidget(parent)
{
  QWidget *signUpPage = RenderSignUpWindow();
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(signUpPage);

  setLayout(mainLayout);
}

QWidget *SignUpWindow::RenderSignUpWindow()
{
  QWidget *signUpWindow = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(signUpWindow);

  QLabel *title = new QLabel("Make an Account", signUpWindow);
  QFont titleFont;

  titleFont.setPointSize(15);
  title->setFont(titleFont);

  QLabel *emailLabel = new QLabel("Email:", signUpWindow);
  QLineEdit *emailInput = new QLineEdit(signUpWindow);
  emailInput->setPlaceholderText("Enter your email");
  emailInput->setStyleSheet("padding: 8px; font-size: 14px;");

  QLabel *usernameLabel = new QLabel("Username:", signUpWindow);
  QLineEdit *usernameInput = new QLineEdit(signUpWindow);
  usernameInput->setPlaceholderText("Select a username");
  usernameInput->setStyleSheet("padding: 8px; font-size: 14px;");

  QLabel *passwordLabel = new QLabel("Password:", signUpWindow);
  QLineEdit *passwordInput = new QLineEdit(signUpWindow);
  passwordInput->setPlaceholderText("Enter your password");
  passwordInput->setEchoMode(QLineEdit::Password);
  passwordInput->setStyleSheet("padding: 8px; font-size: 14px;");

  // Select type of account
  QLabel *accountType = new QLabel("Account Type:", signUpWindow);
  QComboBox *accountTypeSelect = new QComboBox(signUpWindow);
  accountTypeSelect->setPlaceholderText("Please select an account type");
  accountTypeSelect->addItem("Hiring Manager");
  accountTypeSelect->addItem("Freelancer");

  QPushButton *createAccount = new QPushButton("Create Account", signUpWindow);
  QString buttonStyle = "QPushButton { padding: 10px; font-size: 14px; }";
  createAccount->setStyleSheet(buttonStyle);

  connect(createAccount, &QPushButton::clicked, [=]()
          {
    // Get input values
    QString email = emailInput->text();
    QString username = usernameInput->text();
    QString password = passwordInput->text();
    QString accountType = accountTypeSelect->currentText();
    
    // Validate input
    if(email.isEmpty() || username.isEmpty() || password.isEmpty() || accountTypeSelect->currentIndex() < 0) {
        QMessageBox::warning(signUpWindow, "Registration Error", "Please fill in all fields");
        return;
    }
    
    // Call backend to register user
    Server server;
    firebase::auth::User* user = server.registerUser(
        email.toStdString(), 
        password.toStdString(),
        accountType.toStdString(),
        username.toStdString(),
        [](firebase::auth::User* user, const std::string& error) { std::cout << error << std::endl;}
    );
    
    if(user) {
        QMessageBox::information(signUpWindow, "Success", "Account created successfully!");
        
        // Redirect to appropriate portal based on account type
        if(accountType == "Hiring Manager") {
            emit hiringManagerSignupSuccess();
        } else if (accountType == "Freelancer"){
            emit freelancerSignupSuccess();
        } else {
          QMessageBox::information(signUpWindow, "Field empty", "Please pick an account type");
        }
    } else {
        QMessageBox::critical(signUpWindow, "Registration Error", "Failed to create account. Please try again.");
    } });

  QPushButton *backButton = new QPushButton("Back", signUpWindow);

  connect(backButton, &QPushButton::clicked, [this]()
          { emit returnToHomeRequested(); });
  // Layout arrangement
  layout->addWidget(backButton, 0, Qt::AlignLeft); // Back button top-left
  layout->addSpacing(20);
  layout->addWidget(title, 0, Qt::AlignCenter);
  layout->addSpacing(40);

  layout->addWidget(emailLabel);
  layout->addWidget(emailInput);
  layout->addSpacing(15);

  layout->addWidget(usernameLabel);
  layout->addWidget(usernameInput);
  layout->addSpacing(15);

  layout->addWidget(passwordLabel);
  layout->addWidget(passwordInput);
  layout->addSpacing(15);

  layout->addWidget(accountType);
  layout->addWidget(accountTypeSelect);
  layout->addSpacing(30);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(createAccount);
  layout->addLayout(buttonLayout);

  layout->addStretch();
  return signUpWindow;
}