#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "../backend/server.h"

class SignUpWindow : public QWidget
{
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
      QStackedWidget *stack;
  Server *server;

public:
  explicit SignUpWindow(QWidget *parent = nullptr);
  ~SignUpWindow()
  {
    if (server)
    {
      delete server;
      server = nullptr;
    }
  }
  QWidget *RenderSignUpWindow();
signals:
  void returnToHomeRequested();
  void hiringManagerSignupSuccess();
  void freelancerSignupSuccess();
};