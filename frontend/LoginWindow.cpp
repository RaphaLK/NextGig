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

  QLabel *title = new QLabel("Welcome Back!", loginWindow);
  QFont titleFont;
  
  titleFont.setPointSize(15);
  title->setFont(titleFont);
  
  QPushButton *backButton = new QPushButton("Back", loginWindow);
  connect(backButton, &QPushButton::clicked, [this]() {
    emit returnToHomeRequested();  
  });

  layout->addWidget(backButton);
  layout->addWidget(title);

  return loginWindow;
}
