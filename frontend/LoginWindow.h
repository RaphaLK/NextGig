#pragma once
#include <QWidget>
#include <QStackedWidget>

class LoginWindow : public QWidget {
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
  QStackedWidget *stack;
public:
  explicit LoginWindow(QWidget *parent = nullptr);
  QWidget* RenderLoginWindow();
signals:
  void returnToHomeRequested();
  void hiringManagerLoginSuccess();
};