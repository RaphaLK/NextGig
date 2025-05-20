#include "SignUpWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

SignUpWindow::SignUpWindow(QWidget *parent) : QWidget(parent)
{
	QWidget *signUpPage = RenderSignUpWindow();
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(signUpPage);

	setLayout(mainLayout);
}

QWidget* SignUpWindow::RenderSignUpWindow()
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

	QLabel *confirmEmailLabel = new QLabel("Confirm Email:", signUpWindow);
	QLineEdit *confirmEmailInput = new QLineEdit(signUpWindow);
  confirmEmailInput->setPlaceholderText("Re-type your email");
  confirmEmailInput->setStyleSheet("padding: 8px; font-size: 14px;");

  QLabel* passwordLabel = new QLabel("Password:", signUpWindow);
  QLineEdit* passwordInput = new QLineEdit(signUpWindow);
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

	QPushButton* backButton = new QPushButton("Back", signUpWindow);

	connect(backButton, &QPushButton::clicked, [this]() {
    emit returnToHomeRequested();  
  });
  // Layout arrangement
  layout->addWidget(backButton, 0, Qt::AlignLeft);  // Back button top-left
  layout->addSpacing(20);
  layout->addWidget(title, 0, Qt::AlignCenter);
  layout->addSpacing(40);

  layout->addWidget(emailLabel);
  layout->addWidget(emailInput);
  layout->addSpacing(15);

  layout->addWidget(confirmEmailLabel);
  layout->addWidget(confirmEmailInput);
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