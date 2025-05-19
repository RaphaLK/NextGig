#pragma once
#include <QWidget>

class HomeWindow : public QWidget {
  Q_OBJECT // A mandatory macro for any Qt class that uses signals & slots
public:
  explicit HomeWindow(QWidget *parent = nullptr);
};