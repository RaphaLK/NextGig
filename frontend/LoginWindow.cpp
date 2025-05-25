#include "LoginWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include "client.h"

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

  QPushButton *loginButton = new QPushButton("Login", loginWindow);
  connect(loginButton, &QPushButton::clicked, [this, emailInput, passwordInput]() {
    QString email = emailInput->text();
    QString password = passwordInput->text();
    
    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Error", "Please enter email and password");
        return;
    }
    
    // Get client instance
    BackendClient* client = BackendClient::getInstance();
    
    // Connect to server if not already connected
    if (!client->isConnected() && !client->connectToServer()) {
        QMessageBox::critical(this, "Connection Error", "Failed to connect to server");
        return;
    }
    
    // Sign in
    client->signIn(email, password, [this](User* user, const QString& error) {
        if (user) {
            if (user->getUserType() == User::FREELANCER) {
                emit freelancerLoginSuccess();
            } else if (user->getUserType() == User::HIRING_MANAGER) {
                emit hiringManagerLoginSuccess();
            }
        } else {
            QMessageBox::warning(this, "Login Error", "Invalid email or password: " + error);
        }
    });;
  });

  QString buttonStyle = "QPushButton { padding: 10px; font-size: 14px; }";
  loginButton->setStyleSheet(buttonStyle);

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
  buttonLayout->addWidget(loginButton);
  layout->addLayout(buttonLayout);

  layout->addStretch();

  return loginWindow;
}
