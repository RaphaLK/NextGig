#include "HomeWindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    // Main title
    auto *title = new QLabel("NextGig", this);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    // Subtitle or description
    auto *subtitle = new QLabel("Find your next opportunity or hire top talent.", this);
    subtitle->setAlignment(Qt::AlignCenter);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    auto *loginBtn = new QPushButton("Login", this);
    auto *signupBtn = new QPushButton("Sign Up", this);
    auto *helpBtn = new QPushButton("Help", this);

    helpBtn->setMaximumWidth(45);

    buttonLayout->addWidget(loginBtn);
    buttonLayout->addWidget(signupBtn);
    buttonLayout->addWidget(helpBtn);

    // Add widgets to main layout
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(30);
    layout->addLayout(buttonLayout);
    layout->addStretch();
    setLayout(layout);
}