#include "LoginWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

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

  QWidget *buttonContainer = new QWidget(loginWindow);
  QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
  buttonLayout->setContentsMargins(50, 20, 50, 20);  
  
  QLabel *title = new QLabel("Welcome Back!", loginWindow);
  QFont titleFont;
  
  titleFont.setPointSize(15);
  title->setFont(titleFont);

  QPushButton *loginButton_Freelancer = new QPushButton("Login (Freelance)", buttonContainer);
  QPushButton *loginButton_HiringManager = new QPushButton("Login (Hiring Manager)", buttonContainer);

  QString buttonStyle = "QPushButton { padding: 10px; font-size: 14px; }";
  loginButton_Freelancer->setStyleSheet(buttonStyle);
  loginButton_HiringManager->setStyleSheet(buttonStyle);

  QPushButton *backButton = new QPushButton("Back", loginWindow);
  connect(backButton, &QPushButton::clicked, [this]() {
    emit returnToHomeRequested();  
  });

  layout->addWidget(backButton, 0, Qt::AlignLeft);  // Back button top-left
  layout->addSpacing(20);  // Add vertical space
  layout->addWidget(title, 0, Qt::AlignCenter);
  layout->addSpacing(40);  // More space before buttons
  layout->addWidget(buttonContainer, 1);  // Center the button container
  layout->addStretch();  // Push everything up

  buttonLayout->addWidget(loginButton_Freelancer);
  buttonLayout->addWidget(loginButton_HiringManager);

  return loginWindow;
}
