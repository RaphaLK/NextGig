#pragma once
#include <QWidget>

class HiringManagerPortal : public QWidget {
  Q_OBJECT
public:
  explicit HiringManagerPortal(QWidget *parent = nullptr);
  QWidget* renderHiringManagerPortal();

signals:
  void returnToHomeRequested(); // LOGOUT
};

