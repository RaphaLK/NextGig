#pragma once
#include <QWidget>
#include <QStackedWidget>
class HomeWindow : public QWidget {
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
  QStackedWidget *stack;
public:
  explicit HomeWindow(QWidget *parent = nullptr);
  QWidget* RenderHomePage();
};