#include "LoginWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent)
{
  QWidget *loginPage = RenderLoginWindow();
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(loginPage);

  setLayout(mainLayout);
}

QWidget *LoginWindow::RenderLoginWindow()
{
  QWidget *loginWindow = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(loginWindow);

  QLabel *title = new QLabel("Welcome Back!", loginWindow);
  QFont titleFont;
  titleFont.setPointSize(15);
  title->setFont(titleFont);

  // Email Input
  QLabel* emailLabel = new QLabel("Email:", loginWindow);
  QLineEdit* emailInput = new QLineEdit(loginWindow);
  emailInput->setPlaceholderText("Enter your email");
  emailInput->setStyleSheet("padding: 8px; font-size: 14px;");

  // Password Input
  QLabel* passwordLabel = new QLabel("Password:", loginWindow);
  QLineEdit* passwordInput = new QLineEdit(loginWindow);
  passwordInput->setPlaceholderText("Enter your password");
  passwordInput->setEchoMode(QLineEdit::Password);
  passwordInput->setStyleSheet("padding: 8px; font-size: 14px;");

  QPushButton *loginButton_Freelancer = new QPushButton("Login (Freelance)", loginWindow);
  QPushButton *loginButton_HiringManager = new QPushButton("Login (Hiring Manager)", loginWindow);

  QString buttonStyle = "QPushButton { padding: 10px; font-size: 14px; }";
  loginButton_Freelancer->setStyleSheet(buttonStyle);
  loginButton_HiringManager->setStyleSheet(buttonStyle);

  QPushButton *backButton = new QPushButton("Back", loginWindow);
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

  layout->addWidget(passwordLabel);
  layout->addWidget(passwordInput);
  layout->addSpacing(30);

  // Place login buttons below the inputs
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(loginButton_Freelancer);
  buttonLayout->addWidget(loginButton_HiringManager);
  layout->addLayout(buttonLayout);

  layout->addStretch();

  return loginWindow;
}
