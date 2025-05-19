#pragma once
#include <QWidget>
#include <QStackedWidget>

class SignUpWindow : public QWidget {
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
  QStackedWidget *stack;
public:
  explicit SignUpWindow(QWidget *parent = nullptr);
  QWidget* RenderSignUpWindow();
signals:
  void returnToHomeRequested();
};