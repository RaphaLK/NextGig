#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "../backend/server.h"
#include "client.h"

class SignUpWindow : public QWidget
{
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
      QStackedWidget *stack;
  BackendClient *client;

public:
  explicit SignUpWindow(QWidget *parent = nullptr);
  ~SignUpWindow() = default;
  QWidget *RenderSignUpWindow();
signals:
  void returnToHomeRequested();
  void hiringManagerSignupSuccess();
  void freelancerSignupSuccess();
};