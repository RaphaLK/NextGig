#pragma once
#include <QWidget>
#include <QStackedWidget>
class HomeWindow : public QWidget
{
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
      QStackedWidget *stack;
  int hiringManagerIndex;
  int freelancerIndex;
  
public:
  explicit HomeWindow(QWidget *parent = nullptr);

  void setupNavigationStack();
  QWidget *RenderHomePage();

private slots:
  void createAndShowHiringManagerPortal();
  void createAndShowFreelancerPortal();
};