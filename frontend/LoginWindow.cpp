#include "LoginWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

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

  QLabel *title = new QLabel("Make an Account", loginWindow);
  QFont titleFont;
  titleFont.setPointSize(15);
  title->setFont(titleFont);

  return loginWindow;
}
